#ifndef _config_h
#define _config_h

#include <jansson.h>

#define HJORTRON_CONFIG_FILE "hjortron.conf"

typedef struct config_t {
    json_t *root;
} config_t;

int config_init(config_t *config);
void config_deinit(config_t *config);
int config_set(config_t *config, const char *key, const char *value);
const char *config_get(config_t *config, const char *key, const char *default_value);
int config_save(config_t *config);

#endif /* _config_h */
