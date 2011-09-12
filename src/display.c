/* display.c
 *
 * Display list routines.  It's important that only one display list is in
 * use at a time, unless the parameters passed to create_display_list() do
 * not change.
 *
 * Copyright 1997, by Scott Deming.
 * All Rights Reserved!
 *
 * You can use this code any way you wish in any form you wish.  I provide
 * this as a tutorial on the technique and nothing more.
 */

#include <mem.h>
#include <stdlib.h>

#include <alex.h>

display_list *create_display_list(COLOR_MAP *light_table, COLOR_MAP *trans_table)
{
	display_list *pDL = malloc(sizeof(display_list));

	if (pDL == NULL)
		return NULL;

	pDL->pItem = NULL;

	pDL->display_prev_table = NULL;
	pDL->display_light_table = light_table;
	pDL->display_trans_table = trans_table;

	return pDL;
}

void draw_display_bitmap(display_item *pDI, int undo, BITMAP *pScreen, RECT *pRect)
{
	BITMAP *pContents = (BITMAP*) pDI->pContents;
	BITMAP *pOldContents = pDI->pOldContents;
	BITMAP *p = (BITMAP*) (undo?pOldContents:pContents);
	RECT r = {pDI->x, pDI->y, undo?p->w:pDI->is_scaled?pDI->w:p->w, undo?p->h:pDI->is_scaled?pDI->h:p->h};
	RECT tmp_r;

	// dirty rectangle.
	if (pRect)
	{
		rect_copy(&tmp_r, pRect);

		if (pRect->x == -1)
		{
			rect_copy(pRect, &r);
		} else {
			rect_combine(pRect, &tmp_r, &r);
		}
	}

	if (undo)
	{
		// draw it
		if (pDI->is_saving)
			blit(pOldContents, pScreen, 0, 0, pDI->x, pDI->y, pOldContents->w, pOldContents->h);
	} else {
		// draw it.
		if (pDI->is_scaled)
		{
			if (pDI->is_saving)
				blit(pScreen, pOldContents, pDI->x, pDI->y, 0, 0, pDI->w, pDI->h);

			if (pDI->is_sprite)
			{
				stretch_sprite((BITMAP*) pContents, pScreen, pDI->x, pDI->y, pDI->w, pDI->h);
			} else {
				stretch_blit((BITMAP*) pContents, pScreen, 0, 0, pContents->w, pContents->h, pDI->x, pDI->y, pDI->w, pDI->h);
			}
		} else {
			if (pDI->is_saving)
				blit(pScreen, pOldContents, pDI->x, pDI->y, 0, 0, pContents->w, pContents->h);

			if (pDI->is_sprite)
			{
				draw_sprite(pScreen, pContents, pDI->x, pDI->y);
			} else
			if (pDI->is_translucent)
			{
				pDI->pParent->display_prev_table = color_map;
				color_map = pDI->type==DISPLAY_LIGHT?pDI->pParent->display_light_table:pDI->pParent->display_trans_table;
				draw_trans_sprite(pScreen, pContents, pDI->x, pDI->y);
				color_map = pDI->pParent->display_prev_table;
			} else {
				blit(pContents, pScreen, 0, 0, pDI->x, pDI->y, pContents->w, pContents->h);
			}
		}
	}
}

void draw_display_window(display_item *pDI, int undo, BITMAP *pScreen, RECT *pRect)
{
	WINDOW *pContents = (WINDOW*) pDI->pContents;
	BITMAP *pOldContents = pDI->pOldContents;
	RECT tmp_r;

	// dirty rectangle.
	if (pRect)
	{
		rect_copy(&tmp_r, pRect);

		if (pRect->x == -1)
		{
			rect_copy(pRect, &pContents->window);
		} else {
			rect_combine(pRect, &tmp_r, &pContents->window);
		}
	}

	if (undo)
	{
		// draw it
		blit(pOldContents, pScreen, 0, 0, pContents->window.x, pContents->window.y, pContents->window.w, pContents->window.h);
	} else {
		// save it
		if (pDI->is_saving)
		{
			blit(pScreen, pOldContents, pContents->window.x, pContents->window.y, 0, 0, pContents->window.w, pContents->window.h);
		}

		// draw it
		gui_draw_window(pContents, pScreen);
	}
}

