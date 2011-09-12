/* ex4.c - Tile map example.
 *
 * Demonstrates using tiled maps for scrolling playfields.
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

volatile int time_to_blit = 0;
volatile int missed_frames = 0;
int idle_time = 0;

int screen_width, screen_height;

void idle_proc()
{
	// we're idle, do nothing for now.
	idle_time++;
}

void time_to_blit_timer()
{
	if (time_to_blit != 0)
		missed_frames++;

	time_to_blit = 1;
}
END_OF_FUNCTION(time_to_blit_timer);

void main(int argc, char *argv[])
{
	BITMAP *scrFlip;
	PALETTE pal;
	long frames = 0L;
	float start_time, end_time;
	int click = 0;
	int i;

	// Default screen resolution to 320x200.
	screen_width = 320;
	screen_height = 200;

	// Read command line args.	Set screen size accordingly.
	if (argc > 2)
	{
		screen_width = atoi(argv[1]);
		screen_height = atoi(argv[2]);
	}

	printf("screen: %dX%d\n", screen_width, screen_height);

	// Initialize Allegro.	Great library!
	allegro_init();
	install_timer();
	install_mouse();
	set_mouse_speed(0, 0);

	// Set graphics mode.
	if (set_gfx_mode(GFX_AUTODETECT, screen_width, screen_height, 0, 0) == -1)
	{
		printf("Cannot set video mode to %d, %d.\n", screen_width, screen_height);
		exit(1);
	}
	text_mode(-1);

	// Allocate buffer for our of screen rendering.
	scrFlip = create_bitmap(SCREEN_W, SCREEN_H);

	// setup time_to_blit interrupt.
	LOCK_VARIABLE(time_to_blit);
	LOCK_FUNCTION(time_to_blit_timer);
	install_int_ex(time_to_blit_timer, BPS_TO_TIMER(30));

	// main loop.
	start_time = clock();
	while (!kbhit())
	{
		idle_time = 0;
		while (!time_to_blit) idle_proc(); // lock it in at around 30 fps.
		time_to_blit = 0;

		// Increment frame counter.
		frames++;
	}
	end_time = clock();

	// Grab the key that was pressed.
	getch();

	// Clean up.
	destroy_bitmap(scrFlip);

	// Close down allegro.
	allegro_exit();

	// Report.
	printf("Tile Map, Copyright 1997 by Scott Deming.\n");
	printf("\nHow'd we do?\n");
	printf("===============================================================================\n");
	printf("  Time:  %3.2f\n", (float) ((end_time-start_time) / 100));
	printf("Frames:  %lu\n", frames);
	printf("   FPS:  %3.2f\n", (float) (float) frames / (float) ((end_time-start_time) / 100));
	printf("Missed:  %d\n", missed_frames);
	printf("  Idle:  %d (called idle_proc() that many times per frame!)\n", idle_time);
	printf("Screen:  %dX%d\n", screen_width, screen_height);
	printf("-------------------------------------------------------------------------------\n");
	printf("Note:  If \"Idle\" starts to get close to zero then things should be done to\n");
	printf("       speed things up.  When idle proc hits zero, we'll start missing frames\n");
	printf("       (indicated by \"Missed\") and things can start to get a bit jerky.\n");
	printf("       If \"Missed\" is greater than 0, start optimizing.\n");
}

