#ifndef __alex_h_
#define __alex_h_

#include <allegro.h>

#define __ALEX__
#define ALEX_TITLE			"Alex (Allegro Extras)"
#define ALEX_VERSION		0x0002
#define ALEX_VERSION_STR	"0.2"

#define DISPLAY_BITMAP		0		// an actual image
#define DISPLAY_LIGHT		5		// a pixel map representing light levels
#define DISPLAY_WINDOW		100 	// gui window

// border styles
#define WB_NONE 			0
#define WB_THIN 			1
#define WB_NORMAL			2
#define WB_RECESSED 		3
#define WB_THICK			4

// drawing styles
#define WD_BLIT 			0
#define WD_SPRITE			1
#define WD_TRANSLUCENT		2

// hit tests
#define WHT_MISS			0
#define WHT_BORDER			1
#define WHT_USER			2

// messages
#define WMSG_NONE			0x00000000
#define WMSG_IDLE			0x00000001

// focus messages
#define WMSG_GOTFOCUS		0x00000051
#define WMSG_LOSTFOCUS		0x00000052

// mouse messages
#define WMSG_MOUSEOVER		0x00000100
#define WMSG_MOUSELDOWN 	0x00000101
#define WMSG_MOUSELUP		0x00000102
#define WMSG_MOUSELCLICK	0x00000103
#define WMSG_MOUSEMDOWN 	0x00000111
#define WMSG_MOUSEMUP		0x00000112
#define WMSG_MOUSEMCLICK	0x00000113
#define WMSG_MOUSERDOWN 	0x00000121
#define WMSG_MOUSERUP		0x00000122
#define WMSG_MOUSERCLICK	0x00000123

// user messages starting point
#define WMSG_USER			0x00010000

// map drawing styles
#define MD_DEFAULT			0

// map defaults
#define MAP_NUMSHADES		8
#define MAP_MULTSHADES		(256/MAP_NUMSHADES)

// rectangle x2/y2's
#define RECT_X2(r) (r.x+r.w-1)
#define RECT_Y2(r) (r.y+r.h-1)

// primitive structures
typedef struct
{
	int x, y, w, h;
} RECT;

typedef struct
{
	int x, y;
} POINT;

typedef struct
{
	int w, h;
} SIZE;

// notes:
// -translucent images cannot be scaled currently, so images with both flags
//	set will be scaled, but not not blitted translucently.
// -DISPLAY_LIGHT images will be displayed as regular bitmaps if the
//	is_scaled flag is set.
typedef struct display_item
{
	void *pContents;			// bitmap to display.
	BITMAP *pOldContents;		// bitmap saving what was under this item.
	int x, y;					// location to display it at.
	int w, h;					// modified width/height if we want to scale.
	int type;					// display type

	int is_sprite:1;			// Is this a sprite?
	int is_translucent:1;		// Display this in translucent mode?
	int is_scaled:1;			// Is this scaled?	If so, use w, h.
	int is_saving:1;			// Is this saving the old bitmap?

	struct display_item *pNext; // Next in list.
	struct display_item *pPrev; // Pervious in list.

	struct display_list *pParent;		// Parent display list.
} display_item;

typedef struct display_list
{
	COLOR_MAP *display_prev_table;
	COLOR_MAP *display_light_table;
	COLOR_MAP *display_trans_table;

	display_item *pItem;
} display_list;

// the window structure
typedef struct WINDOW
{
	RECT window;			// entire window
	RECT user;				// user area inside of window

	int border_style;		// style of windows border
	int draw_style; 		// method used to draw window to the screen

	BITMAP *contents;		// contents of window

	// message handler for this window.
	int (*msghandler)(display_item *pDI, unsigned long msg, unsigned short _sh, unsigned long _ln);

	// service routines for various message types.
	int (*on_idle)(display_item *pDI);

	int (*on_mouseover)(display_item *pDI, int x, int y);
	int (*on_mouseldown)(display_item *pDI, int x, int y);
	int (*on_mouselup)(display_item *pDI, int x, int y);
	int (*on_mouselclick)(display_item *pDI, int x, int y);
	int (*on_mouserdown)(display_item *pDI, int x, int y);
	int (*on_mouserup)(display_item *pDI, int x, int y);
	int (*on_mouserclick)(display_item *pDI, int x, int y);
	int (*on_mousemdown)(display_item *pDI, int x, int y);
	int (*on_mousemup)(display_item *pDI, int x, int y);
	int (*on_mousemclick)(display_item *pDI, int x, int y);
} WINDOW;

typedef struct
{
	BITMAP *bmp;

	int x, y;
} SPRITE;

typedef struct
{
	RLE_SPRITE *bmp;					// the bitmap
} TILE;

typedef struct
{
	short num_tiles;					// number of tiles in set
	TILE *tiles;						// array of tiles
} TILESET;

typedef struct
{
	short width, height;				// width and height of map, in tiles
	long w_width, w_height; 			// world width and height, in pixels
	short t_width, t_height;			// width and height of tiles

	char sh_width;						// number of bits to shift for
	char sh_height; 					// multiplying by the width or
										// height of a tile.

	long viewer_wx, viewer_wy;			// viewers location, in world coordinates

	unsigned short line_of_sight:1; 	// use line of sight?
	unsigned short los_shading:1;		// use shading?

	short draw_style;					// drawing style

	COLOR_MAP *trans_table; 			// color table for translucency
	COLOR_MAP *light_table; 			// color table for lighting

	BITMAP *los_mask;					// line of sight mask

	short *data;						// map data

	TILESET *tileset;					// tileset
	unsigned short types[256];
} MAP;

