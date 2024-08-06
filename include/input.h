#if !defined (INPUT_H)
#define INPUT_H

#include "SDL2/SDL.h"

SDL_bool GetKeyDown(SDL_Scancode code);
SDL_bool GetKeyUp(SDL_Scancode code);

int GetMouseX();
int GetMouseY();

int GetMouseDeltaX();
int GetMouseDeltaY();

SDL_bool GetButtonDown(int mouse_button);
SDL_bool GetButtonUp(int mouse_button);

#endif // INPUT_H