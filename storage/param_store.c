#include "storage/param_store.h"

static uint32_t fnv1a32(const uint8_t *data, uint16_t length)
{
    uint16_t i;
    uint32_t hash = 2166136261u;
    for (i = 0u; i < length; i++) {
        hash ^= data[i];
        hash *= 16777619u;
    }
    return hash;
}

static void make_image(sscb_param_image_t *image, const sscb_params_t *params, uint32_t sequence)
{
    image->magic = SSCB_PARAM_STORE_MAGIC;
    image->version = params->parameter_version;
    image->length = (uint16_t)sizeof(sscb_params_t);
    image->sequence = sequence;
    image->params = *params;
    image->crc32 = fnv1a32((const uint8_t *)&image->params, image->length);
}

static int valid_image(const sscb_param_image_t *image)
{
    if (image->magic != SSCB_PARAM_STORE_MAGIC || image->length != sizeof(sscb_params_t)) {
        return 0;
    }
    return image->crc32 == fnv1a32((const uint8_t *)&image->params, image->length);
}

void sscb_param_store_init(sscb_param_store_t *store)
{
    if (store == 0) {
        return;
    }
    store->slot_a.magic = 0u;
    store->slot_b.magic = 0u;
}

sscb_status_t sscb_param_store_save(sscb_param_store_t *store, const sscb_params_t *params)
{
    uint32_t seq_a;
    uint32_t seq_b;
    if (store == 0 || params == 0) {
        return SSCB_ERR_ARG;
    }

    seq_a = valid_image(&store->slot_a) ? store->slot_a.sequence : 0u;
    seq_b = valid_image(&store->slot_b) ? store->slot_b.sequence : 0u;
    if (seq_a <= seq_b) {
        make_image(&store->slot_a, params, seq_b + 1u);
    } else {
        make_image(&store->slot_b, params, seq_a + 1u);
    }
    return SSCB_OK;
}

sscb_status_t sscb_param_store_load(sscb_param_store_t *store, sscb_params_t *params)
{
    int a_valid;
    int b_valid;
    if (store == 0 || params == 0) {
        return SSCB_ERR_ARG;
    }
    a_valid = valid_image(&store->slot_a);
    b_valid = valid_image(&store->slot_b);

    if (a_valid && (!b_valid || store->slot_a.sequence >= store->slot_b.sequence)) {
        *params = store->slot_a.params;
        return SSCB_OK;
    }
    if (b_valid) {
        *params = store->slot_b.params;
        return SSCB_OK;
    }

    sscb_params_load_defaults(params);
    (void)sscb_param_store_save(store, params);
    return SSCB_ERR_NOT_FOUND;
}
