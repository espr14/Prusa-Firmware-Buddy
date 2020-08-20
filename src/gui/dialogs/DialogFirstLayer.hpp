#pragma once

#include "i18n.h"
#include "DialogStateful.hpp"

class DialogFirstLayer : public DialogStateful<PhasesFirstLayer> {
public:
    DialogFirstLayer(string_view_utf8 name);
};
