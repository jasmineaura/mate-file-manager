/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*-

   caja-cell-renderer-pixbuf-emblem.c: cell renderer which can render
   an emblem on top of a pixbuf (for use in FMListView and FMTreeView)

   Copyright (C) 2003 Juerg Billeter

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public
   License along with this program; if not, write to the
   Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
   Boston, MA 02110-1301, USA.

   This is based on GtkCellRendererPixbuf written by
   Jonathan Blandford <jrb@redhat.com>

   Author: Juerg Billeter <j@bitron.ch>
*/

#include "caja-cell-renderer-pixbuf-emblem.h"

static void caja_cell_renderer_pixbuf_emblem_get_property (GObject *object,
        guint		param_id,
        GValue		*value,
        GParamSpec	*pspec);
static void caja_cell_renderer_pixbuf_emblem_set_property (GObject *object,
        guint		param_id,
        const GValue	*value,
        GParamSpec	*pspec);
static void caja_cell_renderer_pixbuf_emblem_init       (CajaCellRendererPixbufEmblem *cellpixbuf);
static void caja_cell_renderer_pixbuf_emblem_class_init  (CajaCellRendererPixbufEmblemClass *klass);
static void caja_cell_renderer_pixbuf_emblem_finalize (GObject *object);
static void caja_cell_renderer_pixbuf_emblem_create_stock_pixbuf (CajaCellRendererPixbufEmblem *cellpixbuf,
        GtkWidget             *widget);
static void caja_cell_renderer_pixbuf_emblem_get_size   (GtkCellRenderer            *cell,
        GtkWidget                  *widget,
        GdkRectangle               *rectangle,
        gint                       *x_offset,
        gint                       *y_offset,
        gint                       *width,
        gint                       *height);
static void     caja_cell_renderer_pixbuf_emblem_render     (GtkCellRenderer            *cell,
        GdkWindow                  *window,
        GtkWidget                  *widget,
        GdkRectangle               *background_area,
        GdkRectangle               *cell_area,
        GdkRectangle               *expose_area,
        GtkCellRendererState                       flags);

enum
{
    PROP_ZERO,
    PROP_PIXBUF,
    PROP_PIXBUF_EXPANDER_OPEN,
    PROP_PIXBUF_EXPANDER_CLOSED,
    PROP_STOCK_ID,
    PROP_STOCK_SIZE,
    PROP_STOCK_DETAIL,
    PROP_PIXBUF_EMBLEM
};

#define CELLINFO_KEY "caja-cell-renderer-pixbuf-emblem-info"

typedef struct _CajaCellRendererPixbufEmblemInfo CajaCellRendererPixbufEmblemInfo;
struct _CajaCellRendererPixbufEmblemInfo
{
    gchar *stock_id;
    GtkIconSize stock_size;
    gchar *stock_detail;
};

G_DEFINE_TYPE (CajaCellRendererPixbufEmblem, caja_cell_renderer_pixbuf_emblem, GTK_TYPE_CELL_RENDERER);

static void
caja_cell_renderer_pixbuf_emblem_init (CajaCellRendererPixbufEmblem *cellpixbuf)
{
    CajaCellRendererPixbufEmblemInfo *cellinfo;

    cellinfo = g_new0 (CajaCellRendererPixbufEmblemInfo, 1);
    cellinfo->stock_size = GTK_ICON_SIZE_MENU;
    g_object_set_data (G_OBJECT (cellpixbuf), CELLINFO_KEY, cellinfo);
}