typedef struct
{
	long wx, wy;						// world x and y coords.

	int is_blocking:1;					// does this object block other objects?
	int is_blockable:1; 				// can this object be blocked?
} MAP_OBJECT;

/***********/
/* sptypes ******************************************************************/
/***********/

int minmax(int val, int min, int max);

int getbitcount(int num);

void rect_combine(RECT *r, RECT *left, RECT *right);
void rect_copy(RECT *s, RECT *d);
void rect_expand(RECT *r, int size);

void rect_dirty_max(RECT *r);

/*******/
/* gui **********************************************************************/
/*******/

void gui_initialize(COLOR_MAP *light_table, COLOR_MAP *trans_table);
void gui_setdisplaylist(display_list *dl);
void gui_setfont(FONT *f);
void gui_mappalette(void);
void gui_setpalette(PALETTE pal);

int gui_hittest(WINDOW *win, int x, int y);

void gui_set_window_styles(WINDOW *win, int border_style, int draw_style);
WINDOW *gui_create_window(int w, int h, int border_style, int draw_style);
void gui_destroy_window(WINDOW *win);
void gui_move_window(WINDOW *win, int x, int y);
void gui_clear_window(WINDOW *win, int color);
void gui_draw_window(WINDOW *win, BITMAP *scrBuffer);
void gui_blit_window(WINDOW *win, BITMAP *img, int x, int y, int w, int h);
void gui_sprite_window(WINDOW *win, BITMAP *img, int x, int y);
void gui_trans_sprite_window(WINDOW *win, BITMAP *img, int x, int y);
void gui_textout_window(WINDOW *win, char *s, int x, int y, int col, int scol);
void gui_textout_centre_window(WINDOW *win, char *s, int y, int col, int scol);

void gui_frame(BITMAP *scrBuffer, int border_style, int selected, int x, int y, int w, int h);
void gui_box(BITMAP *scrBuffer, int inverse, int x, int y, int w, int h);

void gui_status_bar(BITMAP *scrBuffer, int x, int y, int w, int h, char *text, int percent);

int gui_sendmsg(display_item *pDI, unsigned long msg, unsigned short _sh, unsigned long _ln);
int gui_defmsghandler(display_item *pDI, unsigned long msg, unsigned short _sh, unsigned long _ln);

/********/
/* tile *********************************************************************/
/********/

void tile_bitmap(BITMAP *tile, BITMAP *scr);

/*********/
/* match ********************************************************************/
/*********/

void match_bitmap(BITMAP *img, PALETTE palFrom);

/***********/
/* display ******************************************************************/
/***********/

display_list *create_display_list(COLOR_MAP *light_table, COLOR_MAP *trans_table);

void draw_display_bitmap(display_item *pDI, int undo, BITMAP *pScreen, RECT *pRect);
void draw_display_window(display_item *pDI, int undo, BITMAP *pScreen, RECT *pRect);

void render_display_list(display_list *pDL, BITMAP *pScreen, RECT *pRect);
void undo_render_display_list(display_list *pDL, BITMAP *pScreen, RECT *pRect);

display_item *create_display_item(void	*pContents, int type, int x, int y, int is_translucent, int is_sprite, int is_saving);
display_item *add_display_item(display_list *pDL, display_item *pItem);
display_item *insert_display_item(display_list *pDL, display_item *pItem);
display_item *remove_display_item(display_item *pDI);

int destroy_display_list(display_list *pDL);
int destroy_display_item(display_item *pItem);

void scale_display_item(display_item *pItem, int w, int h);
void move_display_item(display_item *pDI, int x, int y);

int display_hittest(display_item *pDI, int x, int y);

display_item *get_bottom_window(display_list *pDL, int x, int y);
display_item *get_top_window(display_list *pDL, int x, int y);

void bring_window_to_top(display_item *pDI);
void send_window_to_bottom(display_item *pDI);

void set_all_windows_styles(display_list *pDL, int border_style, int draw_style);

/********/
/* fade *********************************************************************/
/********/

int fade_bitmap(BITMAP *imgFrom, BITMAP *imgTo, BITMAP *imgBuffer, int x_start, int y_start, int frame, int num_frames);

/**********/
/* sprite *******************************************************************/
/**********/

SPRITE *create_sprite(BITMAP *bmp, int x, int y);
void destroy_sprite(SPRITE *spr);
void move_sprite(SPRITE *spr, int x, int y);

SPRITE *outline_sprite(SPRITE *spr);

/*******/
/* map **********************************************************************/
/*******/

MAP *load_map(char *mapfile, char *tilefile, PALETTE pal);
void destroy_map(MAP *map);
TILESET *load_tileset(char *tilefile, PALETTE pal);
void destroy_tileset(TILESET *tileset);

void map_settileset(MAP *map, TILESET *tileset);
void map_setstyle(MAP *map, int draw_style);
void map_settables(MAP *map, COLOR_MAP *trans_table, COLOR_MAP *light_table);

MAP_OBJECT *create_map_object(long wx, long wy, int is_blocking, int is_blockable);
void destroy_map_object(MAP_OBJECT *mobj);
void set_map_object(MAP *map, MAP_OBJECT *mobj, int wx, int wy);
void move_map_object(MAP *map, MAP_OBJECT *mobj, int dx, int dy);

short map_getx_offset(MAP *map, int wx);
short map_gety_offset(MAP *map, int wy);
short map_getx_tile(MAP *map, int wx);
short map_gety_tile(MAP *map, int wy);
char map_get_tiletype(MAP *map, int wx, int wy);

void render_map(MAP *map, BITMAP *scrBuffer, int sx, int sy, int wx, int wy, int mw, int mh);
void render_map_default(MAP *map, BITMAP *scrBuffer, int sx, int sy, int mx, int my, int mw, int mh);

#endif
