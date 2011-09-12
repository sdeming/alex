/* map.c
 *
 * tiled map routines.
 *
 * Copyright 1997, by Scott Deming.
 * All Rights Reserved!
 *
 * You can use this code any way you wish in any form you wish.  I provide
 * this as a tutorial on the technique and nothing more.
 */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <io.h>
#include <unistd.h>

#include <alex.h>

/* WGT STUFF WILL BE REMOVED AS SOON AS A MAP EDITOR IS BUILT! */
typedef struct
{
	char r, b, g;
} WGTRGB;

typedef WGTRGB WGTPAL[255];

static WGTPAL wgt_pal;

// used for the line of sight routines.
static int map_pathblocked = 0;

// load a map and return a pointer to it.  If tilefile != NULL we load
// a tileset of that name and attach it to the map, and copy it's palette
// into pal.
MAP *load_map(char *mapfile, char *tilefile, PALETTE pal)
{
	int fh;
	MAP *map;
	TILESET *tileset = NULL;
	unsigned short magic;
	int br;
	int x, y;

	// allocate
	map = malloc(sizeof(MAP));
	if (map == NULL)
	{
		return NULL;
	}

	// initialize
	map->width = 0;
	map->height = 0;
	map->t_width = 0;
	map->t_height = 0;
	map->w_width = 0;
	map->w_height = 0;
	map->viewer_wx = 0;
	map->viewer_wy = 0;
	map->line_of_sight = 1;
	map->los_shading = 1;
	map->draw_style = MD_DEFAULT;
	map->trans_table = NULL;
	map->light_table = NULL;
	map->data = NULL;
	map->tileset = NULL;
	map->los_mask = NULL;

	// open map file
	fh = open(mapfile, O_BINARY);
	if (fh == -1)
	{
		free(map);
		return NULL;
	}

	// read magic number
	br = read(fh, &magic, 2);
	if (magic < 8974)
	{
		free(map);
		close(fh);

		return NULL;
	}

	// read map width/height
	br = read(fh, &map->width, 2);
	br = read(fh, &map->height, 2);

	// read map data
	map->data = malloc((map->width * map->height) * 2);
	br = read(fh, map->data, (map->width * map->height) * 2);

	// read tile types info
	br = read(fh, map->types, 256*2);
	//map->types[2] = 3;

	// close map file
	close(fh);

	// load tileset
	if (tilefile != NULL)
	{
		tileset = load_tileset(tilefile, pal);
	}
	map_settileset(map, tileset);

	// create line of sight mask
	map->los_mask = create_bitmap(map->width, map->height);
	for (y=0; y<map->height; y++)
	{
		for (x=0; x<map->width; x++)
		{
			map->los_mask->line[y][x] = map->types[map->data[x+y*map->width]];
		}
	}

	// return it
	return map;
}

// destroy a map and free up it's memory
void destroy_map(MAP *map)
{
	if (map == NULL)
		return;

	if (map->tileset != NULL)
		destroy_tileset(map->tileset);

	if (map->data != NULL)
		free(map->data);

	if (map->los_mask != NULL)
		destroy_bitmap(map->los_mask);

	free (map);
}

