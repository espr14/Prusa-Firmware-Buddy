/// crash_recovery.h
#pragma once

#include "../../Marlin/src/core/types.h"
#include "../../Marlin/src/module/planner.h"

void crash_recovery();
void home_axis(const AxisEnum axis, const bool positive_dir, const bool reset_position = false);

void crash_quick_stop(uint8_t *buffer_pointers, block_t *buffer, abce_pos_t &machine, xyze_pos_t &planned);
void restore_planner_after_crash(uint8_t *buffer_pointers, block_t *buffer, abce_pos_t &machine, xyze_pos_t &planned);
