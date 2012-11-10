/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * Caja
 *
 * Copyright (C) 2008 Red Hat, Inc.
 *
 * Caja is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * Author: David Zeuthen <davidz@redhat.com>
 */

#include <config.h>
#include <string.h>
#include <glib/gi18n.h>
#include <gio/gio.h>
#include <gtk/gtk.h>

#include "caja-autorun.h"


typedef struct
{
    CajaAutorunGetContent callback;
    gpointer user_data;
} GetContentTypesData;

static void
get_types_cb (GObject *source_object,
              GAsyncResult *res,
              gpointer user_data)
{
    GetContentTypesData *data;
    char **types;

    data = user_data;
    types = g_mount_guess_content_type_finish (G_MOUNT (source_object), res, NULL);

    g_object_set_data_full (source_object,
                            "caja-content-type-cache",
                            g_strdupv (types),
                            (GDestroyNotify)g_strfreev);

    if (data->callback)
    {
        data->callback (types, data->user_data);
    }
    g_strfreev (types);
    g_free (data);
}

void
caja_autorun_get_x_content_types_for_mount_async (GMount *mount,
        CajaAutorunGetContent callback,
        GCancellable *cancellable,
        gpointer user_data)
{
    char **cached;
    GetContentTypesData *data;

    if (mount == NULL)
    {
        if (callback)
        {
            callback (NULL, user_data);
        }
        return;
    }

    cached = g_object_get_data (G_OBJECT (mount), "caja-content-type-cache");
    if (cached != NULL)
    {
        if (callback)
        {
            callback (cached, user_data);
        }
        return;
    }

    data = g_new (GetContentTypesData, 1);
    data->callback = callback;
    data->user_data = user_data;

    g_mount_guess_content_type (mount,
                                FALSE,
                                cancellable,
                                get_types_cb,
                                data);
}


char **
caja_autorun_get_cached_x_content_types_for_mount (GMount      *mount)
{
    char **cached;

    if (mount == NULL)
    {
        return NULL;
    }

    cached = g_object_get_data (G_OBJECT (mount), "caja-content-type-cache");
    if (cached != NULL)
    {
        return g_strdupv (cached);
    }

    return NULL;
}