static void
caja_cell_renderer_pixbuf_emblem_class_init (CajaCellRendererPixbufEmblemClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);
    GtkCellRendererClass *cell_class = GTK_CELL_RENDERER_CLASS (klass);

    object_class->finalize = caja_cell_renderer_pixbuf_emblem_finalize;

    object_class->get_property = caja_cell_renderer_pixbuf_emblem_get_property;
    object_class->set_property = caja_cell_renderer_pixbuf_emblem_set_property;

    cell_class->get_size = caja_cell_renderer_pixbuf_emblem_get_size;
    cell_class->render = caja_cell_renderer_pixbuf_emblem_render;

    g_object_class_install_property (object_class,
                                     PROP_PIXBUF,
                                     g_param_spec_object ("pixbuf",
                                             "Pixbuf Object",
                                             "The pixbuf to render",
                                             GDK_TYPE_PIXBUF,
                                             G_PARAM_READABLE |
                                             G_PARAM_WRITABLE));

    g_object_class_install_property (object_class,
                                     PROP_PIXBUF_EXPANDER_OPEN,
                                     g_param_spec_object ("pixbuf_expander_open",
                                             "Pixbuf Expander Open",
                                             "Pixbuf for open expander",
                                             GDK_TYPE_PIXBUF,
                                             G_PARAM_READABLE |
                                             G_PARAM_WRITABLE));

    g_object_class_install_property (object_class,
                                     PROP_PIXBUF_EXPANDER_CLOSED,
                                     g_param_spec_object ("pixbuf_expander_closed",
                                             "Pixbuf Expander Closed",
                                             "Pixbuf for closed expander",
                                             GDK_TYPE_PIXBUF,
                                             G_PARAM_READABLE |
                                             G_PARAM_WRITABLE));

    g_object_class_install_property (object_class,
                                     PROP_STOCK_ID,
                                     g_param_spec_string ("stock_id",
                                             "Stock ID",
                                             "The stock ID of the stock icon to render",
                                             NULL,
                                             G_PARAM_READWRITE));

    g_object_class_install_property (object_class,
                                     PROP_STOCK_SIZE,
                                     g_param_spec_enum ("stock_size",
                                             "Size",
                                             "The size of the rendered icon",
                                             GTK_TYPE_ICON_SIZE,
                                             GTK_ICON_SIZE_MENU,
                                             G_PARAM_READWRITE));

    g_object_class_install_property (object_class,
                                     PROP_STOCK_DETAIL,
                                     g_param_spec_string ("stock_detail",
                                             "Detail",
                                             "Render detail to pass to the theme engine",
                                             NULL,
                                             G_PARAM_READWRITE));

    g_object_class_install_property (object_class,
                                     PROP_PIXBUF_EMBLEM,
                                     g_param_spec_object ("pixbuf_emblem",
                                             "Pixbuf Emblem Object",
                                             "The emblem to overlay",
                                             GDK_TYPE_PIXBUF,
                                             G_PARAM_READABLE |
                                             G_PARAM_WRITABLE));
}

static void
caja_cell_renderer_pixbuf_emblem_finalize (GObject *object)
{
    CajaCellRendererPixbufEmblem *cellpixbuf = CAJA_CELL_RENDERER_PIXBUF_EMBLEM (object);
    CajaCellRendererPixbufEmblemInfo *cellinfo = g_object_get_data (object, CELLINFO_KEY);

    if (cellpixbuf->pixbuf && cellinfo->stock_id)
    {
        g_object_unref (cellpixbuf->pixbuf);
    }

    if (cellinfo->stock_id)
    {
        g_free (cellinfo->stock_id);
    }

    if (cellinfo->stock_detail)
    {
        g_free (cellinfo->stock_detail);
    }

    g_free (cellinfo);
    g_object_set_data (object, CELLINFO_KEY, NULL);

    (* G_OBJECT_CLASS (caja_cell_renderer_pixbuf_emblem_parent_class)->finalize) (object);
}

static void
caja_cell_renderer_pixbuf_emblem_get_property (GObject        *object,
        guint           param_id,
        GValue         *value,
        GParamSpec     *pspec)
{
    CajaCellRendererPixbufEmblem *cellpixbuf = CAJA_CELL_RENDERER_PIXBUF_EMBLEM (object);
    CajaCellRendererPixbufEmblemInfo *cellinfo = g_object_get_data (object, CELLINFO_KEY);

    switch (param_id)
    {
    case PROP_PIXBUF:
        g_value_set_object (value,
                            cellpixbuf->pixbuf ? G_OBJECT (cellpixbuf->pixbuf) : NULL);
        break;
    case PROP_PIXBUF_EXPANDER_OPEN:
        g_value_set_object (value,
                            cellpixbuf->pixbuf_expander_open ? G_OBJECT (cellpixbuf->pixbuf_expander_open) : NULL);
        break;
    case PROP_PIXBUF_EXPANDER_CLOSED:
        g_value_set_object (value,
                            cellpixbuf->pixbuf_expander_closed ? G_OBJECT (cellpixbuf->pixbuf_expander_closed) : NULL);
        break;
    case PROP_STOCK_ID:
        g_value_set_string (value, cellinfo->stock_id);
        break;
    case PROP_STOCK_SIZE:
        g_value_set_enum (value, cellinfo->stock_size);
        break;
    case PROP_STOCK_DETAIL:
        g_value_set_string (value, cellinfo->stock_detail);
        break;
    case PROP_PIXBUF_EMBLEM:
        g_value_set_object (value,
                            cellpixbuf->pixbuf_emblem ? G_OBJECT (cellpixbuf->pixbuf_emblem) : NULL);
        break;

    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, param_id, pspec);
        break;
    }
}