// load a tileset and return a pointer to it.  Copy it's palette into pal.
TILESET *load_tileset(char *tilefile, PALETTE pal)
{
	int fh, i;
	TILESET *tileset;
	BITMAP *tmpbmp = NULL;
	short version;
	char sig[14];
	int br;
	short w, h, used;
	short x, y;
	char *spr;

	// allocate
	//printf("* allocate\n");
	tileset = malloc(sizeof(TILESET));
	if (tileset == NULL)
		return NULL;

	//printf("* initialize\n");
	// initialize
	tileset->num_tiles = 0;

	//printf(" open\n");
	// open sprite file
	fh = open(tilefile, O_BINARY);
	if (fh == -1)
	{
		free(tileset);
		return NULL;
	}

	//printf(" read version\n");
	// read version
	br = read(fh, &version, 2);
	if (version < 4)
	{
		free(tileset);
		close(fh);
		return NULL;
	}

	// read signature
	br = read(fh, sig, 13);
	sig[13] = '\0';
	//printf(" read sig (%s)\n", sig);
	if (strcmp(sig, " Sprite File ") != 0)
	{
		free(tileset);
		close(fh);
		return NULL;
	}

	// read palette
	br = read(fh, wgt_pal, 768);
	for (i=0; i<256; i++)
	{
		pal[i].r = wgt_pal[i].r; // wgt's palettes have blue and green reversed.
		pal[i].g = wgt_pal[i].b;
		pal[i].b = wgt_pal[i].g;
	}
	//printf(" read pal\n");

	// get number of tiles
	br = read(fh, &tileset->num_tiles, 2);
	tileset->num_tiles++;
	//printf(" read num tiles: %d\n", tileset->num_tiles);

	// allocate all tiles
	//printf(" allocate tiles\n");
	tileset->tiles = malloc(sizeof(TILE) * tileset->num_tiles);

	// load each tile
	for (i=0; i<tileset->num_tiles; i++)
	{
		br = read(fh, &used, 2);
		if (used == 1)
		{
			// copy into allegro style bitmap
			br = read(fh, &w, 2);
			br = read(fh, &h, 2);
			if (tmpbmp == NULL)
			{
				tmpbmp = create_bitmap(w, h);
			}
			//tileset->tiles[i].bmp = create_bitmap(w, h);
			//printf(" --- tile: %d X %d == %d X %d\n", w, h, tileset->tiles[i].bmp->w, tileset->tiles[i].bmp->h);
			spr = malloc(w*h);
			br = read(fh, spr, w*h);
			for (y=0; y<h; y++)
			{
				for (x=0; x<w; x++)
				{
					//tileset->tiles[i].bmp->line[y][x] = spr[x+y*w];
					tmpbmp->line[y][x] = spr[x+y*w];
				}
			}
			tileset->tiles[i].bmp = get_rle_sprite(tmpbmp);

			free(spr);
		} else {
			//printf("*** NOT USED ***\n");
			tileset->tiles[i].bmp = NULL;
		}
	}

	// destroy workspace
	if (tmpbmp != NULL)
	{
		destroy_bitmap(tmpbmp);
	}

	// close it up
	//printf("* close\n");
	close(fh);

	return tileset;
}

// destroy a tileset and free up it's memory
void destroy_tileset(TILESET *tileset)
{
	int i;

	if (tileset == NULL)
		return;

	for (i=0; i<tileset->num_tiles; i++)
	{
		if (tileset->tiles[i].bmp != NULL)
		{
			destroy_rle_sprite(tileset->tiles[i].bmp);
		}
	}

	free(tileset->tiles);
}

void map_settileset(MAP *map, TILESET *tileset)
{
	int i;

	map->tileset = tileset;

	for (i=0; i<map->tileset->num_tiles; i++)
	{
		if (map->tileset->tiles[i].bmp != NULL)
		{
			map->t_width = map->tileset->tiles[i].bmp->w;
			map->sh_width = getbitcount(map->t_width)-1;
			map->t_height = map->tileset->tiles[i].bmp->h;
			map->sh_height = getbitcount(map->t_height)-1;
			map->w_width = map->width*map->t_width;
			map->w_height = map->height*map->t_height;
			break;
		}
	}
	//printf("tilesize = %d X %d\n", map->t_width, map->t_height);
	//printf("mapsize = %d X %d\n", map->width, map->height);
	//printf("worldsize = %lu X %lu\n", map->w_width, map->w_height);
}

void map_setstyle(MAP *map, int draw_style)
{
	map->draw_style = draw_style;
}

void map_settables(MAP *map, COLOR_MAP *trans_table, COLOR_MAP *light_table)
{
	map->trans_table = trans_table;
	map->light_table = light_table;
}

MAP_OBJECT *create_map_object(long wx, long wy, int is_blocking, int is_blockable)
{
	MAP_OBJECT *mobj;

	mobj = malloc(sizeof(MAP_OBJECT));
	if (mobj == NULL)
		return NULL;

	mobj->wx = wx;
	mobj->wy = wy;
	mobj->is_blocking = is_blocking;
	mobj->is_blockable = is_blockable;

	return mobj;
}

void destroy_map_object(MAP_OBJECT *mobj)
{
	if (mobj == NULL)
		return;

	free(mobj);
}

void set_map_object(MAP *map, MAP_OBJECT *mobj, int wx, int wy)
{
	mobj->wx = wx;
	mobj->wy = wy;
}

void move_map_object(MAP *map, MAP_OBJECT *mobj, int dx, int dy)
{
	long nx = mobj->wx+dx;
	long ny = mobj->wy+dy;

	// make sure we don't run off the map.
	nx = MIN(nx, map->w_width-1);
	nx = MAX(nx, 0);
	ny = MIN(ny, map->w_height-1);
	ny = MAX(ny, 0);

	// see if the object is allowed in it's new location
	if (mobj->is_blockable)
	{
		if (map_get_tiletype(map, nx, ny) != 0)
		{
			nx = mobj->wx;
			ny = mobj->wy;
		}
	}

	// set the new world coords for this object
	mobj->wx = nx;
	mobj->wy = ny;
}

