/// crash_recovery.h
#pragma once

#include "../../Marlin/src/core/types.h"

void crash_recovery();
void home_axis(const AxisEnum axis, const bool positive_dir, const bool reset_position = false);
