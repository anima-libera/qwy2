
#include "keycode.hpp"
#include <iostream>
#include <array>

using namespace std::literals::string_view_literals;

static std::pair<std::string_view, SDL_Keycode> key_name_and_code_table[] = {
	{"return"sv, SDLK_RETURN},
	{"escape"sv, SDLK_ESCAPE},
	{"backspace"sv, SDLK_BACKSPACE},
	{"tab"sv, SDLK_TAB},
	{"space"sv, SDLK_SPACE},
	{"exclaim"sv, SDLK_EXCLAIM},
	{"quotedbl"sv, SDLK_QUOTEDBL},
	{"hash"sv, SDLK_HASH},
	{"percent"sv, SDLK_PERCENT},
	{"dollar"sv, SDLK_DOLLAR},
	{"ampersand"sv, SDLK_AMPERSAND},
	{"quote"sv, SDLK_QUOTE},
	{"leftparen"sv, SDLK_LEFTPAREN},
	{"rightparen"sv, SDLK_RIGHTPAREN},
	{"asterisk"sv, SDLK_ASTERISK},
	{"plus"sv, SDLK_PLUS},
	{"comma"sv, SDLK_COMMA},
	{"minus"sv, SDLK_MINUS},
	{"period"sv, SDLK_PERIOD},
	{"slash"sv, SDLK_SLASH},
	{"0"sv, SDLK_0},
	{"1"sv, SDLK_1},
	{"2"sv, SDLK_2},
	{"3"sv, SDLK_3},
	{"4"sv, SDLK_4},
	{"5"sv, SDLK_5},
	{"6"sv, SDLK_6},
	{"7"sv, SDLK_7},
	{"8"sv, SDLK_8},
	{"9"sv, SDLK_9},
	{"colon"sv, SDLK_COLON},
	{"semicolon"sv, SDLK_SEMICOLON},
	{"less"sv, SDLK_LESS},
	{"equals"sv, SDLK_EQUALS},
	{"greater"sv, SDLK_GREATER},
	{"question"sv, SDLK_QUESTION},
	{"at"sv, SDLK_AT},
	{"leftbracket"sv, SDLK_LEFTBRACKET},
	{"backslash"sv, SDLK_BACKSLASH},
	{"rightbracket"sv, SDLK_RIGHTBRACKET},
	{"caret"sv, SDLK_CARET},
	{"underscore"sv, SDLK_UNDERSCORE},
	{"backquote"sv, SDLK_BACKQUOTE},
	{"a"sv, SDLK_a},
	{"b"sv, SDLK_b},
	{"c"sv, SDLK_c},
	{"d"sv, SDLK_d},
	{"e"sv, SDLK_e},
	{"f"sv, SDLK_f},
	{"g"sv, SDLK_g},
	{"h"sv, SDLK_h},
	{"i"sv, SDLK_i},
	{"j"sv, SDLK_j},
	{"k"sv, SDLK_k},
	{"l"sv, SDLK_l},
	{"m"sv, SDLK_m},
	{"n"sv, SDLK_n},
	{"o"sv, SDLK_o},
	{"p"sv, SDLK_p},
	{"q"sv, SDLK_q},
	{"r"sv, SDLK_r},
	{"s"sv, SDLK_s},
	{"t"sv, SDLK_t},
	{"u"sv, SDLK_u},
	{"v"sv, SDLK_v},
	{"w"sv, SDLK_w},
	{"x"sv, SDLK_x},
	{"y"sv, SDLK_y},
	{"z"sv, SDLK_z},
	{"capslock"sv, SDLK_CAPSLOCK},
	{"f1"sv, SDLK_F1},
	{"f2"sv, SDLK_F2},
	{"f3"sv, SDLK_F3},
	{"f4"sv, SDLK_F4},
	{"f5"sv, SDLK_F5},
	{"f6"sv, SDLK_F6},
	{"f7"sv, SDLK_F7},
	{"f8"sv, SDLK_F8},
	{"f9"sv, SDLK_F9},
	{"f10"sv, SDLK_F10},
	{"f11"sv, SDLK_F11},
	{"f12"sv, SDLK_F12},
	{"printscreen"sv, SDLK_PRINTSCREEN},
	{"scrolllock"sv, SDLK_SCROLLLOCK},
	{"pause"sv, SDLK_PAUSE},
	{"insert"sv, SDLK_INSERT},
	{"home"sv, SDLK_HOME},
	{"pageup"sv, SDLK_PAGEUP},
	{"delete"sv, SDLK_DELETE},
	{"end"sv, SDLK_END},
	{"pagedown"sv, SDLK_PAGEDOWN},
	{"right"sv, SDLK_RIGHT},
	{"left"sv, SDLK_LEFT},
	{"down"sv, SDLK_DOWN},
	{"up"sv, SDLK_UP},
	{"numlockclear"sv, SDLK_NUMLOCKCLEAR},
	{"kp_divide"sv, SDLK_KP_DIVIDE},
	{"kp_multiply"sv, SDLK_KP_MULTIPLY},
	{"kp_minus"sv, SDLK_KP_MINUS},
	{"kp_plus"sv, SDLK_KP_PLUS},
	{"kp_enter"sv, SDLK_KP_ENTER},
	{"kp_1"sv, SDLK_KP_1},
	{"kp_2"sv, SDLK_KP_2},
	{"kp_3"sv, SDLK_KP_3},
	{"kp_4"sv, SDLK_KP_4},
	{"kp_5"sv, SDLK_KP_5},
	{"kp_6"sv, SDLK_KP_6},
	{"kp_7"sv, SDLK_KP_7},
	{"kp_8"sv, SDLK_KP_8},
	{"kp_9"sv, SDLK_KP_9},
	{"kp_0"sv, SDLK_KP_0},
	{"kp_period"sv, SDLK_KP_PERIOD},
	{"application"sv, SDLK_APPLICATION},
	{"power"sv, SDLK_POWER},
	{"kp_equals"sv, SDLK_KP_EQUALS},
	{"f13"sv, SDLK_F13},
	{"f14"sv, SDLK_F14},
	{"f15"sv, SDLK_F15},
	{"f16"sv, SDLK_F16},
	{"f17"sv, SDLK_F17},
	{"f18"sv, SDLK_F18},
	{"f19"sv, SDLK_F19},
	{"f20"sv, SDLK_F20},
	{"f21"sv, SDLK_F21},
	{"f22"sv, SDLK_F22},
	{"f23"sv, SDLK_F23},
	{"f24"sv, SDLK_F24},
	{"execute"sv, SDLK_EXECUTE},
	{"help"sv, SDLK_HELP},
	{"menu"sv, SDLK_MENU},
	{"select"sv, SDLK_SELECT},
	{"stop"sv, SDLK_STOP},
	{"again"sv, SDLK_AGAIN},
	{"undo"sv, SDLK_UNDO},
	{"cut"sv, SDLK_CUT},
	{"copy"sv, SDLK_COPY},
	{"paste"sv, SDLK_PASTE},
	{"find"sv, SDLK_FIND},
	{"mute"sv, SDLK_MUTE},
	{"volumeup"sv, SDLK_VOLUMEUP},
	{"volumedown"sv, SDLK_VOLUMEDOWN},
	{"kp_comma"sv, SDLK_KP_COMMA},
	{"kp_equalsas400"sv, SDLK_KP_EQUALSAS400},
	{"alterase"sv, SDLK_ALTERASE},
	{"sysreq"sv, SDLK_SYSREQ},
	{"cancel"sv, SDLK_CANCEL},
	{"clear"sv, SDLK_CLEAR},
	{"prior"sv, SDLK_PRIOR},
	{"return2"sv, SDLK_RETURN2},
	{"separator"sv, SDLK_SEPARATOR},
	{"out"sv, SDLK_OUT},
	{"oper"sv, SDLK_OPER},
	{"clearagain"sv, SDLK_CLEARAGAIN},
	{"crsel"sv, SDLK_CRSEL},
	{"exsel"sv, SDLK_EXSEL},
	{"kp_00"sv, SDLK_KP_00},
	{"kp_000"sv, SDLK_KP_000},
	{"thousandsseparator"sv, SDLK_THOUSANDSSEPARATOR},
	{"decimalseparator"sv, SDLK_DECIMALSEPARATOR},
	{"currencyunit"sv, SDLK_CURRENCYUNIT},
	{"currencysubunit"sv, SDLK_CURRENCYSUBUNIT},
	{"kp_leftparen"sv, SDLK_KP_LEFTPAREN},
	{"kp_rightparen"sv, SDLK_KP_RIGHTPAREN},
	{"kp_leftbrace"sv, SDLK_KP_LEFTBRACE},
	{"kp_rightbrace"sv, SDLK_KP_RIGHTBRACE},
	{"kp_tab"sv, SDLK_KP_TAB},
	{"kp_backspace"sv, SDLK_KP_BACKSPACE},
	{"kp_a"sv, SDLK_KP_A},
	{"kp_b"sv, SDLK_KP_B},
	{"kp_c"sv, SDLK_KP_C},
	{"kp_d"sv, SDLK_KP_D},
	{"kp_e"sv, SDLK_KP_E},
	{"kp_f"sv, SDLK_KP_F},
	{"kp_xor"sv, SDLK_KP_XOR},
	{"kp_power"sv, SDLK_KP_POWER},
	{"kp_percent"sv, SDLK_KP_PERCENT},
	{"kp_less"sv, SDLK_KP_LESS},
	{"kp_greater"sv, SDLK_KP_GREATER},
	{"kp_ampersand"sv, SDLK_KP_AMPERSAND},
	{"kp_dblampersand"sv, SDLK_KP_DBLAMPERSAND},
	{"kp_verticalbar"sv, SDLK_KP_VERTICALBAR},
	{"kp_dblverticalbar"sv, SDLK_KP_DBLVERTICALBAR},
	{"kp_colon"sv, SDLK_KP_COLON},
	{"kp_hash"sv, SDLK_KP_HASH},
	{"kp_space"sv, SDLK_KP_SPACE},
	{"kp_at"sv, SDLK_KP_AT},
	{"kp_exclam"sv, SDLK_KP_EXCLAM},
	{"kp_memstore"sv, SDLK_KP_MEMSTORE},
	{"kp_memrecall"sv, SDLK_KP_MEMRECALL},
	{"kp_memclear"sv, SDLK_KP_MEMCLEAR},
	{"kp_memadd"sv, SDLK_KP_MEMADD},
	{"kp_memsubtract"sv, SDLK_KP_MEMSUBTRACT},
	{"kp_memmultiply"sv, SDLK_KP_MEMMULTIPLY},
	{"kp_memdivide"sv, SDLK_KP_MEMDIVIDE},
	{"kp_plusminus"sv, SDLK_KP_PLUSMINUS},
	{"kp_clear"sv, SDLK_KP_CLEAR},
	{"kp_clearentry"sv, SDLK_KP_CLEARENTRY},
	{"kp_binary"sv, SDLK_KP_BINARY},
	{"kp_octal"sv, SDLK_KP_OCTAL},
	{"kp_decimal"sv, SDLK_KP_DECIMAL},
	{"kp_hexadecimal"sv, SDLK_KP_HEXADECIMAL},
	{"lctrl"sv, SDLK_LCTRL},
	{"lshift"sv, SDLK_LSHIFT},
	{"lalt"sv, SDLK_LALT},
	{"lgui"sv, SDLK_LGUI},
	{"rctrl"sv, SDLK_RCTRL},
	{"rshift"sv, SDLK_RSHIFT},
	{"ralt"sv, SDLK_RALT},
	{"rgui"sv, SDLK_RGUI},
	{"mode"sv, SDLK_MODE},
	{"audionext"sv, SDLK_AUDIONEXT},
	{"audioprev"sv, SDLK_AUDIOPREV},
	{"audiostop"sv, SDLK_AUDIOSTOP},
	{"audioplay"sv, SDLK_AUDIOPLAY},
	{"audiomute"sv, SDLK_AUDIOMUTE},
	{"mediaselect"sv, SDLK_MEDIASELECT},
	{"www"sv, SDLK_WWW},
	{"mail"sv, SDLK_MAIL},
	{"calculator"sv, SDLK_CALCULATOR},
	{"computer"sv, SDLK_COMPUTER},
	{"ac_search"sv, SDLK_AC_SEARCH},
	{"ac_home"sv, SDLK_AC_HOME},
	{"ac_back"sv, SDLK_AC_BACK},
	{"ac_forward"sv, SDLK_AC_FORWARD},
	{"ac_stop"sv, SDLK_AC_STOP},
	{"ac_refresh"sv, SDLK_AC_REFRESH},
	{"ac_bookmarks"sv, SDLK_AC_BOOKMARKS},
	{"brightnessdown"sv, SDLK_BRIGHTNESSDOWN},
	{"brightnessup"sv, SDLK_BRIGHTNESSUP},
	{"displayswitch"sv, SDLK_DISPLAYSWITCH},
	{"kbdillumtoggle"sv, SDLK_KBDILLUMTOGGLE},
	{"kbdillumdown"sv, SDLK_KBDILLUMDOWN},
	{"kbdillumup"sv, SDLK_KBDILLUMUP},
	{"eject"sv, SDLK_EJECT},
	{"sleep"sv, SDLK_SLEEP},
	{"app1"sv, SDLK_APP1},
	{"app2"sv, SDLK_APP2},
	{"audiorewind"sv, SDLK_AUDIOREWIND},
	{"audiofastforward"sv, SDLK_AUDIOFASTFORWARD},
};

