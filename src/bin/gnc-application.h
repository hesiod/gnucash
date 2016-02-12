/*
 * gnc-application.h -- The program entry point for GnuCash
 *
 * Copyright (C) 2006 Chris Shoemaker <c.shoemaker@cox.net>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, contact:
 *
 * Free Software Foundation           Voice:  +1-617-542-5942
 * 51 Franklin Street, Fifth Floor    Fax:    +1-617-542-2652
 * Boston, MA  02110-1301,  USA       gnu@gnu.org
 */

#ifndef __GNC_APPLICATION_H
#define __GNC_APPLICATION_H

#include <gtk/gtk.h>
#include <glib-object.h>
//#include <glib/gi18n.h>

G_BEGIN_DECLS

#define GNC_TYPE_APPLICATION gnc_application_get_type ()
G_DECLARE_FINAL_TYPE (GncApplication, gnc_application, GNC, APPLICATION, GtkApplication)

struct _GncApplicationClass
{
  GtkApplicationClass parent_class;
};

static void gnc_application_init (GncApplication *app);
static void gnc_application_class_init (GncApplicationClass *class);

static void gnc_application_activate (GApplication *app);

static void
gnc_application_open (GApplication  *app,
                  GFile        **files,
                  gint           n_files,
                  const gchar   *hint);


GncApplication * gnc_application_new (void);


G_END_DECLS

#endif
