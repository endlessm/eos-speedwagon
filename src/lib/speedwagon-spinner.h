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

#pragma once

#include <gtk/gtk.h>

#define SPEEDWAGON_TYPE_SPINNER             (speedwagon_spinner_get_type ())
#define SPEEDWAGON_SPINNER(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), SPEEDWAGON_TYPE_SPINNER, SpeedwagonSpinner))
#define SPEEDWAGON_SPINNER_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass),  SPEEDWAGON_TYPE_SPINNER, SpeedwagonSpinnerClass))
#define SPEEDWAGON_IS_SPINNER(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), SPEEDWAGON_TYPE_SPINNER))
#define SPEEDWAGON_IS_SPINNER_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass),  SPEEDWAGON_TYPE_SPINNER))
#define SPEEDWAGON_SPINNER_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj),  SPEEDWAGON_TYPE_SPINNER, SpeedwagonSpinnerClass))

typedef struct _SpeedwagonSpinner        SpeedwagonSpinner;
typedef struct _SpeedwagonSpinnerClass   SpeedwagonSpinnerClass;

struct _SpeedwagonSpinner {
  GtkBox parent;
};

struct _SpeedwagonSpinnerClass {
  GtkBoxClass parent_class;
};

GType speedwagon_spinner_get_type (void) G_GNUC_CONST;

void speedwagon_spinner_set_internal_widgets (SpeedwagonSpinner *spinner,
                                              GSList *internal_widgets);
