/// crash_recovery.cpp

#include "crash_recovery.h"
#include "display.h"
#include "marlin_client.h"
#include "gui.hpp"

#include "../Marlin/src/module/endstops.h"
#include "../../Marlin/src/module/stepper.h"

void print_axis_position(const AxisEnum axis) {
    char text[10];
    snprintf(text, 10, "%f", (double)planner.get_axis_position_mm(axis));
    display::DrawText(Rect16(0, 0, 100, 15), string_view_utf8::MakeRAM((const uint8_t *)text), GuiDefaults::Font, COLOR_BLACK, COLOR_WHITE);
}

void print_current_position(const AxisEnum axis) {
    char text[10];
    snprintf(text, 10, "%f", (double)current_position.pos[axis]);
    display::DrawText(Rect16(0, 20, 100, 15), string_view_utf8::MakeRAM((const uint8_t *)text), GuiDefaults::Font, COLOR_BLACK, COLOR_WHITE);
}

// void print_stepper_position(const AxisEnum axis) {
//     char text[10];
//     snprintf(text, 10, "%f", (double)(stepper.position(axis) * planner.steps_to_mm[axis]));
//     display::DrawText(Rect16(0, 40, 100, 15), string_view_utf8::MakeRAM((const uint8_t *)text), GuiDefaults::Font, COLOR_BLACK, COLOR_WHITE);
// }

void print_offset(const AxisEnum axis) {
    char text[10];
    snprintf(text, 10, "%f", (double)position_shift[axis]);
    display::DrawText(Rect16(0, 40, 100, 15), string_view_utf8::MakeRAM((const uint8_t *)text), GuiDefaults::Font, COLOR_BLACK, COLOR_RED);
}

void print_all(const AxisEnum axis) {
    print_axis_position(axis);
    print_current_position(axis);
    // print_stepper_position(axis);
    print_offset(axis);
}
extern "C" {
void print_cube(color_t color) {
    display::FillRect(Rect16(0, 0, 10, 10), color);
}
}
void do_homing_move_crash(const AxisEnum axis, float distance) {
    if (!(axis == X_AXIS || axis == Y_AXIS))
        return;

    const feedRate_t real_fr_mm_s = homing_feedrate(axis);
    abce_pos_t target;

    // /// move back
    // abce_pos_t target = { planner.get_axis_position_mm(A_AXIS), planner.get_axis_position_mm(B_AXIS), planner.get_axis_position_mm(C_AXIS), planner.get_axis_position_mm(E_AXIS) };
    // target[axis] = 0;
    // planner.set_machine_position_mm(target);
    // target[axis] = (distance > 0) ? -MOVE_BACK_BEFORE_HOMING_DISTANCE : MOVE_BACK_BEFORE_HOMING_DISTANCE;
    // // Set delta/cartesian axes directly
    // planner.buffer_segment(target, real_fr_mm_s / 4, active_extruder);
    // planner.synchronize();

    /// home
    target = { planner.get_axis_position_mm(A_AXIS), planner.get_axis_position_mm(B_AXIS), planner.get_axis_position_mm(C_AXIS), planner.get_axis_position_mm(E_AXIS) };
    target[axis] += distance;

    planner.buffer_segment(target, real_fr_mm_s, active_extruder);
    planner.synchronize();
    /// reset number of crashes
    endstops.validate_homing_move();
}

void home_axis_blocking(const AxisEnum axis, const bool positive_dir, const bool reset_position = false) {
    endstops.enable(true);
    const int dir = positive_dir ? 1 : -1;
    do_homing_move_crash(axis, 1.5f * max_length(axis) * dir);

    if (reset_position) {
        set_axis_is_at_home(axis);
        sync_plan_position();

        if (axis == X_AXIS) {
            /// set position
            abce_pos_t target;
            target = { planner.get_axis_position_mm(A_AXIS), planner.get_axis_position_mm(B_AXIS), planner.get_axis_position_mm(C_AXIS), planner.get_axis_position_mm(E_AXIS) };
            target[axis] = X_MAX_POS;
            planner.set_machine_position_mm(target);
        }
    }
    endstops.not_homing();
    planner.synchronize();
    current_position.pos[axis] = planner.get_axis_position_mm(axis);
    sync_plan_position();
}

void position_backup(abce_pos_t &machine, xyze_pos_t &planned) {
    machine = { planner.get_axis_position_mm(A_AXIS), planner.get_axis_position_mm(B_AXIS), planner.get_axis_position_mm(C_AXIS), planner.get_axis_position_mm(E_AXIS) };
    for (int axis = X_AXIS; axis < E_AXIS; ++axis) {
        planned[axis] = current_position.pos[axis];
        set_current_from_steppers_for_axis(AxisEnum(axis));
        // current_position.pos[axis] = machine[axis];
    }
}

