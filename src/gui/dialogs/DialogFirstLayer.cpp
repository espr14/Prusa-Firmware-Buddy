#include "DialogFirstLayer.hpp"
#include "dialog_response.hpp"

/*****************************************************************************/
// clang-format off
static const char *txt_none = N_("");
static const char *txt_use_filament_load = N_("To calibrate with  \n"
                                              "currently loaded   \n"
                                              "filament,          \n"
                                              "press NEXT.        \n"
                                              "To load filament,  \n"
                                              "press LOAD.        \n"
                                              "To change filament,\n"
                                              "press UNLOAD.");

static const char *txt_use_filament_no_load = N_("To calibrate with  \n"
                                                 "currently loaded   \n"
                                                 "filament,          \n"
                                                 "press NEXT.        \n"
                                                 "To change filament,\n"
                                                 "press UNLOAD.");

static const char *txt_info_1 = N_("Now, let's calibrate\n"
                                   "the distance       \n"
                                   "between the tip    \n"
                                   "of the nozzle and  \n"
                                   "the print sheet.   ");

static const char *txt_last_value = N_("Do you want to use last set value? "
                                       "Last:  %0.3f.   "
                                       "Default: %0.3f.   "
                                       "Click NO to use default value.");

static const char *txt_info_2 = N_("In the next step, \n"
                                   "use the knob to   \n"
                                   "adjust the nozzle \n"
                                   "height.           \n"
                                   "Check the pictures\n"
                                   "in the handbook   \n"
                                   "for reference.");

static const char *txt_printing = N_("Once the printer   \n"
                                     "starts extruding   \n"
                                     "plastic, adjust    \n"
                                     "the nozzle height  \n"
                                     "by turning the knob\n"
                                     "until the filament \n"
                                     "sticks to the print\n"
                                     "sheet.");

static const char *txt_repeat = N_("Do you want to use\n"
                                   "the current value?\n"
                                   "Current: %0.3f.   \n"
                                   "Default: %0.3f.   \n"
                                   "Click NO to use the default value (recommended)");

static const char *txt_clean_sheet = N_("Clean steel sheet.");

static DialogFirstLayer::States Factory() {
    DialogFirstLayer::States ret = {
        DialogFirstLayer::State { txt_none, ClientResponses::GetResponses(PhasesFirstLayer::_first),  ph_txt_none },
        DialogFirstLayer::State { txt_use_filament_load, ClientResponses::GetResponses(PhasesFirstLayer::UseFilamentLoad),  btns_next_load_unload },
        DialogFirstLayer::State { txt_use_filament_no_load, ClientResponses::GetResponses(PhasesFirstLayer::UseFilamentNoLoad),  btns_next_load_unload },
        DialogFirstLayer::State { txt_none, ClientResponses::GetResponses(PhasesFirstLayer::SelectFilament),  ph_txt_none }, /// pick filament
        DialogFirstLayer::State { txt_info_1, ClientResponses::GetResponses(PhasesFirstLayer::Info1),  btn_next },
        DialogFirstLayer::State { txt_last_value, ClientResponses::GetResponses(PhasesFirstLayer::LastValue),  ph_txt_yesno },
        DialogFirstLayer::State { txt_info_2, ClientResponses::GetResponses(PhasesFirstLayer::Info2),  btn_next },
        DialogFirstLayer::State { txt_printing, ClientResponses::GetResponses(PhasesFirstLayer::Preheating),  ph_txt_none },
        DialogFirstLayer::State { txt_printing, ClientResponses::GetResponses(PhasesFirstLayer::MBL),  ph_txt_none },
        DialogFirstLayer::State { txt_printing, ClientResponses::GetResponses(PhasesFirstLayer::Heating),  ph_txt_none },
        DialogFirstLayer::State { txt_printing, ClientResponses::GetResponses(PhasesFirstLayer::Printing),  ph_txt_none },
        DialogFirstLayer::State { txt_repeat, ClientResponses::GetResponses(PhasesFirstLayer::Repeat),  ph_txt_yesno },
        DialogFirstLayer::State { txt_clean_sheet, ClientResponses::GetResponses(PhasesFirstLayer::CleanSheet),  btn_next },
    };
    return ret;
}
// clang-format on
/*****************************************************************************/

DialogFirstLayer::DialogFirstLayer(string_view_utf8 name)
    : DialogStateful<PhasesFirstLayer>(name, Factory()) {}
