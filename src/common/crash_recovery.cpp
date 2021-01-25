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

#define _CAN_HOME(A) \
    (axis == _AXIS(A) && ((A##_MIN_PIN > -1 && A##_HOME_DIR < 0) || (A##_MAX_PIN > -1 && A##_HOME_DIR > 0)))
#if X_SPI_SENSORLESS
    #define CAN_HOME_X true
#else
    #define CAN_HOME_X _CAN_HOME(X)
#endif
#if Y_SPI_SENSORLESS
    #define CAN_HOME_Y true
#else
    #define CAN_HOME_Y _CAN_HOME(Y)
#endif
#if Z_SPI_SENSORLESS
    #define CAN_HOME_Z true
#else
    #define CAN_HOME_Z _CAN_HOME(Z)
#endif
    if (!CAN_HOME_X && !CAN_HOME_Y && !CAN_HOME_Z)
        return;

    const int axis_home_dir = (home_dir(axis));

// Homing Z towards the bed? Deploy the Z probe or endstop.
#if HOMING_Z_WITH_PROBE
    if (axis == Z_AXIS && DEPLOY_PROBE())
        return;
#endif

// Set flags for X, Y, Z motor locking
#if HAS_EXTRA_ENDSTOPS
    switch (axis) {
    #if ENABLED(X_DUAL_ENDSTOPS)
    case X_AXIS:
    #endif
    #if ENABLED(Y_DUAL_ENDSTOPS)
    case Y_AXIS:
    #endif
    #if Z_MULTI_ENDSTOPS
    case Z_AXIS:
    #endif
        stepper.set_separate_multi_axis(true);
    default:
        break;
    }
#endif

    // Fast move towards endstop until triggered

#if HOMING_Z_WITH_PROBE && ENABLED(BLTOUCH)
    if (axis == Z_AXIS && bltouch.deploy())
        return; // The initial DEPLOY
#endif

    do_homing_move(axis, 1.5f * max_length(axis) * axis_home_dir);

#if HOMING_Z_WITH_PROBE && ENABLED(BLTOUCH) && DISABLED(BLTOUCH_HS_MODE)
    if (axis == Z_AXIS)
        bltouch.stow(); // Intermediate STOW (in LOW SPEED MODE)
#endif

    // When homing Z with probe respect probe clearance
    const float bump = axis_home_dir * (
#if HOMING_Z_WITH_PROBE
                           (axis == Z_AXIS && (Z_HOME_BUMP_MM)) ? _MAX(Z_CLEARANCE_BETWEEN_PROBES, Z_HOME_BUMP_MM) :
#endif
                                                                home_bump_mm(axis));

    // If a second homing move is configured...
    if (bump) {
        // Move away from the endstop by the axis HOME_BUMP_MM
        do_homing_move(axis, -bump
#if HOMING_Z_WITH_PROBE
            ,
            MMM_TO_MMS(axis == Z_AXIS ? Z_PROBE_SPEED_FAST : 0)
#endif
        );

        // Slow move towards endstop until triggered

#if HOMING_Z_WITH_PROBE && ENABLED(BLTOUCH) && DISABLED(BLTOUCH_HS_MODE)
        if (axis == Z_AXIS && bltouch.deploy())
            return; // Intermediate DEPLOY (in LOW SPEED MODE)
#endif
        do_homing_move(axis, 2 * bump, get_homing_bump_feedrate(axis));

#if HOMING_Z_WITH_PROBE && ENABLED(BLTOUCH)
        if (axis == Z_AXIS)
            bltouch.stow(); // The final STOW
#endif
    }

#if HAS_EXTRA_ENDSTOPS
    const bool pos_dir = axis_home_dir > 0;
    #if ENABLED(X_DUAL_ENDSTOPS)
    if (axis == X_AXIS) {
        const float adj = ABS(endstops.x2_endstop_adj);
        if (adj) {
            if (pos_dir ? (endstops.x2_endstop_adj > 0) : (endstops.x2_endstop_adj < 0))
                stepper.set_x_lock(true);
            else
                stepper.set_x2_lock(true);
            do_homing_move(axis, pos_dir ? -adj : adj);
            stepper.set_x_lock(false);
            stepper.set_x2_lock(false);
        }
    }
    #endif
    #if ENABLED(Y_DUAL_ENDSTOPS)
    if (axis == Y_AXIS) {
        const float adj = ABS(endstops.y2_endstop_adj);
        if (adj) {
            if (pos_dir ? (endstops.y2_endstop_adj > 0) : (endstops.y2_endstop_adj < 0))
                stepper.set_y_lock(true);
            else
                stepper.set_y2_lock(true);
            do_homing_move(axis, pos_dir ? -adj : adj);
            stepper.set_y_lock(false);
            stepper.set_y2_lock(false);
        }
    }
    #endif
    #if ENABLED(Z_DUAL_ENDSTOPS)
    if (axis == Z_AXIS) {
        const float adj = ABS(endstops.z2_endstop_adj);
        if (adj) {
            if (pos_dir ? (endstops.z2_endstop_adj > 0) : (endstops.z2_endstop_adj < 0))
                stepper.set_z_lock(true);
            else
                stepper.set_z2_lock(true);
            do_homing_move(axis, pos_dir ? -adj : adj);
            stepper.set_z_lock(false);
            stepper.set_z2_lock(false);
        }
    }
    #endif
    #if ENABLED(Z_TRIPLE_ENDSTOPS)
    if (axis == Z_AXIS) {
        // we push the function pointers for the stepper lock function into an array
        void (*lock[3])(bool) = { &stepper.set_z_lock, &stepper.set_z2_lock, &stepper.set_z3_lock };
        float adj[3] = { 0, endstops.z2_endstop_adj, endstops.z3_endstop_adj };

        void (*tempLock)(bool);
        float tempAdj;

        // manual bubble sort by adjust value
        if (adj[1] < adj[0]) {
            tempLock = lock[0], tempAdj = adj[0];
            lock[0] = lock[1], adj[0] = adj[1];
            lock[1] = tempLock, adj[1] = tempAdj;
        }
        if (adj[2] < adj[1]) {
            tempLock = lock[1], tempAdj = adj[1];
            lock[1] = lock[2], adj[1] = adj[2];
            lock[2] = tempLock, adj[2] = tempAdj;
        }
        if (adj[1] < adj[0]) {
            tempLock = lock[0], tempAdj = adj[0];
            lock[0] = lock[1], adj[0] = adj[1];
            lock[1] = tempLock, adj[1] = tempAdj;
        }

        if (pos_dir) {
            // normalize adj to smallest value and do the first move
            (*lock[0])(true);
            do_homing_move(axis, adj[1] - adj[0]);
            // lock the second stepper for the final correction
            (*lock[1])(true);
            do_homing_move(axis, adj[2] - adj[1]);
        } else {
            (*lock[2])(true);
            do_homing_move(axis, adj[1] - adj[2]);
            (*lock[1])(true);
            do_homing_move(axis, adj[0] - adj[1]);
        }

        stepper.set_z_lock(false);
        stepper.set_z2_lock(false);
        stepper.set_z3_lock(false);
    }
    #endif

    // Reset flags for X, Y, Z motor locking
    switch (axis) {
    #if ENABLED(X_DUAL_ENDSTOPS)
    case X_AXIS:
    #endif
    #if ENABLED(Y_DUAL_ENDSTOPS)
    case Y_AXIS:
    #endif
    #if Z_MULTI_ENDSTOPS
    case Z_AXIS:
    #endif
        stepper.set_separate_multi_axis(false);
    default:
        break;
    }
#endif

    set_axis_is_at_home(axis);
    sync_plan_position();

    destination[axis] = current_position[axis];

// Put away the Z probe
#if HOMING_Z_WITH_PROBE
    if (axis == Z_AXIS && STOW_PROBE())
        return;
#endif

#ifdef HOMING_BACKOFF_MM
    constexpr xyz_float_t endstop_backoff = HOMING_BACKOFF_MM;
    const float backoff_mm = endstop_backoff[
    #if ENABLED(DELTA)
        Z_AXIS
    #else
        axis
    #endif
    ];
    if (backoff_mm) {
        current_position[axis] -= ABS(backoff_mm) * axis_home_dir;
        line_to_current_position(
    #if HOMING_Z_WITH_PROBE
            (axis == Z_AXIS) ? MMM_TO_MMS(Z_PROBE_SPEED_FAST) :
    #endif
                             homing_feedrate(axis));
    }
#endif

// Clear retracted status if homing the Z axis
#if ENABLED(FWRETRACT)
    if (axis == Z_AXIS)
        fwretract.current_hop = 0.0;
#endif

    endstops.not_homing();
    current_position.pos[axis] = 0;
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
