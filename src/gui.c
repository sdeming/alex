/* gui.c
 *
 * gui type routines for games.
 *
 * Copyright 1997, by Scott Deming.
 * All Rights Reserved!
 *
 * You can use this code any way you wish in any form you wish.  I provide
 * this as a tutorial on the technique and nothing more.
 */

#include <stdio.h>

#include <alex.h>

#define GUI_FONT (gui_font?gui_font:font)

// hz for the poster to execute
#define GUI_POSTER_FREQUENCY		10

static RGB gui_rgb_white = {255, 255, 255};
static RGB gui_rgb_black = {0, 0, 0};
static RGB gui_rgb_dgrey = {64, 64, 64};
static RGB gui_rgb_grey = {128, 128, 128};
static RGB gui_rgb_lgrey = {192, 192, 192};
static RGB gui_rgb_fill = {0, 0, 255};

static int gui_col_dg;				// dark grey
static int gui_col_g;				// grey
static int gui_col_lg;				// light grey
static int gui_col_w;				// white
static int gui_col_b;				// black
static int gui_col_fill;			// fill (used for status bar)
static int gui_col_border;			// border color (used for thin border)
static int gui_col_select_border;	// selected border color

static COLOR_MAP *gui_light_table = NULL;
static COLOR_MAP *gui_trans_table = NULL;

static FONT *gui_font = NULL;

static display_list *gui_display_list = NULL;

static int gui_posterinstalled = 0;

void gui_msgposter()
{
	display_item *di;
	WINDOW *win;
	static int _mouse_x = 0, _mouse_y = 0, _mouse_b = 0;

	if (gui_display_list == NULL)
		return;

	di = get_top_window(gui_display_list, mouse_x, mouse_y);
	if (di == NULL)
		return;

	win = (WINDOW*) di->pContents;
	if (win == NULL)
		return;

	// WMSG_MOUSEOVER
	if (mouse_x != _mouse_x || mouse_y != _mouse_y)
	{
		gui_sendmsg(di, WMSG_MOUSEOVER, mouse_x, mouse_y);
		if (mouse_b & 1)
		{
			// WMSG_MOUSELDOWN
			gui_sendmsg(di, WMSG_MOUSELDOWN, mouse_x, mouse_y);
		}
		if (mouse_b & 2)
		{
			// WMSG_MOUSERDOWN
			gui_sendmsg(di, WMSG_MOUSERDOWN, mouse_x, mouse_y);
		}
		if (mouse_b & 3)
		{
			// WMSG_MOUSEMDOWN
			gui_sendmsg(di, WMSG_MOUSEMDOWN, mouse_x, mouse_y);
		}
		_mouse_x = mouse_x;
		_mouse_y = mouse_y;
		_mouse_b = mouse_b;
	} else
	{
		// left button
		if ((mouse_b & 1) && !(_mouse_b & 1))
		{
			// WMSG_MOUSELDOWN
			gui_sendmsg(di, WMSG_MOUSELDOWN, mouse_x, mouse_y);
		} else
		if ((!mouse_b & 1) && (_mouse_b & 1))
		{
			// WMSG_MOUSELUP & WMSG_MOUSERCLICK
			gui_sendmsg(di, WMSG_MOUSELUP, mouse_x, mouse_y);
			gui_sendmsg(di, WMSG_MOUSELCLICK, mouse_x, mouse_y);
		}

		// right button
		if ((mouse_b & 2) && !(_mouse_b & 2))
		{
			// WMSG_MOUSERDOWN
			gui_sendmsg(di, WMSG_MOUSERDOWN, mouse_x, mouse_y);
		} else
		if ((!mouse_b & 2) && (_mouse_b & 2))
		{
			// WMSG_MOUSERUP & WMSG_MOUSERCLICK
			gui_sendmsg(di, WMSG_MOUSERUP, mouse_x, mouse_y);
			gui_sendmsg(di, WMSG_MOUSERCLICK, mouse_x, mouse_y);
		}

		// middle button
		if ((mouse_b & 4) && !(_mouse_b & 4))
		{
			// WMSG_MOUSEMDOWN
			gui_sendmsg(di, WMSG_MOUSEMDOWN, mouse_x, mouse_y);
		} else
		if ((!mouse_b & 4) && (_mouse_b & 4))
		{
			// WMSG_MOUSEMUP & WMSG_MOUSEMCLICK
			gui_sendmsg(di, WMSG_MOUSEMUP, mouse_x, mouse_y);
			gui_sendmsg(di, WMSG_MOUSEMCLICK, mouse_x, mouse_y);
		}

		_mouse_b = mouse_b;
	}
}
END_OF_FUNCTION(gui_msgposter);