SDL_Keycode key_name_to_sdl_keycode(std::string_view key_name)
{
	for (auto& key_name_and_code : key_name_and_code_table)
	{
		if (key_name_and_code.first == key_name)
		{
			return key_name_and_code.second;
		}
	}
	
	std::cout << "\x1b[31m" << "Error: " <<
		"No key named \"" << key_name << "\" found."
		<< "\x1b[39m" << std::endl;
	return -1;
}

std::string_view key_sdl_keycode_to_name(SDL_Keycode sdl_keycode)
{
	for (auto& key_name_and_code : key_name_and_code_table)
	{
		if (key_name_and_code.second == sdl_keycode)
		{
			return key_name_and_code.first;
		}
	}
	
	std::cout << "\x1b[31m" << "Error: " <<
		"No key code " << sdl_keycode << " found."
		<< "\x1b[39m" << std::endl;
	return "unkown"sv;
}

unsigned char mouse_button_name_to_code(std::string_view button_name)
{
	if (button_name == "left")
	{
		return SDL_BUTTON_LEFT;
	}
	else if (button_name == "middle")
	{
		return SDL_BUTTON_MIDDLE;
	}
	else if (button_name == "right")
	{
		return SDL_BUTTON_RIGHT;
	}
	else if (button_name == "x1")
	{
		return SDL_BUTTON_X1;
	}
	else if (button_name == "x2")
	{
		return SDL_BUTTON_X2;
	}
	else
	{
		std::cout << "\x1b[31m" << "Error: " <<
			"No mouse button named \"" << button_name << "\" found."
			<< "\x1b[39m" << std::endl;
		return 255;
	}
}

std::string_view mouse_button_code_to_name(unsigned char button_code)
{
	if (button_code == SDL_BUTTON_LEFT)
	{
		return "left"sv;
	}
	else if (button_code == SDL_BUTTON_MIDDLE)
	{
		return "middle"sv;
	}
	else if (button_code == SDL_BUTTON_RIGHT)
	{
		return "right"sv;
	}
	else if (button_code == SDL_BUTTON_X1)
	{
		return "x1"sv;
	}
	else if (button_code == SDL_BUTTON_X2)
	{
		return "x2"sv;
	}
	else
	{
		std::cout << "\x1b[31m" << "Error: " <<
			"No mouse button code \"" << button_code << "\" found."
			<< "\x1b[39m" << std::endl;
		return "unkown"sv;
	}
}