void render_display_list(display_list *pDL, BITMAP *pScreen, RECT *pRect)
{
	display_item *pItem, *p;

	if (pRect)
		rect_dirty_max(pRect);

	p = pDL->pItem;
	while (p != NULL)
	{
		pItem = p;

		// render item.
		if (!pItem || !pItem->pContents)
			continue;

		switch (pItem->type)
		{
			case DISPLAY_BITMAP:
			case DISPLAY_LIGHT:
				draw_display_bitmap(pItem, 0, pScreen, pRect);
				break;
			case DISPLAY_WINDOW:
				draw_display_window(pItem, 0, pScreen, pRect);
				break;
		}

		// "Next please?"
		p = p->pNext;
	}
}

void undo_render_display_list(display_list *pDL, BITMAP *pScreen, RECT *pRect)
{
	display_item *pItem, *p;

	if (pRect)
		rect_dirty_max(pRect);

	p = pDL->pItem;

	while (p->pNext != NULL)
		p = p->pNext;

	while (p != NULL)
	{
		pItem = p;

		// render item.
		if (!pItem || !pItem->pOldContents)
			continue;

		switch (pItem->type)
		{
			case DISPLAY_BITMAP:
			case DISPLAY_LIGHT:
				draw_display_bitmap(pItem, 1, pScreen, pRect);
				break;
			case DISPLAY_WINDOW:
				draw_display_window(pItem, 1, pScreen, pRect);
				break;
		}

		// "Previous please?"
		p = p->pPrev;
	}
}

display_item *create_display_item(void *pContents, int type, int x, int y, int is_translucent, int is_sprite, int is_saving)
{
	display_item *pItem = malloc(sizeof(display_item));

	if (pItem == NULL)
		return NULL;

	// set pointers
	pItem->pParent = NULL;
	pItem->pNext = NULL;
	pItem->pPrev = NULL;

	pItem->pContents = pContents;
	pItem->pOldContents = NULL; 	  // default

	// set defaults
	pItem->w = 0;
	pItem->h = 0;

	// set location.
	move_display_item(pItem, x, y);

	// set parameters
	pItem->type = type;
	pItem->is_scaled = 0;
	pItem->is_translucent = is_translucent;
	pItem->is_sprite = is_sprite;
	pItem->is_saving = is_saving;

	// is_saving modifiers
	if (pItem->is_saving)
	{
		switch (pItem->type)
		{
			case DISPLAY_BITMAP:
			case DISPLAY_LIGHT:
				pItem->pOldContents = create_bitmap(pItem->is_scaled?pItem->w:((BITMAP*) pContents)->w, pItem->is_scaled?pItem->h:((BITMAP*) pContents)->h);
				break;
			case DISPLAY_WINDOW:
				pItem->pOldContents = create_bitmap(((WINDOW*) pContents)->window.w, ((WINDOW*) pContents)->window.h);
				break;
		}
	}

	return pItem;
}

display_item *add_display_item(display_list *pDL, display_item *pItem)
{
	display_item *p = pDL->pItem;

	if (pDL == NULL || pItem == NULL)
		return pItem;

	pItem->pPrev = NULL;
	pItem->pNext = NULL;
	pItem->pParent = pDL;

	if (p == NULL)
	{
		pDL->pItem = pItem;
		return pItem;
	}

	while (p->pNext != NULL)
		p = p->pNext;

	p->pNext = pItem;
	pItem->pPrev = p;

	return pItem;
}

display_item *insert_display_item(display_list *pDL, display_item *pItem)
{
	if (pDL == NULL || pItem == NULL)
		return pItem;

	pItem->pNext = pDL->pItem;
	pItem->pParent = pDL;
	pDL->pItem = pItem;
	if (pItem->pNext != NULL)
		pItem->pPrev = pItem;

	return pItem;
}

display_item *remove_display_item(display_item *pDI)
{
	display_list *pDL;
	display_item *p;

	if (pDI == NULL || pDI->pParent == NULL)
		return pDI;

	p = pDI;
	pDL = pDI->pParent;

	if (pDI->pPrev == NULL && pDI->pNext == NULL)
	{
		pDL->pItem = NULL;
		return pDI;
	}

	if (p->pPrev == NULL)
	{
		// first in list
		pDL->pItem = p->pNext;
		pDL->pItem->pPrev = NULL;
	} else
	if (p->pNext == NULL)
	{
		// last in list
		p = p->pPrev;
		p->pNext = NULL;
	} else {
		// somewhere inbetween
		p->pPrev->pNext = p->pNext;
		p->pNext->pPrev = p->pPrev;
	}

	return pDI;
}

