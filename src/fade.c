/* fade.c
 *
 * Fade between two images.
 *
 * Copyright 1997, by Scott Deming.
 * All Rights Reserved!
 *
 * You can use this code any way you wish in any form you wish.  I provide
 * this as a tutorial on the technique and nothing more.
 */

#include <alex.h>

// Fades a single frame.  Based on 'num_frames' and 'frame', we'll find out the
// value of each pixel to copy into 'imgBuffer' by interpolating the color values
// from 'imgFrom' and 'imgTo'.
// Returns 0 for success, or -1 for some kind of failure, ie image sizes don't match.
int fade_bitmap(BITMAP *imgFrom, BITMAP *imgTo, BITMAP *imgBuffer, int x_start, int y_start, int frame, int num_frames)
{
	fixed perc_to = fdiv(itofix(frame), itofix(num_frames));
	fixed perc_from = itofix(1) - perc_to;
	PALETTE pal;
	RGB rgb;
	int y, x;

	// Get the current palette.
	get_palette(pal);

	// Make sure images are the same size.
	if (imgFrom->w != imgTo->w || imgFrom->h != imgTo->h)
	{
		return -1;
	}

	// Interpolate the image.
	for (y=0; y<imgFrom->h; y++)					// Run through the rows.
	{
		for (x=0; x<imgFrom->w; x++)				// Run through the columns.
		{
			rgb.r = (fixtoi(fmul(itofix(pal[imgFrom->line[y][x]].r), perc_from)) + fixtoi(fmul(itofix(pal[imgTo->line[y][x]].r), perc_to))) << 2;
			rgb.g = (fixtoi(fmul(itofix(pal[imgFrom->line[y][x]].g), perc_from)) + fixtoi(fmul(itofix(pal[imgTo->line[y][x]].g), perc_to))) << 2;
			rgb.b = (fixtoi(fmul(itofix(pal[imgFrom->line[y][x]].b), perc_from)) + fixtoi(fmul(itofix(pal[imgTo->line[y][x]].b), perc_to))) << 2;

			// Set the pixel to the closest matching color in the palette.
			imgBuffer->line[y+y_start][x+x_start] = makecol8(rgb.r, rgb.g, rgb.b);
		}
	}

	return 0;
}
