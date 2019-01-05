#ifndef _transition_h
#define _transition_h

typedef enum transition_type_t {
    TRANSITION_ONLY_SRC,
    TRANSITION_XFADE,
} transition_type_t;

typedef struct transition_t{
    transition_type_t type;
    struct scene_t *source;
    void  *source_opaque;
    struct scene_t *dest;
    void *dest_opaque;
    uint32_t duration;
} transition_t;


#endif /* _transition_h */