/*
 * This file is part of hjortron-frontend.
 *
 * Copyright 2019 Henrik Andersson <henrik.4e@gmail.com>
 *
 * hjortron-frontend is free software: you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * hjortron-frontend is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with hjortron-frontend.  If not, see
 * <https://www.gnu.org/licenses/>.
 *
 */

#include <asoundlib.h>

#include "engine.h"
#include "logger.h"
#include "overlay.h"
#include "draw.h"
#include "error.h"

/* https://iconos8.es/line-awesome/cheatsheet */
#define BATTERY_0 "\uf132"
#define BATTERY_1 "\uf133"
#define BATTERY_2 "\uf134"
#define BATTERY_3 "\uf135"
#define BATTERY_4 "\uf136"

#define STAR "\uf318"
#define LIGHT_BULB "\u266"
#define SPEAKER "\uf375"

#define MAX_CTRL_VALUE 20

static SDL_Color overlay_color = {0xff, 0xff, 0xff, 0xff};

char *icon_to_codepoint[] = {
    "\uf132", /* ICON_BATTERY_0 */
    "\uf133",
    "\uf134",
    "\uf135",
    "\uf136", /* ICON_BATTERY_4 */

    "\uf266", /* ICON_LIGHT_BULB */

    "\uf374", /* ICON_SPEAKER_MUTE */
    "\uf373", /* ICON_SPEAKER_LOW */
    "\uf375", /* ICON_SPEAKER_HIGH */
};

static void
_overlay_alsa_master_volume(overlay_t *overlay)
{
    long min, max;
    snd_mixer_t *h;
    snd_mixer_selem_id_t *sid;
    snd_mixer_elem_t *e;

    snd_mixer_open(&h, 0);
    snd_mixer_attach(h, "default");
    snd_mixer_selem_register(h, NULL, NULL);
    snd_mixer_load(h);

    snd_mixer_selem_id_alloca(&sid);
    snd_mixer_selem_id_set_index(sid, 0);
    snd_mixer_selem_id_set_name(sid, "Master");

    e = snd_mixer_find_selem(h, sid);
    snd_mixer_selem_get_playback_volume_range(e, &min, &max);
    snd_mixer_selem_set_playback_volume_all(e, overlay->volume * max / MAX_CTRL_VALUE);

    snd_mixer_close(h);
}

static void
_overlay_render_bar(SDL_Renderer *renderer, SDL_Rect *d, double value)
{
    int spacing = 2;
    SDL_Rect r;
    SDL_SetRenderDrawColor(renderer, 0xff, 0xff, 0xff, 0xff);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_RenderFillRect(renderer, d);

    r = *d;
    r.x += spacing;
    r.y += spacing;
    r.w -= spacing*2;
    r.h -= spacing*2;
    SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xff);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
    SDL_RenderFillRect(renderer, &r);


    SDL_SetRenderDrawColor(renderer, 0xff, 0xff, 0xff, 0xff);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    r.w = r.w * value;
    SDL_RenderFillRect(renderer, &r);

}

int
overlay_init(overlay_t *overlay, SDL_Renderer *renderer, uint32_t size)
{
    int i;
    SDL_Surface *surface;

    overlay->show = false;
    overlay->button_press_tick = 0;
    overlay->button_release_tick = 0;

    overlay->brightness = SDL_GetWindowBrightness(overlay->engine->window);

    overlay->font = TTF_OpenFont("./line-awesome.ttf", size);
    if (overlay->font == NULL)
    {
        error("overlay", "failed to load icon font");
        return 1;
    }

    for (i = 0; i < ICON_CNT; i++)
    {
        surface = render_text(overlay->font, TTF_STYLE_NORMAL,
                              overlay_color, icon_to_codepoint[i]);
        overlay->icons[i] = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_FreeSurface(surface);
    }

    return 0;
}