int destroy_display_list(display_list *pDL)
{
	display_item *p, *pNext;

	if (pDL == NULL)
		return 0;

	p = pDL->pItem;

	while (p != NULL)
	{
		pNext = p->pNext;
		destroy_display_item(p);
		p = pNext;
	}

	free(pDL);

	return 0;
}

int destroy_display_item(display_item *pItem)
{
	if (pItem == NULL)
		return 0;

	if (pItem->pOldContents != NULL)
		destroy_bitmap(pItem->pOldContents);

	free(pItem);

	return (0);
}

void scale_display_item(display_item *pItem, int w, int h)
{
	pItem->w = w;
	pItem->h = h;
	pItem->is_scaled = 1;
}

void move_display_item(display_item *pDI, int x, int y)
{
	if (pDI == NULL)
		return;

	pDI->x = x;
	pDI->y = y;

	// type modifiers
	switch (pDI->type)
	{
		case DISPLAY_WINDOW:
			if (pDI->x == -1 && pDI->y == -1)
			{
				pDI->x = ((WINDOW*) pDI->pContents)->window.x;
				pDI->y = ((WINDOW*) pDI->pContents)->window.y;
			} else {
				gui_move_window((WINDOW*) pDI->pContents, pDI->x, pDI->y);
			}
			break;
	}
}

int display_hittest(display_item *pDI, int x, int y)
{
	BITMAP *p;
	int w, h;

	if (pDI->type == DISPLAY_WINDOW)
	{
		return (gui_hittest((WINDOW*) pDI->pContents, x, y));
	}

	p = (BITMAP*) pDI->pContents;
	w = pDI->is_scaled?pDI->w:p->w;
	h = pDI->is_scaled?pDI->h:p->h;

	if ((x > pDI->x) && (x < pDI->x+w-1) &&
		(y > pDI->y) && (y < pDI->y+h-1))
	{
		return 1;
	}

	return 0;
}

display_item *get_bottom_window(display_list *pDL, int x, int y)
{
	display_item *pDI;

	if (pDL == NULL || pDL->pItem == NULL)
		return NULL;

	pDI = pDL->pItem;

	while (pDI != NULL)
	{
		if (pDI->type == DISPLAY_WINDOW)
		{
			if (display_hittest(pDI, x, y))
			{
				return pDI;
			}
		}

		pDI = pDI->pNext;
	}

	return NULL;
}

display_item *get_top_window(display_list *pDL, int x, int y)
{
	display_item *pDI;

	if (pDL == NULL || pDL->pItem == NULL)
		return NULL;

	pDI = pDL->pItem;
	while (pDI->pNext != NULL)
		pDI = pDI->pNext;

	while (pDI != NULL)
	{
		if (pDI->type == DISPLAY_WINDOW)
		{
			if (display_hittest(pDI, x, y))
			{
				return pDI;
			}
		}

		pDI = pDI->pPrev;
	}

	return NULL;
}

void bring_window_to_top(display_item *pDI)
{
	if (pDI == NULL || pDI->pParent == NULL || pDI->type != DISPLAY_WINDOW)
		return;

	remove_display_item(pDI);
	add_display_item(pDI->pParent, pDI);
}

void send_window_to_bottom(display_item *pDI)
{
	if (pDI == NULL || pDI->pParent == NULL || pDI->type != DISPLAY_WINDOW)
		return;

	remove_display_item(pDI);
	insert_display_item(pDI->pParent, pDI);
}

void set_all_windows_styles(display_list *pDL, int border_style, int draw_style)
{
	display_item *pDI;

	if (pDL == NULL || pDL->pItem == NULL)
		return;

	pDI = pDL->pItem;
	while (pDI != NULL)
	{
		if (pDI->type == DISPLAY_WINDOW)
		{
			gui_set_window_styles((WINDOW*) pDI->pContents, border_style, draw_style);
		}

		pDI = pDI->pNext;
	}
}

