
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "motion.h"
#include "log.h"

void motion_init(motion_t *motion)
{
   memset(motion, 0, sizeof(motion_t));
}

int motion_addzone(motion_t *motion, uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t threshold)
{
   void *zone, *frame;

/*
   if ((x + width) > motion->width || (y + height) > motion->height)
   {
      WARN("Zone outside frame. Ignored...");
      return -1;
   }
*/
   zone = realloc(motion->zone, sizeof(motion_zone_t) * (motion->zones + 1));
   if (!zone)
   {
      ERROR("Out of memory.");
      return -2;
   }

   frame = malloc(width * height * sizeof(greybmp_t));
   if (!frame)
   {
      ERROR("Out of memory.");
      return -3;
   }

   motion->zone = zone;

   motion->zone[motion->zones].x = x;
   motion->zone[motion->zones].y = y;
   motion->zone[motion->zones].width = width;
   motion->zone[motion->zones].height = height;
   motion->zone[motion->zones].lastbmp = frame;
   motion->zone[motion->zones].threshold = threshold;

   ++motion->zones;

   return 0;
}

int motion_detect(motion_t *motion, rgbbmp_t *bitmap)
{
   uint16_t z, x, y, triggered;

   uint32_t pixels;

   uint16_t thresh_sq;

   int16_t diff;

   uint32_t diff_sq;

   uint32_t peak_diff_sq;

   uint32_t sum_diff_sq;

   uint32_t count;

   uint32_t grey;

   motion_zone_t *zone;

   rgbbmp_t *avgbmp;

   greybmp_t *lastbmp;

   if (motion->framecnt == 0)
   {
      for (z = 0; z < motion->zones; ++z)
      {
         zone = &motion->zone[z];

         avgbmp = &bitmap[3 * (zone->y * zone->width + zone->x)];

         lastbmp = zone->lastbmp;

         for (y = 0; y < zone->height; ++y)
         {
            for (x = 0; x < zone->width; ++x)
            {
               grey  = 30 * (*avgbmp++);
               grey += 59 * (*avgbmp++);
               grey += 11 * (*avgbmp++);
               grey /= 100;

               *lastbmp++ = grey;
            }

            avgbmp += 3 * (motion->width - zone->width);
         }
      }

      ++motion->framecnt;

      return 0;
   }

   triggered = 0;

   peak_diff_sq = 0;

   sum_diff_sq = 0;

   for (z = 0; z < motion->zones; ++z)
   {
      count = 0;

      zone = &motion->zone[z];

      pixels = zone->height * zone->width;

      thresh_sq = zone->threshold * zone->threshold;

      avgbmp = &bitmap[3 * (zone->y * zone->width + zone->x)];

      lastbmp = zone->lastbmp;

      for (y = 0; y < zone->height; ++y)
      {
         for (x = 0; x < zone->width; ++x)
         {
            grey  = 30 * (*avgbmp++);
            grey += 59 * (*avgbmp++);
            grey += 11 * (*avgbmp++);
            grey /= 100;

            diff = grey - (*lastbmp);

            diff_sq = diff * diff;
            
            if (diff_sq > peak_diff_sq)
            {
               peak_diff_sq = diff_sq;
            }

            sum_diff_sq += diff_sq;

            count += (diff_sq > thresh_sq);

            *lastbmp++ = grey;
         }

         avgbmp += 3 * (motion->width - zone->width);
      }

      MSG("Zone (%u) count=%u peak_diff_sq=%u avg_diff_sq=%u", z, count, peak_diff_sq, sum_diff_sq / pixels);

      triggered += (count != 0);
   }

   ++motion->framecnt;

   return triggered;
}

void motion_free(motion_t *motion)
{
   int i;
   motion_zone_t *z;

   if (motion->zone)
   {
      for (i = 0; i < motion->zones; ++i)
      {
         z = &motion->zone[i];

         if (z->lastbmp)
         {
            free(z->lastbmp);
            z->lastbmp= NULL;
         }
      }

      free(motion->zone);
      motion->zone = NULL;

      memset(motion, 0, sizeof(motion_t));
   }
}

