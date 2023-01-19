
#ifndef QWY2_HEADER_INPUT_EVENTS_
#define QWY2_HEADER_INPUT_EVENTS_

#include <string_view>
#include <SDL2/SDL.h>

SDL_Keycode keyboard_key_name_to_code(std::string_view key_name);
std::string_view keyboard_key_code_to_name(SDL_Keycode sdl_keycode);

unsigned char mouse_button_name_to_code(std::string_view button_name);
std::string_view mouse_button_code_to_name(unsigned char button_code);

#endif /* QWY2_HEADER_INPUT_EVENTS_ */
