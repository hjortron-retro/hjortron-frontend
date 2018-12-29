#ifndef _draw_h
#define _draw_h

#include <SDL.h>
#include <SDL_ttf.h>

typedef enum draw_text_align_t
{
    ALIGN_LEFT,
    ALIGN_CENTER,
    ALIGN_RIGHT
} draw_text_align_t;

void draw_text(SDL_Renderer *renderer,
                TTF_Font *font, SDL_Color color,
                draw_text_align_t align,
                const char *text, SDL_Rect *dest);

#endif /* draw */