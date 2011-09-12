ALEX -- (AL)legro (EX)tras, by Scott Deming
-----------------------------------------------------------------------------
Special thanks to Shawn Hargreaves for making Allegro, and DJ Delorie
for making DJGPP.  And thank everyone in the world who releases source
code for free!	I really get annoyed when people insist on keeping things
trivial to themselves when others can learn a great deal from them.
=============================================================================

Contacting me:

I can be contacted via email at "sdeming@concentric.net".  I'll respond
to everything probably withing 24 hours, unless it's the weekend in which
case it may take me a bit longer (or shorter it really depends).

Also check out "htpp://www.concentric.net/~sdeming"

-----------------------------------------------------------------------------

No real documentation other than a function listing has been written at this
time.  The examples in the "examples" directory use every function, I think,
so looking at those should provide some kind of detail of what each function
does and how it's called.  Documentation is needed badly, but since this is
at it's basic roots right now, and I keep changing fundamental items, it's
really not feasable to write it.

Functionality:

- Display List
- Very (basic) GUI
	Windows, Status Bar, and Image Tiling
- Special Effects
	Image Cross Fading
- Utility
	Image/Palette Matching
- 2D Worlds
	Flat surface tiled maps
=============================================================================

Display List
-----------------------------------------------------------------------------

The display list routines are created for maintaining a list of objects (ie,
windows, and bitmaps) and being able to blit them all in one pass.	Support
for dirty rectangles is provided for optimum performance.  The display list
routines provide a simple management tool for all items to be displayed.

With the display list hittest routine you can easily implement collision
detection.

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

TODO:
- Support for mouse and message passing to complex objects (complex objects
  will be defined as anything other than a simple BITMAP)
- Automatic collision detection notification.

Very basic (for now) GUI routines
-----------------------------------------------------------------------------

