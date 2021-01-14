/// crash_recovery.cpp

#include "crash_recovery.h"
#include "display.h"
#include "marlin_client.h"
#include "gui.hpp"

#include "../Marlin/src/module/endstops.h"
#include "../../Marlin/src/module/planner.h"
#include "../../Marlin/src/module/stepper.h"

void do_homing_move(const AxisEnum axis, const float distance, const feedRate_t fr_mm_s = 0.0);

void home_Marlin(const AxisEnum axis, int dir, bool reset_position) {
    endstops.enable(true);

#define CAN_HOME_X true
#define CAN_HOME_Y true

    do_homing_move(axis, 1.5f * max_length(axis) * dir);
    if (reset_position) {
        set_axis_is_at_home(axis);
        sync_plan_position();
    }
    destination[axis] = current_position[axis];
    endstops.not_homing();
}

void home_to_start_Marlin(AxisEnum axis) {
    endstops.enable(true);
    homeaxis(axis);
    endstops.not_homing();
    current_position.pos[axis] = 0;
    line_to_current_position(100);
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
    display::FillRect(Rect16(0, 0, 10, 10), COLOR_RED);
    marlin_print_pause();
    while (marlin_vars()->print_state != mpsPaused)
        gui_loop();

    display::FillRect(Rect16(0, 0, 10, 10), COLOR_ORANGE);

    home_Marlin(X_AXIS, 1, false);

    const uint32_t m_StartPos_usteps = stepper.position((AxisEnum)X_AXIS);

    home_Marlin(X_AXIS, -1, false);

    const int32_t endPos_usteps = stepper.position((AxisEnum)X_AXIS);
    const int32_t length_usteps = endPos_usteps - m_StartPos_usteps;
    const float length_mm = (length_usteps * planner.steps_to_mm[(AxisEnum)X_AXIS]);
    if (182 <= length_mm && length_mm <= 190) {
        display::FillRect(Rect16(0, 0, 10, 10), COLOR_LIME);
    } else {
        display::FillRect(Rect16(0, 0, 10, 10), COLOR_RED);
    }

    marlin_print_resume();
}
