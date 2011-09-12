/* tile.c
 *
 * Bitmap tiling routine.
 *
 * Copyright 1997, by Scott Deming.
 * All Rights Reserved!
 *
 * You can use this code any way you wish in any form you wish.  I provide
 * this as a tutorial on the technique and nothing more.
 */

#include <alex.h>

// tile a bitmap 'tile' to the bitmap 'scr'.
void tile_bitmap(BITMAP *tile, BITMAP *scr)
{
	int x, y, w, h;

	for (y=0; y<scr->h; y+=tile->h)
	{
		for (x=0; x<scr->w; x+=tile->w)
		{
			w = MIN(tile->w, scr->w-x);
			h = MIN(tile->h, scr->h-y);
			blit(tile, scr, 0, 0, x, y, w, h);
		}
	}
}