static void
caja_cell_renderer_pixbuf_emblem_set_property (GObject      *object,
        guint         param_id,
        const GValue *value,
        GParamSpec   *pspec)
{
    GdkPixbuf *pixbuf;
    CajaCellRendererPixbufEmblem *cellpixbuf = CAJA_CELL_RENDERER_PIXBUF_EMBLEM (object);
    CajaCellRendererPixbufEmblemInfo *cellinfo = g_object_get_data (object, CELLINFO_KEY);

    switch (param_id)
    {
    case PROP_PIXBUF:
        pixbuf = (GdkPixbuf*) g_value_get_object (value);
        if (pixbuf)
        {
            g_object_ref (pixbuf);
        }
        if (cellpixbuf->pixbuf)
        {
            g_object_unref (cellpixbuf->pixbuf);
        }
        cellpixbuf->pixbuf = pixbuf;
        break;
    case PROP_PIXBUF_EXPANDER_OPEN:
        pixbuf = (GdkPixbuf*) g_value_get_object (value);
        if (pixbuf)
        {
            g_object_ref (pixbuf);
        }
        if (cellpixbuf->pixbuf_expander_open)
        {
            g_object_unref (cellpixbuf->pixbuf_expander_open);
        }
        cellpixbuf->pixbuf_expander_open = pixbuf;
        break;
    case PROP_PIXBUF_EXPANDER_CLOSED:
        pixbuf = (GdkPixbuf*) g_value_get_object (value);
        if (pixbuf)
        {
            g_object_ref (pixbuf);
        }
        if (cellpixbuf->pixbuf_expander_closed)
        {
            g_object_unref (cellpixbuf->pixbuf_expander_closed);
        }
        cellpixbuf->pixbuf_expander_closed = pixbuf;
        break;
    case PROP_STOCK_ID:
        if (cellinfo->stock_id)
        {
            g_free (cellinfo->stock_id);
        }
        cellinfo->stock_id = g_strdup (g_value_get_string (value));
        break;
    case PROP_STOCK_SIZE:
        cellinfo->stock_size = g_value_get_enum (value);
        break;
    case PROP_STOCK_DETAIL:
        if (cellinfo->stock_detail)
        {
            g_free (cellinfo->stock_detail);
        }
        cellinfo->stock_detail = g_strdup (g_value_get_string (value));
        break;
    case PROP_PIXBUF_EMBLEM:
        pixbuf = (GdkPixbuf *) g_value_get_object (value);
        if (pixbuf)
        {
            g_object_ref (pixbuf);
        }
        if (cellpixbuf->pixbuf_emblem)
        {
            g_object_unref (cellpixbuf->pixbuf_emblem);
        }
        cellpixbuf->pixbuf_emblem = pixbuf;
        break;

    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, param_id, pspec);
        break;
    }
}

GtkCellRenderer *
caja_cell_renderer_pixbuf_emblem_new (void)
{
    return g_object_new (CAJA_TYPE_CELL_RENDERER_PIXBUF_EMBLEM, NULL);
}

static void
caja_cell_renderer_pixbuf_emblem_create_stock_pixbuf (CajaCellRendererPixbufEmblem *cellpixbuf,
        GtkWidget             *widget)
{
    CajaCellRendererPixbufEmblemInfo *cellinfo = g_object_get_data (G_OBJECT (cellpixbuf), CELLINFO_KEY);

    if (cellpixbuf->pixbuf)
    {
        g_object_unref (cellpixbuf->pixbuf);
    }

    cellpixbuf->pixbuf = gtk_widget_render_icon (widget,
                         cellinfo->stock_id,
                         cellinfo->stock_size,
                         cellinfo->stock_detail);
}

static void
caja_cell_renderer_pixbuf_emblem_get_size (GtkCellRenderer *cell,
        GtkWidget       *widget,
        GdkRectangle    *cell_area,
        gint            *x_offset,
        gint            *y_offset,
        gint            *width,
        gint            *height)
{
    CajaCellRendererPixbufEmblem *cellpixbuf = (CajaCellRendererPixbufEmblem *) cell;
    CajaCellRendererPixbufEmblemInfo *cellinfo = g_object_get_data (G_OBJECT (cell), CELLINFO_KEY);
    gint pixbuf_width  = 0;
    gint pixbuf_height = 0;
    gint calc_width;
    gint calc_height;
    gint xpad, ypad;

    if (!cellpixbuf->pixbuf && cellinfo->stock_id)
        caja_cell_renderer_pixbuf_emblem_create_stock_pixbuf (cellpixbuf, widget);

    if (cellpixbuf->pixbuf)
    {
        pixbuf_width  = gdk_pixbuf_get_width (cellpixbuf->pixbuf);
        pixbuf_height = gdk_pixbuf_get_height (cellpixbuf->pixbuf);
    }
    if (cellpixbuf->pixbuf_expander_open)
    {
        pixbuf_width  = MAX (pixbuf_width, gdk_pixbuf_get_width (cellpixbuf->pixbuf_expander_open));
        pixbuf_height = MAX (pixbuf_height, gdk_pixbuf_get_height (cellpixbuf->pixbuf_expander_open));
    }
    if (cellpixbuf->pixbuf_expander_closed)
    {
        pixbuf_width  = MAX (pixbuf_width, gdk_pixbuf_get_width (cellpixbuf->pixbuf_expander_closed));
        pixbuf_height = MAX (pixbuf_height, gdk_pixbuf_get_height (cellpixbuf->pixbuf_expander_closed));
    }

    gtk_cell_renderer_get_padding (cell, &xpad, &ypad);
    calc_width  = xpad * 2 + pixbuf_width;
    calc_height = ypad * 2 + pixbuf_height;

    if (x_offset) *x_offset = 0;
    if (y_offset) *y_offset = 0;

    if (cell_area && pixbuf_width > 0 && pixbuf_height > 0)
    {
        gfloat xalign, yalign;

        gtk_cell_renderer_get_alignment (cell, &xalign, &yalign);
        if (x_offset)
        {
            *x_offset = (((gtk_widget_get_direction (widget) == GTK_TEXT_DIR_RTL) ?
                          1.0 - xalign : xalign) *
                         (cell_area->width - calc_width - 2 * xpad));
            *x_offset = MAX (*x_offset, 0) + xpad;
        }
        if (y_offset)
        {
            *y_offset = (yalign *
                         (cell_area->height - calc_height - 2 * ypad));
            *y_offset = MAX (*y_offset, 0) + ypad;
        }
    }

    if (width)
        *width = calc_width;

    if (height)
        *height = calc_height;
}

