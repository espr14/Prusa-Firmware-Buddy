#include "DialogFirstLayer.hpp"
#include "dialog_response.hpp"

/*****************************************************************************/
// clang-format off
static const char *txt_none = N_("");
static const char *txt_use_filament = N_("Use loaded filament ...");

static DialogFirstLayer::States Factory() {
    DialogFirstLayer::States ret = {
        DialogFirstLayer::State { txt_use_filament, ClientResponses::GetResponses(PhasesFirstLayer::UseFilament),  btns_next_load_unload },
    };
    return ret;
}
// clang-format on
/*****************************************************************************/
