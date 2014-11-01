#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "fswebcam.h"

#ifndef INC_MOTION_H
#define INC_MOTION_H

typedef avgbmp_t rgbbmp_t;
typedef unsigned char greybmp_t;

typedef struct
{
   uint16_t x;
   uint16_t y;
   uint16_t width;
   uint16_t height;

   uint16_t threshold;

   greybmp_t *lastbmp;
} motion_zone_t;

typedef struct
{
   uint16_t width;
   uint16_t height;

   unsigned int framecnt;

   motion_zone_t *zone;
   unsigned int zones;
} motion_t;

void motion_init(motion_t *motion);

int motion_addzone(motion_t *motion, uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t threshold);

int motion_detect(motion_t *motion, rgbbmp_t *bitmap);

void motion_free(motion_t *motion);

#endif


