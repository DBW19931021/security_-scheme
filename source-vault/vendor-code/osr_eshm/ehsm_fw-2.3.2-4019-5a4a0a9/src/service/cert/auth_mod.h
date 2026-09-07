#ifndef AUTH_MOD_H
#define AUTH_MOD_H

#include <stddef.h>
#include "auth_common.h"
#include "img_parser_mod.h"
#include "tbbr_img_def_exp.h"
#include "types.h"

#define MAX_NUMBER_IDS MAX_IMAGE_IDS
/*
 * Image flags
 */
#define IMG_FLAG_AUTHENTICATED (1 << 0)

#define COT_MAX_VERIFIED_PARAMS 4

/* Compute the number of elements in the given array */
#define ARRAY_SIZE(a)				\
	(sizeof(a) / sizeof((a)[0]))

/*
 * Authentication image descriptor
 */
typedef struct auth_img_desc_s {
    unsigned int img_id;
    img_type_t img_type;
    const struct auth_img_desc_s *parent;
    const auth_method_desc_t *const img_auth_methods;
    const auth_param_desc_t *const authenticated_data;
} auth_img_desc_t;

uint32_t auth_mod_get_parent_id(uint32_t img_id, uint32_t *parent_id);
uint32_t auth_mod_verify_img(uint32_t img_id, void *img_ptr, uint32_t img_len);
uint32_t auth_get_nv_ctr(void *cookie, uint32_t *nv_ctr);
uint32_t auth_set_nv_ctr(void *cookie, uint32_t nv_ctr);

/* Macro to register a CoT defined as an array of auth_img_desc_t pointers */
#define REGISTER_COT(_cot)                                     \
    const auth_img_desc_t *const *const cot_desc_ptr = (_cot); \
    const size_t cot_desc_size = ARRAY_SIZE(_cot);             \
    unsigned int auth_img_flags[MAX_NUMBER_IDS]

extern const auth_img_desc_t *const *const cot_desc_ptr;
extern const size_t cot_desc_size;
extern unsigned int auth_img_flags[];

/* TBBR related getter */
#define tbbr__cot_getter(id)          \
    __extension__({                   \
        assert((id) < cot_desc_size); \
        cot_desc_ptr[id];             \
    })

typedef struct auth_cert_info {
    unsigned int img_id;
    raddr_t addr;
    unsigned int size;
} auth_cert_info_st;

#endif /* AUTH_MOD_H */
