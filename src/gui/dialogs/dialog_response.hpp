// dialog_response.hpp
// texts for all types of response any Dialog can return

#pragma once

#include "client_response.hpp" //MAX_RESPONSES
#include <array>

using PhaseTexts = std::array<const char *, MAX_RESPONSES>;

//todo make some automatic checks names vs enum
//list of all button types
class BtnTexts {
    static const std::array<const char *, static_cast<size_t>(Response::_last) + 1> texts;

public:
    static constexpr const char *Get(Response resp) {
        return texts[static_cast<size_t>(resp)];
    }
};

/*****************************************************************************/
// clang-format off
static const PhaseTexts ph_txt_stop          = { BtnTexts::Get(Response::Stop),             BtnTexts::Get(Response::_none), BtnTexts::Get(Response::_none), BtnTexts::Get(Response::_none) };
static const PhaseTexts ph_txt_continue      = { BtnTexts::Get(Response::Continue),         BtnTexts::Get(Response::_none), BtnTexts::Get(Response::_none), BtnTexts::Get(Response::_none) };
static const PhaseTexts ph_txt_none          = { BtnTexts::Get(Response::_none),            BtnTexts::Get(Response::_none), BtnTexts::Get(Response::_none), BtnTexts::Get(Response::_none) };
static const PhaseTexts ph_txt_yesno         = { BtnTexts::Get(Response::Yes),              BtnTexts::Get(Response::No),    BtnTexts::Get(Response::_none), BtnTexts::Get(Response::_none) };
static const PhaseTexts btns_next_unload = { BtnTexts::Get(Response::Next), BtnTexts::Get(Response::Unload), BtnTexts::Get(Response::_none), BtnTexts::Get(Response::_none) };
static const PhaseTexts btns_next_load_unload = { BtnTexts::Get(Response::Next), BtnTexts::Get(Response::Load), BtnTexts::Get(Response::Unload), BtnTexts::Get(Response::_none) };
static const PhaseTexts btn_next = { BtnTexts::Get(Response::Next), BtnTexts::Get(Response::_none), BtnTexts::Get(Response::_none), BtnTexts::Get(Response::_none) };
// clang-format on
/*****************************************************************************/
