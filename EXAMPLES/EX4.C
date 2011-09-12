/* ex4.c - Tile map example.
 *
 * Demonstrates using tiled maps for scrolling playfields.
 *
 * Keys:
 *	  arrows move the map around.
 *	  holding down ctrl will turn translucency on status window off.
 *	  holding down left shift will turn the 30 fps timer halt off.
 *	  holding down right shift will turn line of sight off.
 *	  pgup and pgdn increse and decrese dx and dy respectivly, making you move faster.
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
int lowest_idle = 9999999;
int highest_idle = 0;

int screen_width, screen_height;
int right_size, bottom_size;

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

int main(int argc, char *argv[])
{
	BITMAP *scrFlip;
	BITMAP *scrMap;
	PALETTE pal;
	WINDOW *winInfo;
	long frames = 0L;
	float start_time, end_time;
	MAP *map;
	MAP_OBJECT *player;
	DATAFILE *small_f, *large_f;
	int x = 0, y = 0;
	int dx = 1, dy = 1;
	int old_x = -1, old_y = -1, old_vx = -1, old_vy = -1;
	int view_w, view_h;
	int do_idle = 1;
	char s[128];
	RECT scroll_limits;

	// Default screen resolution to 320x200.
	screen_width = 320;
	screen_height = 200;

	// Read command line args.	Set screen size accordingly.
	if (argc > 2)
	{
		screen_width = atoi(argv[1]);
		screen_height = atoi(argv[2]);
	}
	right_size = 0;
	bottom_size = 0;

	printf("screen: %dX%d\n", screen_width, screen_height);

	// Initialize Allegro.	Great library!
	allegro_init();
	install_timer();
	install_mouse();
	install_keyboard();

	// Set graphics mode.
	if (set_gfx_mode(GFX_AUTODETECT, screen_width, screen_height, 0, 0) == -1)
	{
		printf("Cannot set video mode to %d, %d.\n", screen_width, screen_height);
		return 1;
	}

	// load fonts from ex.dat
	small_f = load_datafile_object("ex.dat", "arial12");
	if (small_f == NULL)
	{
		allegro_exit();
		printf("Error loading small font from ex.dat\n");
		return 1;
	}

	large_f = load_datafile_object("ex.dat", "arial26");
	if (large_f == NULL)
	{
		allegro_exit();
		printf("Error loading large font from ex.dat\n");
		return 1;
	}

	text_mode(-1);

	// load map
	map = load_map("tile1.wmp", "tile1.spr", pal);
	if (map == NULL)
	{
		allegro_exit();
		printf("Error loading map file.\n");
		return 1;
	}

	// set font and palette
	gui_setfont((FONT*) large_f->dat);
	gui_setpalette(pal);

	// Allocate buffers for our rendering.
	scrFlip = create_bitmap(SCREEN_W, SCREEN_H);
	clear(scrFlip);
	scrMap = create_bitmap(SCREEN_W + map->t_width*2, SCREEN_H + map->t_height*2);

	// Build rgb_table.
	text_mode(makecol8(0, 0, 0));
	textout_centre(screen, font, "Building RGB table.", SCREEN_W>>1, 10, 100);
	create_rgb_table(&rgb_table, pal, rgb_table_callback);
	rgb_map = &rgb_table;

	// Build lighting table.
	text_mode(makecol8(0, 0, 0));
	textout_centre(screen, font, "Building lighting table.", SCREEN_W>>1, 10, 100);
	create_light_table(&light_table, pal, 0, 0, 0, rgb_table_callback);

	// Build translucency table.
	text_mode(makecol8(0, 0, 0));
	textout_centre(screen, font, "Building translucency table.", SCREEN_W>>1, 10, 100);
	create_trans_table(&trans_table, pal, 128, 128, 128, rgb_table_callback);

	color_map = &trans_table;

	// initialize map for light and trans tables.
	map_settables(map, &trans_table, &light_table);

	// initialize gui for light and trans tables.
	gui_initialize(&light_table, &trans_table);

	// create info window
	winInfo = gui_create_window(128, 200, WB_THICK, WD_TRANSLUCENT);
	gui_move_window(winInfo, SCREEN_W-winInfo->window.w, 0);

	// set up scroll limits
	scroll_limits.x = 50;
	scroll_limits.y = 50;
	scroll_limits.w = (SCREEN_W-winInfo->window.w-100);
	scroll_limits.h = (SCREEN_H-100);

	// set up vars.
	view_w = scrMap->w/map->t_width;
	view_h = scrMap->h/map->t_height;

	// set up player
	player = create_map_object(0, 0, 0, 1);

	// setup time_to_blit interrupt.
	LOCK_VARIABLE(time_to_blit);
	LOCK_FUNCTION(time_to_blit_timer);
	install_int_ex(time_to_blit_timer, BPS_TO_TIMER(30));

	// set gui font to small_f;
	gui_setfont((FONT*) small_f->dat);

	missed_frames = 0;
	// main loop.
	start_time = clock();
	while (1)
	{
		idle_time = 0;
		if (do_idle)
			while (!time_to_blit) idle_proc(); // lock it in at around 30 fps.
		time_to_blit = 0;
		if (idle_time < lowest_idle)
			lowest_idle = idle_time;
		if (idle_time > highest_idle)
			highest_idle = idle_time;

		// render map
		map->viewer_wx = player->wx;
		map->viewer_wy = player->wy;
		if (map_getx_tile(map, x) != old_x || map_gety_tile(map, y) != old_y ||
			map_getx_tile(map, map->viewer_wx) != old_vx || map_gety_tile(map, map->viewer_wy) != old_vy)
		{
			render_map(map, scrMap, 0, 0, x, y, view_w, view_h);
			old_x = map_getx_tile(map, x);
			old_y = map_gety_tile(map, y);
			old_vx = map_getx_tile(map, map->viewer_wx);
			old_vy = map_gety_tile(map, map->viewer_wy);
		}
		blit(scrMap, scrFlip, map_getx_offset(map, x), map_gety_offset(map, y), 0, 0, SCREEN_W-right_size, SCREEN_H-bottom_size);
		rectfill(scrFlip, player->wx-x, player->wy-y, player->wx-x+5, player->wy-y+5, makecol8(255, 0, 0));

		// fill the info window with information
		gui_clear_window(winInfo, makecol8(0, 0, 0));
		gui_textout_centre_window(winInfo, "Map Demo", 2, makecol8(255, 0, 0), -1);
		sprintf(s, "X: %d, Y: %d", x, y);
		gui_textout_centre_window(winInfo, s, 14, makecol8(0, 0, 255), -1);
		sprintf(s, "WX: %ld, WY: %ld", player->wx, player->wy);
		gui_textout_centre_window(winInfo, s, 26, makecol8(0, 0, 255), -1);
		sprintf(s, "SW: %d, SH: %d", map->sh_width, map->sh_height);
		gui_textout_centre_window(winInfo, s, 48, makecol8(0, 0, 255), -1);
		sprintf(s, "DX: %d, DY: %d", dx, dy);
		gui_textout_centre_window(winInfo, s, 60, makecol8(0, 0, 255), -1);
		sprintf(s, "X Ofs: %d", map_getx_offset(map, x));
		gui_textout_centre_window(winInfo, s, 72, makecol8(0, 0, 255), -1);
		sprintf(s, "Y Ofs: %d", map_gety_offset(map, y));
		gui_textout_centre_window(winInfo, s, 84, makecol8(0, 0, 255), -1);
		sprintf(s, "Idle: %d", idle_time);
		gui_textout_centre_window(winInfo, s, 96, makecol8(0, 255, 255), -1);
		sprintf(s, "Missed: %d", missed_frames);
		gui_textout_centre_window(winInfo, s, 108, makecol8(0, 255, 255), -1);
		gui_draw_window(winInfo, scrFlip);

		// blit it
		//vsync();
		blit(scrFlip, screen, 0, 0, 0, 0, SCREEN_W, SCREEN_H);

		// check keys
		if (key[KEY_SPACE])
		{
			break;
		}

		if (key[KEY_LSHIFT])
			do_idle = 0;
		else
			do_idle = 1;

		if (key[KEY_RSHIFT])
			map->line_of_sight = 0;
		else
			map->line_of_sight = 1;

		if (key[KEY_CONTROL])
			gui_set_window_styles(winInfo, -1, WD_BLIT);
		else
			gui_set_window_styles(winInfo, -1, WD_TRANSLUCENT);

		if (key[KEY_PGDN])
		{
			dx = MAX(dx-1, 1);
			dy = MAX(dy-1, 1);
		}

		if (key[KEY_PGUP])
		{
			dx = MIN(dx+1, map->t_width);
			dy = MIN(dy+1, map->t_height);
		}

		if (key[KEY_RIGHT])
		{
			move_map_object(map, player, dx, 0);
			if (player->wx-x > RECT_X2(scroll_limits))
			{
				x = player->wx - RECT_X2(scroll_limits);
				if (x > map->w_width-SCREEN_W-1)
				{
					x = map->w_width-SCREEN_W-1;
				}
			}
		}

		if (key[KEY_DOWN])
		{
			move_map_object(map, player, 0, dy);
			if (player->wy-y > RECT_Y2(scroll_limits))
			{
				y = player->wy - RECT_Y2(scroll_limits);
				if (y > map->w_height-1-SCREEN_H)
				{
					y = map->w_height-1-SCREEN_H;
				}
			}
		}

		if (key[KEY_LEFT])
		{
			move_map_object(map, player, -dx, 0);
			if (player->wx-x < scroll_limits.x)
			{
				x = MAX(player->wx-scroll_limits.x, 0);
			}
		}

		if (key[KEY_UP])
		{
			move_map_object(map, player, 0, -dy);
			if (player->wy-y < scroll_limits.y)
			{
				y = MAX(player->wy-scroll_limits.y, 0);
			}
		}

		// Increment frame counter.
		frames++;
	}
	end_time = clock();

	// Clean up.
	unload_datafile_object(small_f);
	unload_datafile_object(large_f);
	destroy_map(map);
	destroy_map_object(player);
	destroy_bitmap(scrFlip);
	destroy_bitmap(scrMap);

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
	printf("  Idle:  %d (lowest was %d, highest was %d)\n", idle_time, lowest_idle, highest_idle);
	printf("Screen:  %dX%d\n", screen_width, screen_height);
	printf("-------------------------------------------------------------------------------\n");
	printf("Note:  If \"Idle\" starts to get close to zero then things should be done to\n");
	printf("       speed things up.  When idle proc hits zero, we'll start missing frames\n");
	printf("       (indicated by \"Missed\") and things can start to get a bit jerky.\n");
	printf("       If \"Missed\" is greater than 0, start optimizing.\n");
	printf("       If you turned off the 30 fps timer halt then your lowest idle will be 0.\n");

	return 0;
}

