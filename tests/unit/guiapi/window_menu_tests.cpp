/// str_utils tests

#define CATCH_CONFIG_MAIN // This tells Catch to provide a main() - only do this in one cpp file
#include "catch2/catch.hpp"

using Catch::Matchers::Equals;

#include "window_menu.hpp"
#include "i18n.h"
#include "WinMenuContainer.hpp"

//WI_LABEL
class WI_LABEL_t : public IWindowMenuItem {
public:
    WI_LABEL_t(string_view_utf8 label, uint16_t id_icon = 0, bool enabled = true, bool hidden = false);
    virtual bool Change(int dif) override;
};

class MI_RETURN : public WI_LABEL_t {
    static constexpr const char *const label = N_("Return");

public:
    MI_RETURN()
        : WI_LABEL_t(_(label), uint16_t(0), true, false) {
    }
};

TEST_CASE("Window menu", "[window_menu]") {
    WinMenuContainer<MI_RETURN> container;

    window_menu_t menu(nullptr, Rect16(0, 0, 240, 320), &container);

    SECTION("Move to 2") {
        menu.Increment(2);
        CHECK(menu.GetIndex() == 2);
    }

    SECTION("Move out of scope") {
        menu.Increment(100);
        CHECK(menu.GetIndex() == menu.GetCount() - 1);
    }
}
