#ifndef IMG_PARSER_MOD_H
#define IMG_PARSER_MOD_H

#include "types.h"
#include "auth_common.h"

/*
 * Image types. A parser should be instantiated and registered for each type
 */
typedef enum img_type_enum {
    IMG_RAW,  /* Binary image */
    IMG_PLAT, /* Platform specific format */
    IMG_CERT, /* X509v3 certificate */
    IMG_MAX_TYPES,
} img_type_t;

/* Image parser library structure */
typedef struct img_parser_lib_desc_s {
    img_type_t img_type;
    const char *name;

    void (*init)(void);
    uint32_t (*check_integrity)(void *img, unsigned int img_len);
    uint32_t (*get_auth_param)(const auth_param_type_desc_t *type_desc, void *img, unsigned int img_len, void **param,
        unsigned int *param_len);
} img_parser_lib_desc_t;

/* Exported functions */
void img_parser_init(void);
uint32_t img_parser_check_integrity(img_type_t img_type, void *img_ptr, uint32_t img_len);
uint32_t img_parser_get_auth_param(img_type_t img_type, const auth_param_type_desc_t *type_desc, void *img_ptr,
    uint32_t img_len, void **param_ptr, uint32_t *param_len);

/* Macro to register an image parser library */
#define REGISTER_IMG_PARSER_LIB(_type, _name, _init, _check_int, _get_param)                                         \
    static const img_parser_lib_desc_t __img_parser_lib_desc_##_type __section(".img_parser_lib_descs") __used = {   \
        .img_type = _type, .name = _name, .init = _init, .check_integrity = _check_int, .get_auth_param = _get_param \
    }

#endif /* IMG_PARSER_MOD_H */