void gui_initialize(COLOR_MAP *light_table, COLOR_MAP *trans_table)
{
	// map gui colors to the current palette.
	gui_mappalette();

	// set default color tables.
	gui_light_table = light_table;
	gui_trans_table = trans_table;

	// install message poster, but make sure we only do it once.
	if (gui_posterinstalled == 0)
	{
		LOCK_VARIABLE(gui_display_list);
		LOCK_FUNCTION(gui_msgposter);
		install_int_ex(gui_msgposter, BPS_TO_TIMER(GUI_POSTER_FREQUENCY));
		gui_posterinstalled = 1;
	}
}

void gui_setdisplaylist(display_list *dl)
{
	gui_display_list = dl;
}

void gui_setfont(FONT *f)
{
	if (f != NULL)
		gui_font = f;
}

void gui_mappalette()
{
	gui_col_dg = makecol(gui_rgb_dgrey.r, gui_rgb_dgrey.g, gui_rgb_dgrey.b);
	gui_col_g = makecol(gui_rgb_grey.r, gui_rgb_grey.g, gui_rgb_grey.b);
	gui_col_lg = makecol(gui_rgb_lgrey.r, gui_rgb_lgrey.g, gui_rgb_lgrey.b);
	gui_col_b = makecol(gui_rgb_black.r, gui_rgb_black.g, gui_rgb_black.b);
	gui_col_w = makecol(gui_rgb_white.r, gui_rgb_white.g, gui_rgb_white.b);
	gui_col_fill = makecol(gui_rgb_fill.r, gui_rgb_fill.g, gui_rgb_fill.b);
	gui_col_border = makecol(gui_rgb_black.r, gui_rgb_black.g, gui_rgb_black.b);
	gui_col_select_border = makecol(gui_rgb_white.r, gui_rgb_white.g, gui_rgb_white.b);
}

void gui_setpalette(PALETTE pal)
{
	set_palette(pal);
	gui_mappalette();
}

int gui_hittest(WINDOW *win, int x, int y)
{
	if ((x > win->window.x) && (x < (win->window.x+win->window.w-1)) &&
		(y > win->window.y) && (y < (win->window.y+win->window.h-1)))
	{
		if ((x > win->user.x) && (x < (win->user.x+win->user.w-1)) &&
			(y > win->user.y) && (x < (win->user.y+win->user.h-1)))
		{
			return WHT_USER;
		}

		return WHT_BORDER;
	}

	return WHT_MISS;
}

void gui_set_window_styles(WINDOW *win, int border_style, int draw_style)
{
	// set border style
	if (border_style != -1)
	{
		win->border_style = border_style;

		// define window inside
		switch (win->border_style)
		{
		case WB_NONE:
			win->user.x = win->window.x;
			win->user.y = win->window.y;
			win->user.w = win->window.w;
			win->user.h = win->window.h;
			break;
		case WB_THIN:
			win->user.x = win->window.x+1;
			win->user.y = win->window.y+1;
			win->user.w = win->window.w-2;
			win->user.h = win->window.h-2;
			break;
		case WB_NORMAL:
		case WB_RECESSED:
			win->user.x = win->window.x+4;
			win->user.y = win->window.y+4;
			win->user.w = win->window.w-8;
			win->user.h = win->window.h-8;
			break;
		case WB_THICK:
			win->user.x = win->window.x+4;
			win->user.y = win->window.y+4;
			win->user.w = win->window.w-8;
			win->user.h = win->window.h-8;
			break;
		}
	}

	// set draw style
	if (draw_style != -1)
		win->draw_style = draw_style;
}