static void
caja_cell_renderer_pixbuf_emblem_render (GtkCellRenderer      *cell,
        GdkWindow            *window,
        GtkWidget            *widget,
        GdkRectangle         *background_area,
        GdkRectangle         *cell_area,
        GdkRectangle         *expose_area,
        GtkCellRendererState  flags)

{
    CajaCellRendererPixbufEmblem *cellpixbuf = (CajaCellRendererPixbufEmblem *) cell;
    CajaCellRendererPixbufEmblemInfo *cellinfo = g_object_get_data (G_OBJECT (cell), CELLINFO_KEY);
    GdkPixbuf *pixbuf;
    GdkRectangle pix_rect;
    GdkRectangle pix_emblem_rect;
    gboolean stock_pixbuf = FALSE;
    gint xpad, ypad;
    gboolean is_expander, is_expanded;
    cairo_t *cr;

    pixbuf = cellpixbuf->pixbuf;
    g_object_get (cell,
                  "is-expander", &is_expander,
                  "is-expanded", &is_expanded,
                  NULL);
    if (is_expander)
    {
        if (is_expanded &&
                cellpixbuf->pixbuf_expander_open != NULL)
        {
            pixbuf = cellpixbuf->pixbuf_expander_open;
        }
        else if (!is_expanded &&
                 cellpixbuf->pixbuf_expander_closed != NULL)
        {
            pixbuf = cellpixbuf->pixbuf_expander_closed;
        }
    }

    if (!pixbuf && !cellinfo->stock_id)
    {
        return;
    }
    else if (!pixbuf && cellinfo->stock_id)
    {
        stock_pixbuf = TRUE;
    }

    caja_cell_renderer_pixbuf_emblem_get_size (cell, widget, cell_area,
            &pix_rect.x,
            &pix_rect.y,
            &pix_rect.width,
            &pix_rect.height);

    if (stock_pixbuf)
        pixbuf = cellpixbuf->pixbuf;

    gtk_cell_renderer_get_padding (cell, &xpad, &ypad);
    pix_rect.x += cell_area->x;
    pix_rect.y += cell_area->y;
    pix_rect.width  -= xpad * 2;
    pix_rect.height -= ypad * 2;

    cr = gdk_cairo_create (window);
    gdk_cairo_rectangle (cr, expose_area);
    cairo_clip (cr);
    gdk_cairo_rectangle (cr, cell_area);
    cairo_clip (cr);

    gdk_cairo_set_source_pixbuf (cr, pixbuf, pix_rect.x, pix_rect.y);
    gdk_cairo_rectangle (cr, &pix_rect);
    cairo_fill (cr);

    if (cellpixbuf->pixbuf_emblem)
    {
        pix_emblem_rect.width = gdk_pixbuf_get_width (cellpixbuf->pixbuf_emblem);
        pix_emblem_rect.height = gdk_pixbuf_get_height (cellpixbuf->pixbuf_emblem);
        pix_emblem_rect.x = pix_rect.x;
        pix_emblem_rect.y = pix_rect.y + pix_rect.height - pix_emblem_rect.height;
        gdk_cairo_set_source_pixbuf (cr, cellpixbuf->pixbuf_emblem,
        			     pix_emblem_rect.x, pix_emblem_rect.y);
        gdk_cairo_rectangle (cr, &pix_emblem_rect);
        cairo_fill (cr);
    }

    cairo_destory (cr);
}