short map_getx_offset(MAP *map, int wx)
{
	return wx - ((wx >> map->sh_width) << map->sh_width);
}

short map_gety_offset(MAP *map, int wy)
{
	return wy - ((wy >> map->sh_height) << map->sh_height);
}

short map_getx_tile(MAP *map, int wx)
{
	return wx >> map->sh_width;
}

short map_gety_tile(MAP *map, int wy)
{
	return wy >> map->sh_height;
}

char map_get_tiletype(MAP *map, int wx, int wy)
{
	int x = wx >> map->sh_width;
	int y = wy >> map->sh_height;

	return map->types[map->data[x+y*map->width]];
}

void map_testobstruction(BITMAP *bmp, int x, int y, int d)
{
	map_pathblocked += bmp->line[y][x];
}

int map_tilevisibility(MAP *map, int x, int y)
{
	int vx = map->viewer_wx >> map->sh_width;
	int vy = map->viewer_wy >> map->sh_height;

	// we don't want to block the tile we're casting from, so skip it.
	// this is inacurate I think
	if (x < vx)
	{
		x++;
	} else
	if (x > vx)
	{
		x--;
	}

	if (y < vy)
	{
		y++;
	} else
	if (y > vy)
	{
		y--;
	}

	// don't block the viewers tile either.
	if (x == vx && y == vy)
		return 0;

	// init
	//map_pathblocked = -(map->los_mask->line[y][x]);
	map_pathblocked = 0;

	// cast the line
	do_line(map->los_mask, x, y, vx, vy, 0, map_testobstruction);

	// reel it in
	return (MIN(map_pathblocked, MAP_NUMSHADES-1));
}

void render_map(MAP *map, BITMAP *scrBuffer, int sx, int sy, int wx, int wy, int mw, int mh)
{
	int mx = wx >> map->sh_width;
	int my = wy >> map->sh_height;

	switch (map->draw_style)
	{
	case MD_DEFAULT:
		render_map_default(map, scrBuffer, sx, sy, mx, my, mw, mh);
		break;
	}
}

void render_map_default(MAP *map, BITMAP *scrBuffer, int sx, int sy, int mx, int my, int mw, int mh)
{
	int x, y;
	short tilenum;
	int vis;
	COLOR_MAP *old = color_map;
	//char s[20];

	// set up the light table for darkening the tiles.
	color_map = map->light_table;

	// use the proper width/heights based on our starting point so we don't
	// overstep our bounds
	if (mx + mw-1 >= map->width)
		mw = map->width-mx;

	if (my + mh-1 >= map->height)
		mh = map->height-my;

	// loop through rows
	for (y=0; y<mh; y++)
	{
		// loop through columns (all optimizations should be done here)
		for (x=0; x<mw; x++)
		{
			// get the tile number for this row/column.
			tilenum = map->data[x+mx+((y+my)*map->width)];

			// calculate the visibility factor of this tile.
			vis = map->line_of_sight?map_tilevisibility(map, x+mx, y+my):0;
			if (vis == 0)
			{
				// draw the tile fully if it's totally visible.
				draw_rle_sprite(scrBuffer, map->tileset->tiles[tilenum].bmp, sx + (x<<map->sh_width), sy + (y<<map->sh_height));
			} else
			if (vis == MAP_NUMSHADES-1)
			{
				// draw a black rectangle for tiles that aren't at all visible.
				rectfill(scrBuffer, sx + (x<<map->sh_width), sy + (y<<map->sh_height), sx + (x<<map->sh_width)+map->t_width, sy + (y<<map->sh_height)+map->t_height, makecol(0, 0, 0));
			} else {
				// draw a darkened version of the tile based on the visibilty factor.  Higher number = darker tile.
				draw_lit_rle_sprite(scrBuffer, map->tileset->tiles[tilenum].bmp, sx + (x<<map->sh_width), sy + (y<<map->sh_height), 256-(vis*MAP_MULTSHADES));
			}

			//sprintf(s, "%d", map->types[tilenum]);
			//textout(scrBuffer, font, s, sx+(x*map->t_width), sy+(y*map->t_height), makecol(255, 255, 255));
		}
	}

	color_map = old;
}
