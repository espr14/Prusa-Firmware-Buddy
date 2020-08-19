#include "DialogFirstLayer.hpp"

DialogFirstLayer::DialogG162(string_view_utf8 name)
    : DialogStateful<PhasesG162>(name, Factory()) {}