WINDOW *gui_create_window(int w, int h, int border_style, int draw_style)
{
	WINDOW *win = malloc(sizeof(WINDOW));
	if (win == NULL)
		return NULL;

	// define window border
	win->window.x = 0;
	win->window.y = 0;
	win->window.w = w;
	win->window.h = h;

	// set styles
	gui_set_window_styles(win, border_style, draw_style);

	// allocate contents bitmap
	win->contents = create_bitmap(win->user.w, win->user.h);
	if (win->contents == NULL)
	{
		free(win);
		return NULL;
	}

	win->msghandler = gui_defmsghandler;
	win->on_idle = NULL;
	win->on_mouseover = NULL;
	win->on_mouseldown = NULL;
	win->on_mouselup = NULL;
	win->on_mouselclick = NULL;
	win->on_mouserdown = NULL;
	win->on_mouserup = NULL;
	win->on_mouserclick = NULL;
	win->on_mousemdown = NULL;
	win->on_mousemup = NULL;
	win->on_mousemclick = NULL;

	return win;
}

void gui_destroy_window(WINDOW *win)
{
	if (win == NULL)
		return;

	if (win->contents == NULL)
		return;

	destroy_bitmap(win->contents);
	free(win);
}

void gui_move_window(WINDOW *win, int x, int y)
{
	win->window.x = x;
	win->window.y = y;

	switch (win->border_style)
	{
	case WB_NONE:
		win->user.x = win->window.x;
		win->user.y = win->window.y;
		break;
	case WB_THIN:
		win->user.x = win->window.x+1;
		win->user.y = win->window.y+1;
		break;
	case WB_NORMAL:
	case WB_RECESSED:
		win->user.x = win->window.x+4;
		win->user.y = win->window.y+4;
		break;
	case WB_THICK:
		win->user.x = win->window.x+4;
		win->user.y = win->window.y+4;
		break;
	}
}

void gui_clear_window(WINDOW *win, int color)
{
	clear_to_color(win->contents, color);
}

void gui_draw_window(WINDOW *win, BITMAP *scrBuffer)
{
	COLOR_MAP *old;

	gui_frame(scrBuffer, win->border_style, 0, win->user.x, win->user.y, win->user.w, win->user.h);

	switch (win->draw_style)
	{
	case WD_BLIT:
		blit(win->contents, scrBuffer, 0, 0, win->user.x, win->user.y, win->user.w, win->user.h);
		break;
	case WD_SPRITE:
		draw_sprite(scrBuffer, win->contents, win->user.x, win->user.y);
		break;
	case WD_TRANSLUCENT:
		old = color_map;
		color_map = gui_trans_table;
		draw_trans_sprite(scrBuffer, win->contents, win->user.x, win->user.y);
		color_map = old;
		break;
	}
}

void gui_blit_window(WINDOW *win, BITMAP *img, int x, int y, int w, int h)
{
	blit(img, win->contents, x, y, x, y, w, h);
}

void gui_sprite_window(WINDOW *win, BITMAP *img, int x, int y)
{
	draw_sprite(win->contents, img, x, y);
}

void gui_trans_sprite_window(WINDOW *win, BITMAP *img, int x, int y)
{
	draw_trans_sprite(win->contents, img, x, y);
}

void gui_textout_window(WINDOW *win, char *s, int x, int y, int col, int scol)
{
	// shadow
	if (scol != -1)
	{
		textout(win->contents, GUI_FONT, s, x+1, y+1, scol);
	}

	// text
	textout(win->contents, GUI_FONT, s, x, y, col);
}

