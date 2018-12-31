#include "draw.h"
#include <assert.h>

SDL_Surface *
render_text(TTF_Font *font, int style, SDL_Color color, const char *text)
{
    SDL_Surface *surface;
    TTF_SetFontStyle(font, style);
    /* TODO: Lookup surface in cache, render new if not found */
    surface = TTF_RenderUTF8_Blended(font, text, color);
    if (!surface)
    {
        fprintf(stderr, "%s(), %s",__func__, TTF_GetError());
    }
    return surface;
}


void
draw_text(SDL_Renderer *renderer, TTF_Font *font, int style, SDL_Color color,
                    draw_text_align_t align, const char *text, SDL_Rect *dest)
{
    assert(font != NULL);

    SDL_Rect d;
    SDL_Surface *surface;
    SDL_Texture *texture;

    surface = render_text(font, style, color, text);

    if (dest != NULL)
    {
        d = *dest;

        /* Always vertically align text by center */
        d.y += (d.h / 2) - (surface->h / 2);

        if (align == ALIGN_CENTER)
        {
            d.x += (d.w / 2) - (surface->w / 2);
        }
        else if (align == ALIGN_RIGHT)
        {
            d.x += d.w - surface->w;
        }

        d.w = surface->w;
        d.h = surface->h;
    }
    else
    {
        int w,h;
        SDL_GetRendererOutputSize(renderer, &w, &h);

        /* Always vertically align text by center */
        d.y = (h / 2) - (surface->h / 2);

        if (align == ALIGN_CENTER)
        {
            d.x = (w / 2) - (surface->w / 2);
        }
        else if (align == ALIGN_RIGHT)
        {
            d.x = w - surface->w;
        }

        d.w = surface->w;
        d.h = surface->h;
    }

    texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    SDL_RenderCopy(renderer, texture, NULL, &d);
    SDL_DestroyTexture(texture);
}
