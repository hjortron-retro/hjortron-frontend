#include "draw.h"
#include <assert.h>
void
draw_centered_text(SDL_Renderer *renderer, TTF_Font *font, SDL_Color color,
                    const char *text, SDL_Rect *dest)
{
    assert(font != NULL);

    SDL_Rect d;
    SDL_Surface *surface;
    SDL_Texture *texture;
    surface = TTF_RenderUTF8_Blended(font, text, color);

    if (dest != NULL)
    {
        d = *dest;
        d.x += (d.w / 2) - (surface->w / 2);
        d.y += (d.h / 2) - (surface->h / 2);
        d.w = surface->w;
        d.h = surface->h;
    }
    else
    {
        int w,h;
        SDL_GetRendererOutputSize(renderer, &w, &h);
        d.w = surface->w;
        d.h = surface->h;
        d.x = (w / 2) - (surface->w / 2);
        d.y = (h / 2) - (surface->h / 2);
    }
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    SDL_RenderCopy(renderer, texture, NULL, &d);
    SDL_DestroyTexture(texture);
}
