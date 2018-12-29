#ifndef _draw_h
#define _draw_h

#include <SDL.h>
#include <SDL_ttf.h>

void draw_centered_text(SDL_Renderer *renderer,
                        TTF_Font *font, SDL_Color color,
                        const char *text, SDL_Rect *dest);

#endif /* draw */