These gui routines don't do much but draw frames and blit.  Very basic
textout functionality is provided for the windows, though there is no
scrolling or anything like that.  Any text displayed off the window is
simply clipped.  Different border styles are available, but only three are
working at the moment (WB_NONE, WB_THIN, and WB_THICK).  Drawing styles
include WD_BLIT for fast blitting, WD_SPRITE for transparency checking on
color 0, and WD_TRANSLUCENT to blit a bitmap with translucency (note, this
requires gui_initialize() to be called with valid pointers to COLOR_MAPs.

There is also a tile_bitmap() function provided which will tile a bitmap
over and over on another bitmap filling it with the tile.

void gui_initialize(FONT *f, COLOR_MAP *light_table, COLOR_MAP *trans_table);
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

void tile_bitmap(BITMAP *tile, BITMAP *scr);

TODO:
- Window managing, using display list
- Support for custom controls (Buttons, check boxes, scroll bars, etc)
- Turn status bar into a control

Special Effects
-----------------------------------------------------------------------------
There is currently only one special effect routine, which is a simple image
crossfade routine.	This is very slow unless you call allegro's
create_rgb_table() function and build an rgb table.  It's pretty though. :)

int fade_bitmap(BITMAP *imgFrom, BITMAP *imgTo, BITMAP *imgBuffer, int x_start, int y_start, int frame, int num_frames);

TODO:
- scatter/unscatter bitmap (uses a lot of memory, needs an x,y,dx,dy,col for
  each pixel in the bitmap.  dx and dy are fixed point numbers, x and y are
  shorts, col is a short so your looking at 14 bytes per pixel minimum.
- many more things I'm sure

Utility
-----------------------------------------------------------------------------
Only one utility currently exists, though I have a feeling that this section
will grow rapidly.	The match_bitmap() function will match a bitmap with it's
palette to the current palette as closely as it can.  It's much faster if
you call allegro's create_rgb_table() function first.

void match_bitmap(BITMAP *img, PALETTE palFrom);

TODO:
- Nothing planned

2D Worlds
-----------------------------------------------------------------------------
Currently the map routines in ALEX use the files created by WGT4.0+'s map
editor and sprite editor.  I plan to change this in the future because those
files are severely limited.  However, I don't have yet to see a map editor
that satisfies me, and I don't particularly want to write one.  Though I
just might have to if I want this section to expand the way I plan.  Right
now these routines work perfectly on every WGT map (wmp) and sprite (spr)
file I've tried.  See ex4.c and ex5.c in the examples directory to see them
implemented.  With these routines you can have unlimited layers, limited only
by memory and speed.  ex5.c demonstrates two layers.  Until I get these
routines optimized heavily they may seem kind of slow.	Though on my p120
laptop (lousy video write time) I don't miss any frames at all using full
screen 320x200, 2 layers, and a 30fps timer halt.

I have a lot of plans for this section and will always listen to input from
others, and I'll be more than happy to accept help on anything.  Especially
a decent tile map editor!

MAP *load_map(char *mapfile, char *tilefile, PALETTE pal);
void destroy_map(MAP *map);
TILESET *load_tileset(char *tilefile, PALETTE pal);
void destroy_tileset(TILESET *tileset);
void map_settileset(MAP *map, TILESET *tileset);
void map_setstyle(MAP *map, int draw_style);
void map_settables(MAP *map, COLOR_MAP *trans_table, COLOR_MAP *light_table);
unsigned short map_getx_offset(MAP *map, int wx);
unsigned short map_gety_offset(MAP *map, int wy);
unsigned short map_getx_tile(MAP *map, int wx);
unsigned short map_gety_tile(MAP *map, int wy);
void render_map(MAP *map, BITMAP *scrBuffer, int sx, int sy, int wx, int wy, int mw, int mh);
void render_blit_map(MAP *map, BITMAP *scrBuffer, int sx, int sy, int mx, int my, int mw, int mh);
void render_sprite_map(MAP *map, BITMAP *scrBuffer, int sx, int sy, int mx, int my, int mw, int mh);
void render_trans_map(MAP *map, BITMAP *scrBuffer, int sx, int sy, int mx, int my, int mw, int mh);

TODO:
- Add error checking in load_map() and load_tileset() routines.  IMPORTANT!
- Heavily optimize, perhaps even assembly for the render functions
- Object manager
- Animated tiles
- Tile based collision checking (kind of there with the map_get?_tile() routines)
- Pixel based collision checking
- Best path from point a to point b
- Lighting (Objects or tile luminance levels/directions)
- Integrate a "chunk" type system to allow larger worlds in less memory
- Build a better map editor than the WGT editor, and use a more advanced file
  format with more features
- Isometric view?  Never attempted to figure this out, but it can't be that
  difficult once you get the paint order down

Tools
-----------------------------------------------------------------------------
bmp2pcx.c -- convert a bmp file to a pcx file.

Examples
-----------------------------------------------------------------------------
In the examples directory you will find a few example programs.

ex1.c -- Demonstrates the crossfade effect, using from.pcx and to.pcx.
ex2.c -- Demonstrates using a display list.  Notice the images change from
	 translucent to solid whenever it has the mouse focus.	Also notice
	 that only the "top" image becomes solid, and the bottom image stays
	 translucent when the mouse is above both.	Click the mouse button
	 to bring the focused image to the top.  Notice that the mouse ends
	 up behind the new top image?  That's because the mouse cursor is
	 part of that same display list.
ex3.c -- Demonstrates using multiple display lists.  This example is very
	 much the same as ex2.c but with two display lists the mouse will
	 ALWAYS remain on top.
ex4.c -- Tile based map demonstration using tile1.wmp and tile1.spr as the
	 it's map/tileset respectively.  Move around with the arrow keys,
	 hold ctrl down to toggle translucency on the status window, hold
	 down the left shift key to toggle the 30 fps timer halt, hold down
	 right the right shift key to toggle the line of sight and use
	 pgup/pgdn to increse/decrese the dx/dy values (delta's for moving
	 the map around).
ex5.c -- Same as ex4.c but it uses "tile1ovr.wmp" as an overlay map.  You
	 may also notice that in ex5.c when we load the map for the overlay
	 we specify NULL for the tileset filename and then simply assign
	 the tileset from the map to the overlay, so they share the same
	 tileset.  This can save on memory, but could prove to be hazardous
	 if you destroy_map() on both of the maps without first setting the
	 tileset on one of them to NULL.
