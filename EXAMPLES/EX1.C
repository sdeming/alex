/* ex1.c - Crossfade example.
 *
 * Demonstrates the fade_bitmap and the match_bitmap routines to perform a
 * crossfade.
 *
 * Copyright 1997, by Scott Deming.
 * All Rights Reserved!
 *
 * You can use this code any way you wish in any form you wish.  I provide
 * this as a tutorial on the technique and nothing more.
 */

#include <conio.h>
#include <mem.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

#include <sys/types.h>

#include <alex.h>

int screen_width, screen_height;

int image_width, image_height;

RGB_MAP rgb_table;

void rgb_table_callback(int pos)
{
	int perc = (pos*100/255);

	gui_status_bar(screen, 20, 25, SCREEN_W-40, 0, "%", perc);
}

void main(int argc, char *argv[])
{
	BITMAP *imgFrom, *imgFromSmall, *imgTo, *imgToSmall, *imgFade;
	WINDOW *winFade;
	PALETTE palFrom, palTo;
	long frames = 0L;
	double start_time, end_time;
	int fade_frame = 0;
	int num_fade_frames = 16;
	int fade_dir = 1;
	int x;

	// Default screen resolution of 320x200.
	screen_width = 320;
	screen_height = 200;

	// Read command line args.	1 arg means to use n frames, 3 means to use n
	// frames and set the screen to specified width/height.
	if (argc > 1)
	{
		num_fade_frames = atoi(argv[1]);
		if (argc > 3)
		{
			screen_width = atoi(argv[2]);
			screen_height = atoi(argv[3]);
		}
	}

	// Default image size.
	image_width = screen_width-64;
	image_height = screen_height-64;

	printf("frames: %d\nscreen: %dX%d\n", num_fade_frames, screen_width, screen_height);

	// Initialize Allegro.	Great library!
	allegro_init();

	// Set graphics mode.
	if (set_gfx_mode(GFX_AUTODETECT, screen_width, screen_height, 0, 0) == -1)
	{
		printf("Cannot set video mode to %d, %d.\n", screen_width, screen_height);
		exit(1);
	}

	// Load "from" image, and scale it down.
	imgFrom = load_bitmap("from.pcx", palFrom);
	imgFromSmall = create_bitmap(image_width, image_height);
	stretch_blit(imgFrom, imgFromSmall, 0, 0, imgFrom->w, imgFrom->h, 0, 0, imgFromSmall->w, imgFromSmall->h);

	// Set gui, no translucency or lighting tables.
	gui_initialize(NULL, NULL);
	gui_setpalette(palFrom);

	// Build rgb_table.
	text_mode(1);
	textout_centre(screen, font, "Building RGB table.", SCREEN_W>>1, 10, 100);
	create_rgb_table(&rgb_table, palFrom, rgb_table_callback);
	rgb_map = &rgb_table;

	// Load "to" image, and scale it down.
	imgTo = load_bitmap("to.pcx", palTo);
	match_bitmap(imgTo, palTo);
	imgToSmall = create_bitmap(image_width, image_height);
	stretch_blit(imgTo, imgToSmall, 0, 0, imgTo->w, imgTo->h, 0, 0, imgToSmall->w, imgToSmall->h);

	// Allocate buffer for our off screen fade rendering and clear it.
	imgFade = create_bitmap(imgToSmall->w, imgToSmall->h);
	clear(imgFade);

	// Create fade window.
	winFade = gui_create_window(imgFade->w+2, imgFade->h+2, WB_THIN, WD_BLIT);
	gui_clear_window(winFade, 0);
	gui_move_window(winFade, (SCREEN_W>>1)-(winFade->window.w>>1), (SCREEN_H>>1)-(winFade->window.h>>1));

	// Clear screen
	clear(screen);

	// Header
	x = SCREEN_W>>1;
	textout_centre(screen, font, "Crossfade Example (ex1.c)", x, 3, makecol8(255, 255, 255));

	// main loop.
	start_time = clock();
	while (!kbhit())
	{
		// Fade the image to the current frame.
		fade_bitmap(imgFromSmall, imgToSmall, imgFade, 0, 0, fade_frame, num_fade_frames);
		gui_blit_window(winFade, imgFade, 0, 0, imgFade->w, imgFade->h);
		gui_draw_window(winFade, screen);

		// Set the fade_frame.
		fade_frame += fade_dir;
		if (fade_frame >= num_fade_frames)
		{
			fade_frame = num_fade_frames;
			fade_dir = -1;
		} else
		if (fade_frame < 1)
		{
			fade_frame = 0;
			fade_dir = 1;
		}

		// Increment frame counter.
		frames++;
	}
	end_time = clock();

	// Grab the key that was pressed.
	getch();

	// Clean up.
	gui_destroy_window(winFade);
	destroy_bitmap(imgFrom);
	destroy_bitmap(imgFromSmall);
	destroy_bitmap(imgTo);
	destroy_bitmap(imgToSmall);
	destroy_bitmap(imgFade);

	// Close down allegro.
	allegro_exit();

	// Report.
	printf("Crossfade, Copyright 1997 by Scott Deming.\n");
	printf("\nHow'd we do?\n");
	printf("===============================================================================\n");
	printf("  Time:  %3.2f\n", (double) ((end_time-start_time) / 100));
	printf("Frames:  %lu\n", frames);
	printf("   FPS:  %3.2f\n", (double) (double) frames / (double) ((end_time-start_time) / 100));
	printf("Screen:  %dX%d\n", screen_width, screen_height);
	printf("-------------------------------------------------------------------------------\n");
}

