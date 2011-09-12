/* sprite.c
 *
 * Sprite functions that can be useful.
 *
 * Copyright 1997, by Scott Deming.
 * All Rights Reserved!
 *
 * You can use this code any way you wish in any form you wish.  I provide
 * this as a tutorial on the technique and nothing more.
 */

#include <alex.h>

SPRITE *create_sprite(BITMAP *bmp, int x, int y)
{
	SPRITE *spr;

	spr = malloc(sizeof(SPRITE));
	if (spr == NULL)
		return NULL;

	spr->bmp = bmp;
	spr->x = x;
	spr->y = y;

	return spr;
}

void destroy_sprite(SPRITE *spr)
{
	if (spr == NULL)
		return;

	if (spr->bmp != NULL)
		destroy_bitmap(spr->bmp);

	free(spr);
}

SPRITE *outline_sprite(SPRITE *spr)
{
	BITMAP *nbmp;
	SPRITE *nspr;
	int x, x1, x2, y, y1, y2;
	int col = makecol(255, 255, 255);

	// create new sprite
	nbmp = create_bitmap(spr->bmp->w, spr->bmp->h);
	if (nbmp == NULL)
		return NULL;

	nspr = create_sprite(nbmp, 0, 0);
	if (nspr == NULL)
		return NULL;

	// clear it out
	clear(nspr->bmp);

	// do rows
	for (y=0; y<spr->bmp->h-1; y++)
	{
		// get left side point
		for (x1=0; (x1<spr->bmp->w-1)&&(spr->bmp->line[y][x1]==0); x1++) ;

		// get right side point
		for (x2=spr->bmp->w-1; (x2>0)&&(spr->bmp->line[y][x2]==0); x2--) ;

		// place the point
		if (x2 > x1)
		{
			nspr->bmp->line[y][x1] = col;
			nspr->bmp->line[y][x2] = col;
		}
	}

	// do columns
	for (x=0; x<spr->bmp->w-1; x++)
	{
		// get top point
		for (y1=0; (y1<spr->bmp->h-1)&&(spr->bmp->line[y1][x]==0); y1++) ;

		// get bottom point
		for (y2=spr->bmp->h-1; (y2>0)&&(spr->bmp->line[y2][x]==0); y2--) ;

		// place the point
		if (y2 > y1)
		{
			nspr->bmp->line[y1][x] = col;
			nspr->bmp->line[y2][x] = col;
		}
	}

	// return it
	return nspr;
}

void move_sprite(SPRITE *spr, int x, int y)
{
	spr->x = x;
	spr->y = y;
}

/*
// returns 1 if point is somewhere inside of spr.
int hittest_point(BITMAP *spr, int x, int y)
{
	if ((x >= sx) && (x <= sx+spr->w-1) &&
		(y >= sy) && (y <= sy+spr->h-1))
	{
		return 1;
	}

	return 0;
}

// returns 1 if the rectangle containing spr1 touches the rectangle containing spr2
// returns 2 if spr1 has any non-zero pixels touching a spr2 non-zero pixel
int hittest_sprite(BITMAP *spr1, BITMAP *spr2, int mode)
{
	int x, y;
	int x1, y1, x2, y2;

	// check for a touching point
	if (hittest_point(spr1, spr2->x, spr2->y) ||
		hittest_point(spr1, spr2->x, spr2->y+spr2->h-1) ||
		hittest_point(spr1, spr2->x+spr2->w-1, spr2->y) ||
		hittest_point(spr1, spr2->x+spr2->w-1, spr2->y+spr2->h-1))
	{
		// define overlapping rectangle
		x1 = MAX(spr1->x, spr2->x);
		y1 = MAX(spr1->y, spr2->y);
		x2 = MIN(spr1->x+w-1, spr2->x+w-1);
		y2 = MIN(spr1->y+h-1, spr2->y+h-1);
		rect(screen, x1, y1, x2, y2, makecol(255, 255, 255));

		// hittest pixels
		for (y=y1; y<y2; y++)
		{
			for (x=x1; x<x2; y++)
			{
				if (spr1->line[y][x] != 0 && spr2->line[y][x] != 0)
				{
					return 2;
				}
			}
		}

		return 1;
	}

	return 0;
}
*/
