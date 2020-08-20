#include "DialogFirstLayer.hpp"
#include "dialog_response.hpp"

/*****************************************************************************/
// clang-format off
static const char *txt_none = N_("");
static const char *txt_use_filament = N_("Use loaded filament ...");

static DialogFirstLayer::States Factory() {
    DialogFirstLayer::States ret = {
        DialogFirstLayer::State { txt_use_filament, ClientResponses::GetResponses(PhasesFirstLayer::_first),  ph_txt_none },
        DialogFirstLayer::State { txt_use_filament, ClientResponses::GetResponses(PhasesFirstLayer::UseFilament),  btns_next_load_unload },
        DialogFirstLayer::State { txt_use_filament, ClientResponses::GetResponses(PhasesFirstLayer::SelectFilament),  ph_txt_none },
        DialogFirstLayer::State { txt_use_filament, ClientResponses::GetResponses(PhasesFirstLayer::Info1),  ph_txt_none },
        DialogFirstLayer::State { txt_use_filament, ClientResponses::GetResponses(PhasesFirstLayer::Info2),  ph_txt_none },
        DialogFirstLayer::State { txt_use_filament, ClientResponses::GetResponses(PhasesFirstLayer::Preheating),  ph_txt_none },
        DialogFirstLayer::State { txt_use_filament, ClientResponses::GetResponses(PhasesFirstLayer::MBL),  ph_txt_none },
        DialogFirstLayer::State { txt_use_filament, ClientResponses::GetResponses(PhasesFirstLayer::Printing),  ph_txt_none },
        DialogFirstLayer::State { txt_use_filament, ClientResponses::GetResponses(PhasesFirstLayer::Repeat),  ph_txt_none },
        DialogFirstLayer::State { txt_use_filament, ClientResponses::GetResponses(PhasesFirstLayer::CleanSheet),  ph_txt_none },
        DialogFirstLayer::State { txt_use_filament, ClientResponses::GetResponses(PhasesFirstLayer::LastValue),  ph_txt_none },
        DialogFirstLayer::State { txt_use_filament, ClientResponses::GetResponses(PhasesFirstLayer::UseFilament),  ph_txt_none }

    };
    return ret;
}
// clang-format on
/*****************************************************************************/

DialogFirstLayer::DialogFirstLayer(string_view_utf8 name)
    : DialogStateful<PhasesFirstLayer>(name, Factory()) {}