void gui_textout_centre_window(WINDOW *win, char *s, int y, int col, int scol)
{
	// shadow
	if (scol != -1)
	{
		textout_centre(win->contents, GUI_FONT, s, (win->user.w/2)+1, y+1, scol);
	}

	// text
	textout_centre(win->contents, GUI_FONT, s, win->user.w/2, y, col);
}

void gui_frame(BITMAP *scrBuffer, int border_style, int selected, int x, int y, int w, int h)
{
	switch (border_style)
	{
	case WB_NONE:
		// do nothing
		break;
	case WB_THIN:
		x--; y--; w+=2; h+=2;
		rect(scrBuffer, x, y, x+w-1, y+h-1, selected?gui_col_select_border:gui_col_border);
		break;
	case WB_NORMAL:
	case WB_RECESSED:
		// to do
		break;
	case WB_THICK:
		// inner border.
		x--; y--; w+=2; h+=2;
		hline(scrBuffer, x, y, x+w-1, gui_col_dg);
		hline(scrBuffer, x, y+h-1, x+w-1, gui_col_lg);
		vline(scrBuffer, x, y, y+h-1, gui_col_dg);
		vline(scrBuffer, x+w-1, y, y+h-1, gui_col_lg);

		// middle
		x--; y--; w+=2; h+=2;
		hline(scrBuffer, x, y, x+w-1, selected?gui_col_w:gui_col_g);
		hline(scrBuffer, x, y+h-1, x+w-1, selected?gui_col_w:gui_col_g);
		vline(scrBuffer, x, y, y+h-1, selected?gui_col_w:gui_col_g);
		vline(scrBuffer, x+w-1, y, y+h-1, selected?gui_col_w:gui_col_g);

		// middle
		x--; y--; w+=2; h+=2;
		hline(scrBuffer, x, y, x+w-1, selected?gui_col_w:gui_col_g);
		hline(scrBuffer, x, y+h-1, x+w-1, selected?gui_col_w:gui_col_g);
		vline(scrBuffer, x, y, y+h-1, selected?gui_col_w:gui_col_g);
		vline(scrBuffer, x+w-1, y, y+h-1, selected?gui_col_w:gui_col_g);

		// outer
		x--; y--; w+=2; h+=2;
		hline(scrBuffer, x, y, x+w-1, gui_col_lg);
		hline(scrBuffer, x, y+h-1, x+w-1, gui_col_dg);
		vline(scrBuffer, x, y, y+h-1, gui_col_lg);
		vline(scrBuffer, x+w-1, y, y+h-1, gui_col_dg);
		break;
	}
}

// The x, y, w, h parameters are the inside size of the frame.	The border
// will be drawn OUTSIDE of those coordinates.
void gui_box(BITMAP *scrBuffer, int inverse, int x, int y, int w, int h)
{
	int l = inverse?gui_col_dg:gui_col_lg;
	int d = inverse?gui_col_lg:gui_col_dg;

	// border
	hline(scrBuffer, x, y, x+w-1, inverse?gui_col_b:gui_col_lg);
	hline(scrBuffer, x, y+h-1, x+w-1, inverse?gui_col_lg:gui_col_b);
	vline(scrBuffer, x, y, y+h-1, inverse?gui_col_b:gui_col_lg);
	vline(scrBuffer, x+w-1, y, y+h-1, inverse?gui_col_lg:gui_col_b);

	// inner border
	x++; y++; w-=2; h-=2;
	hline(scrBuffer, x, y, x+w-1, l);
	hline(scrBuffer, x, y+h-1, x+w-1, d);
	vline(scrBuffer, x, y, y+h-1, l);
	vline(scrBuffer, x+w-1, y, y+h-1, d);

	// middle
	x++; y++; w-=2; h-=2;
	rectfill(scrBuffer, x, y, x+w-1, y+h-1, gui_col_g);
}

