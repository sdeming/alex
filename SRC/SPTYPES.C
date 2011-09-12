/* sptypes.c
 *
 * Some "spiffy" types and helper functions.
 *
 * Copyright 1997, by Scott Deming.
 * All Rights Reserved!
 *
 * You can use this code any way you wish in any form you wish.  I provide
 * this as a tutorial on the technique and nothing more.
 */

#include <allegro.h>

#include <alex.h>

int minmax(int val, int min, int max)
{
	if (val > max)
		val = max;
	else
	if (val < min)
		val = min;

	return val;
}

int getbitcount(int num)
{
	int bits = 0;

	while (num > 0)
	{
		num>>=1;
		bits++;
	}

	return bits;
}

void rect_combine(RECT *r, RECT *left, RECT *right)
{
	int x2, y2;

	r->x = MIN(left->x, right->x);
	r->y = MIN(left->y, right->y);

	x2 = MAX((left->x+left->w), (right->x+right->w));
	y2 = MAX((left->y+left->h), (right->y+right->h));

	r->w = x2 - r->x;
	r->h = y2 - r->y;
}

void rect_copy(RECT *s, RECT *d)
{
	s->x = d->x;
	s->y = d->y;
	s->w = d->w;
	s->h = d->h;
}

void rect_expand(RECT *r, int size)
{
	r->x-=size;
	r->y-=size;
	r->w+=size<<1;
	r->h+=size<<1;
}

void rect_dirty_max(RECT *r)
{
	r->x = -1;
	r->y = -1;
	r->w = -1;
	r->h = -1;
}
