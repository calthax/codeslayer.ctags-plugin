/*
 * Copyright (C) 2010 - Jeff Johnston
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include <codeslayer/codeslayer.h>
#include "ctags-menu.h"
#include "ctags-project-properties.h"
#include "ctags-engine.h"
#include <gtk/gtk.h>
#include <glib.h>
#include <gmodule.h>

G_MODULE_EXPORT void activate   (CodeSlayer *codeslayer);
G_MODULE_EXPORT void deactivate (CodeSlayer *codeslayer);

static GtkWidget *menu;
static GtkWidget *project_properties;
static CtagsEngine *engine;

G_MODULE_EXPORT
void activate (CodeSlayer *codeslayer)
{
  GtkAccelGroup *accel_group;
  accel_group = codeslayer_get_menubar_accel_group (codeslayer);
  menu = ctags_menu_new (accel_group);

  project_properties = ctags_project_properties_new ();
  engine = ctags_engine_new (codeslayer, menu, project_properties);
  ctags_engine_load_configurations (engine);

  codeslayer_add_to_menubar (codeslayer, GTK_MENU_ITEM (menu));
  codeslayer_add_to_project_properties (codeslayer, project_properties, "Ctags");
}

G_MODULE_EXPORT
void deactivate (CodeSlayer *codeslayer)
{
  codeslayer_remove_from_menubar (codeslayer, GTK_MENU_ITEM (menu));
  codeslayer_remove_from_project_properties (codeslayer, project_properties);
  g_object_unref (engine);
}
