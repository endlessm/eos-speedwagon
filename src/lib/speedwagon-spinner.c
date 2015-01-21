/*
 * Copyright (C) 2015 Endless Mobile
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 *
 * Written by:
 *     Jasper St. Pierre <jstpierre@mecheye.net>
 */

/* A hack workaround for the fact that we can't override
 * gtk_container_forall from gjs. */

#include "speedwagon-spinner.h"

struct _SpeedwagonSpinnerPrivate
{
  GSList *internal_widgets;
};
typedef struct _SpeedwagonSpinnerPrivate SpeedwagonSpinnerPrivate;

G_DEFINE_TYPE_WITH_PRIVATE (SpeedwagonSpinner, speedwagon_spinner, GTK_TYPE_BOX);

static void
speedwagon_spinner_forall (GtkContainer *container,
                           gboolean      include_internals,
                           GtkCallback   callback,
                           gpointer      callback_data)
{
  SpeedwagonSpinner *spinner = SPEEDWAGON_SPINNER (container);
  SpeedwagonSpinnerPrivate *priv = speedwagon_spinner_get_instance_private (spinner);

  GTK_CONTAINER_CLASS (speedwagon_spinner_parent_class)->forall (container, include_internals, callback, callback_data);

  if (include_internals)
    g_slist_foreach (priv->internal_widgets, (GFunc) callback, callback_data);
}

static void
speedwagon_spinner_dispose (GObject *object)
{
  SpeedwagonSpinner *spinner = SPEEDWAGON_SPINNER (object);
  SpeedwagonSpinnerPrivate *priv = speedwagon_spinner_get_instance_private (spinner);

  g_slist_free_full (priv->internal_widgets, (GDestroyNotify) g_object_unref);
}

static void
speedwagon_spinner_class_init (SpeedwagonSpinnerClass *klass)
{
  GtkContainerClass *container_class = GTK_CONTAINER_CLASS (klass);
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  container_class->forall = speedwagon_spinner_forall;
  object_class->dispose = speedwagon_spinner_dispose;
}

static void
speedwagon_spinner_init (SpeedwagonSpinner *spinner)
{
}

/**
 * speedwagon_spinner_set_internal_widgets:
 * @spinner:
 * @internal_widgets: (element-type Gtk.Widget) (transfer container):
 */
void
speedwagon_spinner_set_internal_widgets (SpeedwagonSpinner *spinner,
                                         GSList *internal_widgets)
{
  SpeedwagonSpinnerPrivate *priv = speedwagon_spinner_get_instance_private (spinner);

  priv->internal_widgets = internal_widgets;
  g_slist_foreach (priv->internal_widgets, (GFunc) g_object_ref_sink, NULL);
}