void gui_status_bar(BITMAP *scrBuffer, int x, int y, int w, int h, char *text, int percent)
{
	char s[128];
	int fill_w = w*percent/100;

	// width must be at least 20 pixels.
	w = MAX(w, 20);

	// height must be at least the fonts height + 3.
	h = MAX(h, text_height(GUI_FONT)+3);

	// Get the string we want to display.
	if (text == NULL)
	{
		strcpy(s, "");
	} else {
		if (strlen(text) > 127)
			strcpy(s, "");
		else
		if (text[0] == '%' && text[1] != '%')
			sprintf(s, "%3d%%", percent);
		else
			strcpy(s, text);
	}

	// Transparent text pixels.
	text_mode(-1);

	// draw a thick frame around bar.
	gui_frame(scrBuffer, WB_THICK, 0, x, y, w, h);

	// fill it in.
	rectfill(scrBuffer, x, y, x+fill_w-1, y+h-1, gui_col_fill);
	rectfill(scrBuffer, x+fill_w-1, y, x+w-1, y+h-1, gui_col_g);

	// write percentage in bar.
	x = x+(w>>1);
	y = y+(h>>1)-(text_height(GUI_FONT)>>1);
	textout_centre(scrBuffer, GUI_FONT, s, x+1, y+1, gui_col_b);	// shadow
	textout_centre(scrBuffer, GUI_FONT, s, x, y, gui_col_w);		// actual
}

int gui_sendmsg(display_item *pDI, unsigned long msg, unsigned short _sh, unsigned long _ln)
{
	WINDOW *win;

	if (pDI == NULL)
		return 0;

	win = pDI->pContents;
	if (win == NULL)
		return 0;

	return win->msghandler(pDI, msg, _sh, _ln);
}

// default window/control message handling function
int gui_defmsghandler(display_item *pDI, unsigned long msg, unsigned short _sh, unsigned long _ln)
{
	WINDOW *win;

	if (pDI == NULL)
		return 0;

	win = (WINDOW*) pDI->pContents;
	if (win == NULL)
		return 0;

	switch (msg)
	{
	// main messages
	case WMSG_NONE:
		break;
	case WMSG_IDLE:
		break;

	// focus messages
	case WMSG_GOTFOCUS:
		break;
	case WMSG_LOSTFOCUS:
		break;

	// mouse messages
	case WMSG_MOUSEOVER:
		if (win->on_mouseover != NULL)
			win->on_mouseover(pDI, _sh, _ln);
		break;
	case WMSG_MOUSELDOWN:
		if (win->on_mouseldown != NULL)
			win->on_mouseldown(pDI, _sh, _ln);
		break;
	case WMSG_MOUSELUP:
		if (win->on_mouselup != NULL)
			win->on_mouselup(pDI, _sh, _ln);
		break;
	case WMSG_MOUSELCLICK:
		if (win->on_mouselclick != NULL)
			win->on_mouselclick(pDI, _sh, _ln);
		break;
	case WMSG_MOUSEMDOWN:
		if (win->on_mousemdown != NULL)
			win->on_mousemdown(pDI, _sh, _ln);
		break;
	case WMSG_MOUSEMUP:
		if (win->on_mousemup != NULL)
			win->on_mousemup(pDI, _sh, _ln);
		break;
	case WMSG_MOUSEMCLICK:
		if (win->on_mousemclick != NULL)
			win->on_mousemclick(pDI, _sh, _ln);
		break;
	case WMSG_MOUSERDOWN:
		if (win->on_mouserdown != NULL)
			win->on_mouserdown(pDI, _sh, _ln);
		break;
	case WMSG_MOUSERUP:
		if (win->on_mouserup != NULL)
			win->on_mouserup(pDI, _sh, _ln);
		break;
	case WMSG_MOUSERCLICK:
		if (win->on_mouserclick != NULL)
			win->on_mouserclick(pDI, _sh, _ln);
		break;

	default:
		break;
	}

	return 0;
}