void position_restore(abce_pos_t &machine, xyze_pos_t &planned) {
    machine.pos[E_AXIS] = planner.get_axis_position_mm(E_AXIS);
    planner.set_machine_position_mm(machine);
    for (int axis = X_AXIS; axis < E_AXIS; ++axis)
        current_position.pos[axis] = planned[axis];
}

/// like Planner::quick_stop but saves buffer for later restore
void crash_quick_stop(uint8_t *buffer_pointers, block_t *buffer, abce_pos_t &machine, xyze_pos_t &planned) {
    const bool was_enabled = STEPPER_ISR_ENABLED();
    if (was_enabled)
        DISABLE_STEPPER_DRIVER_INTERRUPT();

    /// save pointers
    buffer_pointers[0] = Planner::block_buffer_head;
    buffer_pointers[1] = Planner::block_buffer_nonbusy;
    buffer_pointers[2] = Planner::block_buffer_planned;
    buffer_pointers[3] = Planner::block_buffer_tail;
    buffer_pointers[4] = Planner::delay_before_delivering;
    // Drop all queue entries
    Planner::block_buffer_nonbusy
        = Planner::block_buffer_planned
        = Planner::block_buffer_head
        = Planner::block_buffer_tail;

    // Restart the block delay for the first movement - As the queue was
    // forced to empty, there's no risk the ISR will touch this.
    Planner::delay_before_delivering = 100;

#if HAS_SPI_LCD
    // Clear the accumulated runtime
    clear_block_buffer_runtime();
#endif

    // Make sure to drop any attempt of queuing moves for at least 1 second
    // Planner::cleaning_buffer_counter = 1000;

    // Reenable Stepper ISR
    if (was_enabled)
        ENABLE_STEPPER_DRIVER_INTERRUPT();

    // And stop the stepper ISR
    stepper.quick_stop();

    /// copy buffer
    memcpy(buffer, Planner::block_buffer, sizeof(block_t) * BLOCK_BUFFER_SIZE);
    position_backup(machine, planned);
}

void restore_planner_after_crash(uint8_t *buffer_pointers, block_t *buffer, abce_pos_t &machine, xyze_pos_t &planned) {
    memcpy(Planner::block_buffer, buffer, sizeof(block_t) * BLOCK_BUFFER_SIZE);

    Planner::block_buffer_head = buffer_pointers[0];
    Planner::block_buffer_nonbusy = buffer_pointers[1];
    Planner::block_buffer_planned = buffer_pointers[2];
    Planner::block_buffer_tail = buffer_pointers[3];
    Planner::delay_before_delivering = buffer_pointers[4];
}

void crash_recovery() {
    uint8_t buffer_pointers[5];
    block_t buffer[BLOCK_BUFFER_SIZE];
    abce_pos_t position_machine;
    xyze_pos_t position_planned;

    display::FillRect(Rect16(229, 0, 10, 10), COLOR_GREEN);

    print_all(X_AXIS);

    /// backup current position and planner buffer and clear the buffer to enable immediate movements
    crash_quick_stop(buffer_pointers, buffer, position_machine, position_planned);

    // current_position.z += 20;
    // line_to_current_position(homing_feedrate(Z_AXIS));
    // while (planner.movesplanned() != 0 || queue.length != 0) {
    //     gui_loop();
    //     print_all(X_AXIS);
    // }

    display::FillRect(Rect16(229, 0, 10, 10), COLOR_OLIVE);
    print_all(X_AXIS);

    marlin_print_pause();
    display::FillRect(Rect16(229, 0, 10, 10), COLOR_SILVER);

    while (marlin_vars()->print_state != mpsPaused) {
        gui_loop();
        print_all(X_AXIS);
    }
    display::FillRect(Rect16(229, 0, 10, 10), COLOR_LIME);

    home_axis_blocking(X_AXIS, true, true);
    float length[2] = { planner.get_axis_position_mm(X_AXIS), planner.get_axis_position_mm(Y_AXIS) };
    home_axis_blocking(X_AXIS, false);
    length[0] -= planner.get_axis_position_mm(X_AXIS);
    length[1] -= planner.get_axis_position_mm(Y_AXIS);
    bool axis_ok[2] = { false, false };

    if (182 <= length[0] && length[0] <= 190)
        axis_ok[0] = true;

    if (axis_ok[0]) {
        marlin_print_resume();
        Planner::cleaning_buffer_counter = 1000;
        while (marlin_vars()->print_state != mpsPrinting) {
            Planner::cleaning_buffer_counter = 1000;
            gui_loop();
            print_all(X_AXIS);
        }
        restore_planner_after_crash(buffer_pointers, buffer, position_machine, position_planned);
    }
    Planner::cleaning_buffer_counter = 0;
}