void
overlay_render(overlay_t *overlay, SDL_Renderer *renderer)
{
    int w, h;
    int icon;
    SDL_Rect or, d, tmp;

    /* if overlay is hidden, skip */
    if (overlay->show == false)
        return;

    SDL_GetRendererOutputSize(renderer, &w, &h);

    /* overlay rectangle */
    or.x = 0,
    or.y = (h / 2) - ((h/6) / 2);
    or.w = w;
    or.h = (h/6);

    /* render overlay into to texture if dirty */

    /* render texture to screen */

    /* draw background */
    SDL_SetRenderDrawColor(renderer, 0x0, 0x0, 0x0, 0x7f);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_RenderFillRect(renderer, &or);

    /* render battery status */
    int life;
    tmp.x = tmp.y = tmp.w = tmp.h = 0;
    tmp.x = w;
    SDL_PowerState state = SDL_GetPowerInfo(NULL, &life);
    if (life > 0 && (state == SDL_POWERSTATE_ON_BATTERY
                    || state == SDL_POWERSTATE_CHARGING
                    || state == SDL_POWERSTATE_CHARGED))
    {
        icon = ICON_BATTERY_0 + (4 * (life / 100.0));
        d = or;
        SDL_QueryTexture(overlay->icons[icon], NULL, NULL, &d.w, &d.h);
        d.x = or.w - 5 - d.w;
        d.y = or.y + ((or.h / 2) - (d.h / 2));
        SDL_RenderCopy(renderer, overlay->icons[icon], NULL, &d);
        tmp = d;
    }

    /* render volume bar */
    icon = ICON_SPEAKER_LOW;
    d = or;
    if (overlay->volume <= 0)
        icon = ICON_SPEAKER_MUTE;
    else if (overlay->volume > (MAX_CTRL_VALUE/2))
        icon = ICON_SPEAKER_HIGH;

    SDL_QueryTexture(overlay->icons[icon], NULL, NULL, &d.w, &d.h);
    d.x += 5;
    d.y = or.y + ((or.h / 2) - (d.h / 2));
    SDL_RenderCopy(renderer, overlay->icons[icon], NULL, &d);

    d.x += d.w + 5;
    d.w = tmp.x - d.x - 10;
    d.h = or.h / 3;
    d.y = or.y + ((or.h / 2) - (d.h / 2));
    _overlay_render_bar(renderer, &d, overlay->volume / (double)MAX_CTRL_VALUE);

}

int
overlay_handle_event(overlay_t *overlay, SDL_Event *event)
{
    if (event->type == SDL_CONTROLLERBUTTONDOWN)
    {
        switch(event->cbutton.button)
        {
            case SDL_CONTROLLER_BUTTON_START:
                overlay->button_press_tick = SDL_GetTicks();
                break;

            case SDL_CONTROLLER_BUTTON_B:
                if (overlay->show)
                {
                    overlay->volume++;
                    if (overlay->volume > MAX_CTRL_VALUE - 1)
                        overlay->volume = MAX_CTRL_VALUE;
                    _overlay_alsa_master_volume(overlay);
                }
                break;

            case SDL_CONTROLLER_BUTTON_A:
                if (overlay->show)
                {
                    overlay->volume--;
                    if (overlay->volume < 0)
                        overlay->volume = 0;
                    _overlay_alsa_master_volume(overlay);
                }
                break;

            case SDL_CONTROLLER_BUTTON_X:
                if (overlay->show)
                {
                    overlay->brightness++;
                    if (overlay->brightness > MAX_CTRL_VALUE - 1)
                        overlay->brightness = MAX_CTRL_VALUE;
                    SDL_SetWindowBrightness(overlay->engine->window,
                                            overlay->brightness / (double)MAX_CTRL_VALUE);
                }
                break;

            case SDL_CONTROLLER_BUTTON_Y:
                if (overlay->show)
                {
                    overlay->brightness--;
                    if (overlay->brightness < 0)
                        overlay->brightness = 0;
                    SDL_SetWindowBrightness(overlay->engine->window,
                                            overlay->brightness / (double)MAX_CTRL_VALUE);
                }

        }
    }
    else if (event->type == SDL_CONTROLLERBUTTONUP)
    {
        switch(event->cbutton.button)
        {
            case SDL_CONTROLLER_BUTTON_START:
                overlay->button_press_tick = 0;
                overlay->button_release_tick = SDL_GetTicks();
                break;
        }
    }

    overlay->dirty = true;

    /* if overlay is show, consume controller events */
    return overlay->show ? 1 : 0;
}

int overlay_tick(overlay_t *overlay)
{
    uint32_t current_tick;
    current_tick = SDL_GetTicks();
    if (overlay->button_press_tick > 0
        && overlay->show == false
        && current_tick > (overlay->button_press_tick + 250))
    {
        overlay->button_press_tick = 0;
        overlay->show = true;
        return 1;
    }

    if (overlay->button_release_tick > 0)
    {
        overlay->button_press_tick = 0;
        overlay->button_release_tick = 0;
        overlay->show = false;
        return 1;
    }

    if (overlay->dirty)
    {
        overlay->dirty = false;
        return 1;
    }

    return 0;
}
