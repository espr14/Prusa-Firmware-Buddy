#pragma once

#include "i18n.h"
#include "DialogStateful.hpp"

class DialogFirstLayer : public DialogStateful<PhasesFirstLayer> {
public:
    DialogFirstLayer();
};
