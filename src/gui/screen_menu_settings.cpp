// screen_menu_settings.cpp

#include "screen_menu_settings.hpp"
#include "screen_menus.hpp"
#include "gui.hpp"
#include "config.h"
#include "app.h"
#include "marlin_client.h"
#include "screen_menu.hpp"
#include "cmsis_os.h"
#include "sys.h"
#include "eeprom.h"
#include "eeprom_loadsave.h"
#include "filament_sensor.hpp"
#include "dump.h"
#include "sound.hpp"
#include "WindowMenuItems.hpp"
#include "MItem_menus.hpp"
#include "MItem_tools.hpp"
#include "i18n.h"
#include "Marlin/src/core/serial.h"

/*****************************************************************************/
//MI_FILAMENT_SENSOR
size_t MI_FILAMENT_SENSOR::init_index() const {
    fsensor_t fs = fs_wait_initialized();
    if (fs == fsensor_t::NotConnected) //tried to enable but there is no sensor
    {
        fs_disable();
        no_sensor_msg();
        fs = fsensor_t::Disabled;
    }
    return fs == fsensor_t::Disabled ? 0 : 1;
}
// bool fs_not_connected;

void MI_FILAMENT_SENSOR::CheckDisconnected() {
    fsensor_t fs = fs_wait_initialized();
    if (fs == fsensor_t::NotConnected) { //only way to have this state is that fs just disconnected
        fs_disable();
        index = 0;
        no_sensor_msg();
    }
}

virtual void MI_FILAMENT_SENSOR::OnChange(size_t old_index) {
    old_index == 1 ? fs_disable() : fs_enable();
    fsensor_t fs = fs_wait_initialized();
    if (fs == fsensor_t::NotConnected) //tried to enable but there is no sensor
    {
        fs_disable();
        index = old_index;
        no_sensor_msg();
    }
}
