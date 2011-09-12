/* match.c
 *
 * Match a bitmap to use the current screen palette.
 *
 * Copyright 1997, by Scott Deming.
 * All Rights Reserved!
 *
 * You can use this code any way you wish in any form you wish.  I provide
 * this as a tutorial on the technique and nothing more.
 */

#include <alex.h>

void match_bitmap(BITMAP *img, PALETTE palFrom)
{
	int y, x;

	for (y=0; y<img->h; y++)
	{
		for (x=0; x<img->w; x++)
		{
			img->line[y][x] = makecol8(palFrom[img->line[y][x]].r<<2, palFrom[img->line[y][x]].g<<2, palFrom[img->line[y][x]].b<<2);
		}
	}
}
