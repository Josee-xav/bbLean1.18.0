#pragma once
#include "bblib.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ------------------------------------------------------------------------- */
/* parse a StyleItem */

struct StyleItem;

BBLIB_EXPORT void parse_item (LPCSTR szItem, struct StyleItem *item);
BBLIB_EXPORT int findtex (const char *p, int prop);
struct styleprop
{
    const char *key;
    int val;
};
BBLIB_EXPORT const struct styleprop *get_styleprop(int prop);
BBLIB_EXPORT int find_in_propitem (struct styleprop const * props, int value);

extern const struct styleprop styleprop_1[];
extern const struct styleprop styleprop_2[];
extern const struct styleprop styleprop_3[];

#ifdef __cplusplus
}
#endif


