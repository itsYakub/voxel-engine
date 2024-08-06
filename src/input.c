#include "input.h"

#include "core.h"

extern core_data CORE;

SDL_bool GetKeyDown(SDL_Scancode code) {
    return CORE.input.keyboard.key_state_current[code];
}

SDL_bool GetKeyUp(SDL_Scancode code) {
    return !CORE.input.keyboard.key_state_current[code];
}

int GetMouseX() {
    return CORE.input.mouse.position[0];
}

int GetMouseY() {
    return CORE.input.mouse.position[1];
}

int GetMouseDeltaX() {
    return CORE.input.mouse.position[0] - CORE.input.mouse.position_previous[0];
}

int GetMouseDeltaY() {
    return CORE.input.mouse.position[1] - CORE.input.mouse.position_previous[1];
}

SDL_bool GetButtonDown(int mouse_button) {
    return CORE.input.mouse.mouse_state_current[mouse_button];
}

SDL_bool GetButtonUp(int mouse_button) {
    return !CORE.input.mouse.mouse_state_current[mouse_button];
}
