#pragma once
#include "BBApi.h"
#include "styleprops.h"

#ifdef __cplusplus
extern "C" {
#endif

const struct styleprop styleprop_1[] = {
 {"solid"           ,B_SOLID           },
 {"horizontal"      ,B_HORIZONTAL      },
 {"splithorizontal" ,B_SPLITHORIZONTAL }, // "horizontal" is match .*horizontal
 {"blockhorizontal" ,B_BLOCKHORIZONTAL },
 {"mirrorhorizontal",B_MIRRORHORIZONTAL},
 {"wavehorizontal"  ,B_WAVEHORIZONTAL  },
 {"vertical"        ,B_VERTICAL        },
 {"splitvertical"   ,B_SPLITVERTICAL   }, // "vertical" is match .*vertical
 {"blockvertical"   ,B_BLOCKVERTICAL   },
 {"mirrorvertical"  ,B_MIRRORVERTICAL  },
 {"wavevertical"    ,B_WAVEVERTICAL    },
 {"diagonal"        ,B_DIAGONAL        },
 {"crossdiagonal"   ,B_CROSSDIAGONAL   },
 {"pipecross"       ,B_PIPECROSS       },
 {"elliptic"        ,B_ELLIPTIC        },
 {"rectangle"       ,B_RECTANGLE       },
 {"pyramid"         ,B_PYRAMID         },
 {NULL              ,-1                }
 };

const struct styleprop styleprop_2[] = {
 {"flat"        ,BEVEL_FLAT     },
 {"raised"      ,BEVEL_RAISED   },
 {"sunken"      ,BEVEL_SUNKEN   },
 {NULL          ,-1             }
 };

const struct styleprop styleprop_3[] = {
 {"bevel1"      ,BEVEL1 },
 {"bevel2"      ,BEVEL2 },
 {"bevel3"      ,BEVEL2+1 },
 {NULL          ,-1     }
 };

/* ------------------------------------------------------------------------- */
// parse a given string and assigns settings to a StyleItem class

const struct styleprop *get_styleprop(int n)
{
    switch (n) {
        case 1: return styleprop_1;
        case 2: return styleprop_2;
        case 3: return styleprop_3;
        default : return NULL;
    }
}

int findtex(const char *p, int prop)
{
    const struct styleprop *s = get_styleprop(prop);
    do
        if (strstr(p, s->key))
            break;
    while ((++s)->key);
    return s->val;
}
int find_exact (const char * p, int prop)
{
    const struct styleprop * s = get_styleprop(prop);
    do
        if (strcmp(p, s->key) == 0)
            break;
    while ((++s)->key);
    return s->val;
}

void parse_item(LPCSTR szItem, StyleItem *item)
{
    char buf[256];
    char * ptr = &buf[0];
    int t = -1;
    char option[256];
    bool found = false;
    _strlwr(strcpy(buf, szItem));
    t = item->parentRelative = NULL != strstr(buf, "parentrelative");
    if (t) {
        item->type = item->bevelstyle = item->bevelposition = item->interlaced = 0;
        return;
    }

    while (NextToken(option, &ptr, NULL))
    {
        int gt = -1;
        if (strlen(option) == 0)
            break;
        gt = find_exact(option, 1);
        if (gt != -1)
        {
          found = true;
          item->type = gt; 
          break;
        }
    }
    if (!found)
    {
      item->type = strstr(buf, "gradient") ? B_DIAGONAL : B_SOLID;
    }

    t = findtex(buf, 2);
    item->bevelstyle = (-1 != t) ? t : BEVEL_RAISED;

    t = BEVEL_FLAT == item->bevelstyle ? 0 : findtex(buf, 3);
    item->bevelposition = (-1 != t) ? t : BEVEL1;

    item->interlaced = NULL!=strstr(buf, "interlaced");
}   

int find_in_propitem (struct styleprop const * props, int value)
{
    int i = 0;
    while (props[i].key != NULL)
    {
        if (props[i].val == value)
            return i;
        ++i;
    }
    return -1;
}



#ifdef __cplusplus
}
#endif


