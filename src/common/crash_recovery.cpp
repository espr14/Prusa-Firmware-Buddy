/// crash_recovery.cpp

#include "crash_recovery.h"
#include "display.h"
#include "marlin_client.h"
#include "gui.hpp"

#include "../Marlin/src/module/endstops.h"
#include "../../Marlin/src/module/planner.h"
#include "../../Marlin/src/module/stepper.h"

void do_homing_move_crash(const AxisEnum axis, const float distance, const feedRate_t fr_mm_s = 0.0) {

#if HOMING_Z_WITH_PROBE && HAS_HEATED_BED && ENABLED(WAIT_FOR_BED_HEATER)
    // Wait for bed to heat back up between probing points
    if (axis == Z_AXIS && distance < 0 && thermalManager.isHeatingBed()) {
        serialprintPGM(msg_wait_for_bed_heating);
        LCD_MESSAGEPGM(MSG_BED_HEATING);
        thermalManager.wait_for_bed();
        ui.reset_status();
    }
#endif

    // Only do some things when moving towards an endstop
    const int8_t axis_home_dir =
#if ENABLED(DUAL_X_CARRIAGE)
        (axis == X_AXIS) ? x_home_dir(active_extruder) :
#endif
                         home_dir(axis);
    const bool is_home_dir = (axis_home_dir > 0) == (distance > 0);

#if ENABLED(SENSORLESS_HOMING)
    sensorless_t stealth_states;
#endif

    if (is_home_dir) {

#if HOMING_Z_WITH_PROBE && QUIET_PROBING
        if (axis == Z_AXIS)
            probing_pause(true);
#endif

// Disable stealthChop if used. Enable diag1 pin on driver.
#if ENABLED(SENSORLESS_HOMING)
        stealth_states = start_sensorless_homing_per_axis(axis);
#endif
    }

    const feedRate_t real_fr_mm_s = fr_mm_s ?: homing_feedrate(axis);

#if ENABLED(MOVE_BACK_BEFORE_HOMING)
    if ((axis == X_AXIS) || (axis == Y_AXIS)) {
        abce_pos_t target = { planner.get_axis_position_mm(A_AXIS), planner.get_axis_position_mm(B_AXIS), planner.get_axis_position_mm(C_AXIS), planner.get_axis_position_mm(E_AXIS) };
        target[axis] = 0;
        planner.set_machine_position_mm(target);
        float dist = (distance > 0) ? -MOVE_BACK_BEFORE_HOMING_DISTANCE : MOVE_BACK_BEFORE_HOMING_DISTANCE;
        target[axis] = dist;
    #if IS_KINEMATIC && DISABLED(CLASSIC_JERK)
        const xyze_float_t delta_mm_cart { 0 };
    #endif
        // Set delta/cartesian axes directly
        planner.buffer_segment(target
    #if IS_KINEMATIC && DISABLED(CLASSIC_JERK)
            ,
            delta_mm_cart
    #endif
            ,
            real_fr_mm_s / 4, active_extruder);
        planner.synchronize();
    }
#endif

    abce_pos_t target = { planner.get_axis_position_mm(A_AXIS), planner.get_axis_position_mm(B_AXIS), planner.get_axis_position_mm(C_AXIS), planner.get_axis_position_mm(E_AXIS) };
    target[axis] = 0;
    planner.set_machine_position_mm(target);
    target[axis] = distance;

#if IS_KINEMATIC && DISABLED(CLASSIC_JERK)
    const xyze_float_t delta_mm_cart { 0 };
#endif

    // Set delta/cartesian axes directly
    planner.buffer_segment(target
#if IS_KINEMATIC && DISABLED(CLASSIC_JERK)
        ,
        delta_mm_cart
#endif
        ,
        real_fr_mm_s, active_extruder);

    planner.synchronize();

    if (is_home_dir) {

#if HOMING_Z_WITH_PROBE && QUIET_PROBING
        if (axis == Z_AXIS)
            probing_pause(false);
#endif

        endstops.validate_homing_move();

// Re-enable stealthChop if used. Disable diag1 pin on driver.
#if ENABLED(SENSORLESS_HOMING)
        end_sensorless_homing_per_axis(axis, stealth_states);
#endif
    }
}

void home_Marlin(const AxisEnum axis, int dir, bool reset_position) {

    endstops.enable(true);
    // #define CAN_HOME_X true
    // #define CAN_HOME_Y true
    do_homing_move_crash(axis, 1.5f * max_length(axis) * home_dir(axis));
    set_axis_is_at_home(axis);
    sync_plan_position();
    destination[axis] = current_position[axis];
    current_position.pos[axis] = 0;

    endstops.not_homing();
    // line_to_current_position(100);
    planner.synchronize();
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

    display::FillRect(Rect16(0, 0, 10, 10), COLOR_ORANGE);

    // home_to_start_Marlin(X_AXIS);
    home_Marlin(X_AXIS, 1, false);

    const uint32_t m_StartPos_usteps = stepper.position((AxisEnum)X_AXIS);

    // home_Marlin(X_AXIS, -1, false);

    display::FillRect(Rect16(0, 0, 10, 10), COLOR_WHITE);

    marlin_print_resume();

    const int32_t endPos_usteps = stepper.position((AxisEnum)X_AXIS);
    const int32_t length_usteps = endPos_usteps - m_StartPos_usteps;
    const float length_mm = (length_usteps * planner.steps_to_mm[(AxisEnum)X_AXIS]);
    if (182 <= length_mm && length_mm <= 190) {
        display::FillRect(Rect16(0, 0, 10, 10), COLOR_LIME);
    } else {
        display::FillRect(Rect16(0, 0, 10, 10), COLOR_RED);
    }
}
