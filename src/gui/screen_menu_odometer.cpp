// screen_menu_odometer.cpp

#include <stdlib.h>

#include "cmath_ext.h"
#include "screen_menus.hpp"
#include "screen_menu.hpp"
#include "i18n.h"
#include "odometer.hpp"
#include "MItem_tools.hpp"
#include "DialogMoveZ.hpp"

using MenuContainer = WinMenuContainer<MI_RETURN>;

class ScreenMenuOdometer : public AddSuperWindow<screen_t> {
    static constexpr const char *label = N_("STATISTICS");
    static const constexpr char *x_text = N_("X-axis");
    static const constexpr char *y_text = N_("Y-axis");
    static const constexpr char *z_text = N_("Z-axis");
    static const constexpr char *e_text = N_("Filament");
    static const constexpr char *t_text = N_("Print Time");
    static const constexpr char *val_format = "%0.1f m"; // do not translate
    static const constexpr char *time_format = "%u s";   // do not translate

    MenuContainer container;
    window_menu_t menu;
    window_header_t header;
    window_text_t x_txt;
    window_text_t y_txt;
    window_text_t z_txt;
    window_text_t e_txt;
    window_text_t t_txt;
    window_numb_t x_val;
    window_numb_t y_val;
    window_numb_t z_val;
    window_numb_t e_val;
    window_numb_t t_val;

    static float getVal(Odometer_s::axis_t axis) {
        Odometer_s::instance().force_to_eeprom();
        return Odometer_s::instance().get(axis) * .001f;
    }
    static uint32_t getTime() {
        Odometer_s::instance().force_to_eeprom();
        return Odometer_s::instance().get_time();
    }

    void windowEvent(EventLock /*has private ctor*/, window_t *sender, GUI_event_t event, void *param) {
        if (event == GUI_event_t::HELD_RELEASED) {
            DialogMoveZ::Show();
            return;
        }

        SuperWindowEvent(sender, event, param);
    }

public:
    ScreenOdometer()
        : OdometerScreen(_(label)) {
        EnableLongHoldScreenAction();
        Odometer_s::instance().force_to_eeprom();
        Item<MI_ODOMETER_DIST_X>().UpdateValue(Odometer_s::instance().get_from_eeprom(Odometer_s::axis_t::X));
        Item<MI_ODOMETER_DIST_Y>().UpdateValue(Odometer_s::instance().get_from_eeprom(Odometer_s::axis_t::Y));
        Item<MI_ODOMETER_DIST_Z>().UpdateValue(Odometer_s::instance().get_from_eeprom(Odometer_s::axis_t::Z));
        Item<MI_ODOMETER_DIST_E>().UpdateValue(Odometer_s::instance().get_from_eeprom(Odometer_s::axis_t::E));
        Item<MI_ODOMETER_TIME>().UpdateValue(Odometer_s::instance().get_time());
    }
};

ScreenFactory::UniquePtr GetScreenMenuOdometer() {
    return ScreenFactory::Screen<ScreenOdometer>();
}
