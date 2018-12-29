#ifndef _scene_h
#define _scene_h

#include <SDL_events.h>

typedef struct scene_t {
    int (*mount)(struct scene_t *scene, void *opaque);
    void (*unmount)(struct scene_t *scene);
    void (*render_back)(struct scene_t *scene);
    void (*render_front)(struct scene_t *scene);
    void (*render_overlay)(struct scene_t *scene);
    int (*tick)(struct scene_t *scene);
    void (*handle_event)(struct scene_t *scene, SDL_Event *event);
    struct engine_t *engine;
    void *opaque;
} scene_t;

void scene_render(scene_t *scene);
void scene_handle_event(scene_t *scene, SDL_Event *event);
int scene_tick(scene_t *scene);

#endif /* _scene_h */