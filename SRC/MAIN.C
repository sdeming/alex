/* main.c
 *
 * Fade test program.
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
COLOR_MAP light_table;
COLOR_MAP trans_table;

void rgb_table_callback(int pos)
{
	int perc = (pos*100/255);

	gui_status_bar(screen, 20, 25, SCREEN_W-40, 0, "%", perc);
}

// it's ugly to put everything in main, but this isn't the important part. :)
void main(int argc, char *argv[])
{
	BITMAP *imgTile, *imgFrom, *imgFromSmall, *imgTo, *imgToSmall, *imgFade;
	BITMAP *imgLight;
	BITMAP *scrFlip;
	WINDOW *winFade;
	PALETTE palTile, palFrom, palTo;
	long frames = 0L;
	float start_time, end_time;
	int fade_frame = 0;
	int num_fade_frames = 16;
	int fade_dir = 1;
	RECT undo, dirty, render;
	display_list *dl;
	display_item *di;
	int f_dx = 1, f_dy = 1;
	int f_x, f_y;
	int t_dx = -1, t_dy = -1;
	int t_x, t_y;
	int i;

	// Default screen mode of 320x240, mode x
	screen_width = 320;
	screen_height = 240;

	// Default image size.
	image_width = 128;
	image_height = 128;

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

	printf("frames: %d\nscreen: %dX%d\n", num_fade_frames, screen_width, screen_height);

	// Initialize Allegro.	Great library!
	allegro_init();
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
	set_palette(palFrom);

	// Map colors for the gui routines.
	gui_mappalette();

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

	// Load "to" image, and scale it down.
	imgTo = load_bitmap("to.pcx", palTo);
	match_bitmap(imgTo, palTo);
	imgToSmall = create_bitmap(image_width, image_height);
	stretch_blit(imgTo, imgToSmall, 0, 0, imgTo->w, imgTo->h, 0, 0, imgToSmall->w, imgToSmall->h);

	// Allocate buffer for our off screen fade rendering and clear it.
	imgFade = create_bitmap(imgToSmall->w, imgToSmall->h);
	clear(imgFade);

	// Allocate buffer for our of screen rendering.
	scrFlip = create_bitmap(SCREEN_W, SCREEN_H);

	// Load "tile" image, match it to current palette, and tile the main screen.
	imgTile = load_bitmap("tile.bmp", palTile);
	match_bitmap(imgTile, palTile);
	tile_bitmap(imgTile, scrFlip);

	// Create the spotlight mouse bitmap.
	imgLight = create_bitmap(64, 64);
	clear_to_color(imgLight, 255);
	for (i=0; i<256; i++)
		circlefill(imgLight, 32, 32, 32-i/8, 255-i);

	// Create fade window.
	winFade = gui_create_window(imgFade->w+8, imgFade->h+8);
	gui_clear_window(winFade, 0);
	gui_move_window(winFade, (SCREEN_W>>1)-(winFade->window.w>>1), (SCREEN_H>>1)-(winFade->window.h>>1));

	// Draw a few boxes.
	gui_box(scrFlip, 0, 2, 150, 25, 25);
	gui_box(scrFlip, 1, 29, 150, 25, 25);
	gui_box(scrFlip, 0, SCREEN_W-28, 150, 25, 25);
	gui_box(scrFlip, 1, SCREEN_W-55, 150, 25, 25);

	// Blit initial screen.
	blit(scrFlip, screen, 0, 0, 0, 0, SCREEN_W, SCREEN_H);

	// initialize dirty rectangles.
	rect_dirty_max(&dirty);
	rect_dirty_max(&render);
	rect_dirty_max(&undo);

	text_mode(0);

	f_x = 0; f_y = 0;
	t_x = SCREEN_W-imgToSmall->w; t_y = 0;

	// main loop.
	start_time = clock();
	while (!kbhit())
	{
		// Fade the image to the current frame.
		fade_bitmap(imgFromSmall, imgToSmall, imgFade, 0, 0, fade_frame, num_fade_frames);
		gui_blit_window(winFade, imgFade, 0, 0, imgFade->w, imgFade->h);

		// Build display list.
		dl = create_display_list(&light_table, &trans_table);
		add_display_item(dl, create_display_item(winFade, DISPLAY_WINDOW, 0, 0, 0, 0, 1));
		add_display_item(dl, create_display_item(imgFromSmall, DISPLAY_BITMAP, f_x, f_y, 1, 0, 1));
		add_display_item(dl, create_display_item(imgToSmall, DISPLAY_BITMAP, t_x, t_y, 1, 0, 1));
		add_display_item(dl, create_display_item(imgLight, DISPLAY_LIGHT, mouse_x, mouse_y, 1, 0, 1));

		// Render display list.
		render_display_list(dl, scrFlip, &render);
		rect_combine(&dirty, &render, &undo);

		// Blit it.
		blit(scrFlip, screen, dirty.x, dirty.y, dirty.x, dirty.y, dirty.w, dirty.h);

		// Restore original background in off screen buffer.
		undo_render_display_list(dl, scrFlip, &undo);

		// Destroy display list.
		destroy_display_list(dl);

		// Move translucent boucing images.
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
	destroy_bitmap(imgTile);
	destroy_bitmap(imgLight);
	destroy_bitmap(scrFlip);

	// Close down allegro.
	allegro_exit();

	// Report.
	printf("Fade, Copyright 1997 by Scott Deming.\t\t\t\t");
	printf("Allegro version\n");
	printf("\nHow'd we do?\n");
	printf("===============================================================================\n");
	printf("  Time:  %3.2f\n", (float) ((end_time-start_time) / 100));
	printf("Frames:  %lu\n", frames);
	printf("   FPS:  %3.2f\n", (float) (float) frames / (float) ((end_time-start_time) / 100));
	printf("Screen:  %dX%d\n", screen_width, screen_height);
	printf("-------------------------------------------------------------------------------\n");
}

