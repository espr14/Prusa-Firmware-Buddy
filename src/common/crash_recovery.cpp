/// crash_recovery.cpp

#include "crash_recovery.h"
#include "display.h"
#include "marlin_client.h"
#include "gui.hpp"

#include "../Marlin/src/module/endstops.h"
#include "../../Marlin/src/module/planner.h"
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

void print_stepper_position(const AxisEnum axis) {
    char text[10];
    snprintf(text, 10, "%f", (double)(stepper.position(axis) * planner.steps_to_mm[axis]));
    display::DrawText(Rect16(0, 40, 100, 15), string_view_utf8::MakeRAM((const uint8_t *)text), GuiDefaults::Font, COLOR_BLACK, COLOR_WHITE);
}

void print_offset(const AxisEnum axis) {
    char text[10];
    snprintf(text, 10, "%f", (double)position_shift[axis]);
    display::DrawText(Rect16(0, 60, 100, 15), string_view_utf8::MakeRAM((const uint8_t *)text), GuiDefaults::Font, COLOR_BLACK, COLOR_RED);
}

void print_all(const AxisEnum axis) {
    print_axis_position(axis);
    print_current_position(axis);
    print_stepper_position(axis);
    print_offset(axis);
}

void do_homing_move_crash(const AxisEnum axis, float distance) {
    if (!(axis == X_AXIS || axis == Y_AXIS))
        return;

    // if ((home_dir(axis) > 0) != (distance > 0))
    //     distance = -distance;
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
    // target[axis] = 0;
    // planner.set_machine_position_mm(target);
    target[axis] += distance;

    planner.buffer_segment(target, real_fr_mm_s, active_extruder);
    planner.synchronize();
    /// reset number of crashes
    endstops.validate_homing_move();
}

void home_Marlin(const AxisEnum axis, int dir, bool reset_position = false) {
    if (!(dir == 1 || dir == -1))
        return;

    endstops.enable(true);
    // #define CAN_HOME_X true
    // #define CAN_HOME_Y true
    do_homing_move_crash(axis, 1.5f * max_length(axis) * dir);
    planner.synchronize();

    if (reset_position) {
        set_axis_is_at_home(axis);
        sync_plan_position();

        if (axis == X_AXIS) {
            /// set position
            abce_pos_t target;
            target = { planner.get_axis_position_mm(A_AXIS), planner.get_axis_position_mm(B_AXIS), planner.get_axis_position_mm(C_AXIS), planner.get_axis_position_mm(E_AXIS) };
            target[axis] = X_MAX_POS + .5f;
            planner.set_machine_position_mm(target);
        }

        // destination[axis] = current_position[axis];
        // current_position.pos[axis] = 0;
    }

    endstops.not_homing();
    // line_to_current_position(100);
    planner.synchronize();
    // synchronize positions
    current_position.pos[axis] = planner.get_axis_position_mm(axis);
    sync_plan_position();
}

void home_to_start_Marlin(AxisEnum axis) {
    endstops.enable(true);
    homeaxis(axis);
    endstops.not_homing();
    current_position.pos[axis] = 0;
    // line_to_current_position(100);
    planner.synchronize();
}

void home_to_end_Marlin(AxisEnum axis) {

    // stepperX.homing_threshold(X_STALL_SENSITIVITY);

    endstops.enable(true);
    homeaxis(axis);
    endstops.not_homing();
    current_position.pos[axis] = 0;
    line_to_current_position(100);
    planner.synchronize();
}

void home_to_end_custom(AxisEnum axis) {
    endstops.enable(true);
    // sg_sampling_enable();
    current_position.pos[axis] = 190;
    line_to_current_position(100);

    if (planner.movesplanned())
        gui_loop();

    // sg_sampling_disable();
    endstops.not_homing();
}

void crash_recovery() {
    display::FillRect(Rect16(0, 0, 10, 10), COLOR_GREEN);
    marlin_print_pause();
    while (marlin_vars()->print_state != mpsPaused)
        gui_loop();

    print_all(X_AXIS);

    // home_to_start_Marlin(X_AXIS);
    home_Marlin(X_AXIS, 1, true);
    print_all(X_AXIS);

    // const uint32_t m_StartPos_usteps = stepper.position((AxisEnum)X_AXIS);

    home_Marlin(X_AXIS, -1);
    print_all(X_AXIS);

    // display::FillRect(Rect16(0, 0, 10, 10), COLOR_WHITE);

    // marlin_print_resume();

    // const int32_t endPos_usteps = stepper.position((AxisEnum)X_AXIS);
    // const int32_t length_usteps = endPos_usteps - m_StartPos_usteps;
    // const float length_mm = (length_usteps * planner.steps_to_mm[(AxisEnum)X_AXIS]);
    // if (182 <= length_mm && length_mm <= 190) {
    //     display::FillRect(Rect16(0, 0, 10, 10), COLOR_LIME);
    // } else {
    //     display::FillRect(Rect16(0, 0, 10, 10), COLOR_RED);
    // }
}
