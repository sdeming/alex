/* ex2.c - Display list example.
 *
 * Demonstrates using a display list.
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

int image_width, image_height;

RGB_MAP rgb_table;
COLOR_MAP light_table;
COLOR_MAP trans_table;

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

void rgb_table_callback(int pos)
{
	int perc = (pos*100/255);

	gui_status_bar(screen, 20, 25, SCREEN_W-40, 0, "%", perc);
}

void main(int argc, char *argv[])
{
	BITMAP *imgTile, *imgFrom, *imgFromSmall, *imgTo, *imgToSmall, *imgLight;
	BITMAP *scrFlip;
	WINDOW *winFrom, *winTo;
	PALETTE palTile, palFrom, palTo;
	RECT undo, dirty, render;
	long frames = 0L;
	float start_time, end_time;
	display_list *dl;
	display_item *diWinFrom, *diWinTo, *diImgLight, *diTop;
	int f_dx = 1, f_dy = 1, f_x, f_y;
	int t_dx = -1, t_dy = -1, t_x, t_y;
	int click = 0;
	int i;

	// Default screen resolution to 320x200.
	screen_width = 320;
	screen_height = 200;

	// Default image size.
	image_width = 128;
	image_height = 128;

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

	// Load "from" image, and scale it down.
	imgFrom = load_bitmap("from.pcx", palFrom);
	imgFromSmall = create_bitmap(image_width, image_height);
	stretch_blit(imgFrom, imgFromSmall, 0, 0, imgFrom->w, imgFrom->h, 0, 0, imgFromSmall->w, imgFromSmall->h);

	// Set palette.
	gui_setpalette(palFrom);

	// Build rgb_table.
	text_mode(1);
	textout_centre(screen, font, "Building RGB table.", SCREEN_W>>1, 10, 100);
	create_rgb_table(&rgb_table, palFrom, rgb_table_callback);
	rgb_map = &rgb_table;

	// Build lighting table.
	text_mode(1);
	textout_centre(screen, font, "Building lighting table.", SCREEN_W>>1, 10, 100);
	create_light_table(&light_table, palFrom, 64, 64, 64, rgb_table_callback);

	// Build translucency table.
	text_mode(1);
	textout_centre(screen, font, "Building translucency table.", SCREEN_W>>1, 10, 100);
	create_trans_table(&trans_table, palFrom, 128, 128, 128, rgb_table_callback);

	// Set up gui to use lighting and translucency tables.
	gui_initialize(&light_table, &trans_table);

	// Load "to" image, and scale it down.
	imgTo = load_bitmap("to.pcx", palTo);
	match_bitmap(imgTo, palTo);
	imgToSmall = create_bitmap(image_width, image_height);
	stretch_blit(imgTo, imgToSmall, 0, 0, imgTo->w, imgTo->h, 0, 0, imgToSmall->w, imgToSmall->h);

	// Allocate buffer for our of screen rendering.
	scrFlip = create_bitmap(SCREEN_W, SCREEN_H);

	// Load "tile" image, match it to current palette, and tile the main screen.
	imgTile = load_bitmap("tile.pcx", palTile);
	match_bitmap(imgTile, palTile);
	tile_bitmap(imgTile, scrFlip);

	// Create the spotlight mouse bitmap.
	imgLight = create_bitmap(64, 64);
	clear_to_color(imgLight, 255);
	for (i=0; i<255; i++)
		circlefill(imgLight, 32, 32, 32-i/8, 255-i);

	// Create from/to windows.
	winFrom = gui_create_window(imgFromSmall->w+2, imgFromSmall->h+2, WB_THIN, WD_TRANSLUCENT);
	gui_clear_window(winFrom, 0);
	gui_blit_window(winFrom, imgFromSmall, 0, 0, imgFromSmall->w, imgFromSmall->h);

	winTo = gui_create_window(imgToSmall->w+2, imgToSmall->w+2, WB_THIN, WD_TRANSLUCENT);
	gui_clear_window(winTo, 0);
	gui_blit_window(winTo, imgToSmall, 0, 0, imgToSmall->w, imgToSmall->h);

	// Blit initial screen.
	blit(scrFlip, screen, 0, 0, 0, 0, SCREEN_W, SCREEN_H);

	// initialize dirty rectangles.
	rect_dirty_max(&dirty);
	rect_dirty_max(&render);
	rect_dirty_max(&undo);

	text_mode(0);

	// initialize coordinates.
	f_x = 0; f_y = 0;
	t_x = SCREEN_W-imgToSmall->w; t_y = 0;

	// build initial display list.
	dl = create_display_list(&light_table, &trans_table);
	diWinFrom = add_display_item(dl, create_display_item(winFrom, DISPLAY_WINDOW, f_x, f_y, 1, 0, 1));
	diWinTo = add_display_item(dl, create_display_item(winTo, DISPLAY_WINDOW, t_x, t_y, 1, 0, 1));
	diImgLight = add_display_item(dl, create_display_item(imgLight, DISPLAY_LIGHT, mouse_x, mouse_y, 1, 0, 1));

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

		// Set window drawing styles.
		set_all_windows_styles(dl, -1, WD_TRANSLUCENT);

		if ((diTop = get_top_window(dl, mouse_x, mouse_y)) != NULL)
			gui_set_window_styles((WINDOW*) diTop->pContents, -1, WD_BLIT);

		// mouse click
		if (mouse_b & 1)
		{
			click = 1;
		} else {
			if (click)
			{
				bring_window_to_top(get_top_window(dl, mouse_x, mouse_y));
				click = 0;
			}
		}

		// Move display items.
		move_display_item(diWinFrom, f_x, f_y);
		move_display_item(diWinTo, t_x, t_y);
		move_display_item(diImgLight, mouse_x-(imgLight->w>>1), mouse_y-(imgLight->h>>1));

		// Render display lists.
		render_display_list(dl, scrFlip, &render);
		rect_combine(&dirty, &render, &undo);
		blit(scrFlip, screen, dirty.x, dirty.y, dirty.x, dirty.y, dirty.w, dirty.h);

		// Restore original background in off screen buffer.
		undo_render_display_list(dl, scrFlip, &undo);

		// Move bouncing windows.
		f_x += f_dx;
		f_y += f_dy;
		if (f_x >= SCREEN_W-imgFromSmall->w || f_x <= 0)
		{
			f_x = MIN(f_x, SCREEN_W-imgFromSmall->w);
			f_x = MAX(f_x, 0);
			f_dx = -f_dx;
		}
		if (f_y >= SCREEN_H-imgFromSmall->h || f_y <= 0)
		{
			f_y = MIN(f_y, SCREEN_H-imgFromSmall->h);
			f_y = MAX(f_y, 0);
			f_dy = -f_dy;
		}

		t_x += t_dx;
		t_y += t_dy;
		if (t_x >= SCREEN_W-imgFromSmall->w || t_x <= 0)
		{
			t_x = MIN(t_x, SCREEN_W-imgFromSmall->w);
			t_x = MAX(t_x, 0);
			t_dx = -t_dx;
		}
		if (t_y >= SCREEN_H-imgFromSmall->h || t_y <= 0)
		{
			t_y = MIN(t_y, SCREEN_H-imgFromSmall->h);
			t_y = MAX(t_y, 0);
			t_dy = -t_dy;
		}

		// Increment frame counter.
		frames++;
	}
	end_time = clock();

	// Grab the key that was pressed.
	getch();

	// Clean up.
	destroy_display_list(dl);
	destroy_bitmap(imgFrom);
	destroy_bitmap(imgFromSmall);
	destroy_bitmap(imgTo);
	destroy_bitmap(imgToSmall);
	destroy_bitmap(imgTile);
	destroy_bitmap(imgLight);
	destroy_bitmap(scrFlip);

	// Close down allegro.
	allegro_exit();

	// Report.
	printf("Display List, Copyright 1997 by Scott Deming.\n");
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

