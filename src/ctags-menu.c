/*
 * Copyright (C) 2010 - Jeff Johnston
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.remove_group_item
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include <gdk/gdkkeysyms.h>
#include <codeslayer/codeslayer.h>
#include "ctags-menu.h"

static void ctags_menu_class_init  (CtagsMenuClass *klass);
static void ctags_menu_init        (CtagsMenu      *menu);
static void ctags_menu_finalize    (CtagsMenu      *menu);

static void add_menu_items         (CtagsMenu      *menu,
                                    GtkWidget      *submenu,
                                    GtkAccelGroup  *accel_group);
static void find_tag_action        (CtagsMenu      *menu);
static void previous_action        (CtagsMenu      *menu);
static void next_action            (CtagsMenu      *menu);
                                        
enum
{
  FIND_TAG,
  PREVIOUS,
  NEXT,
  LAST_SIGNAL
};

static guint ctags_menu_signals[LAST_SIGNAL] = { 0 };

G_DEFINE_TYPE (CtagsMenu, ctags_menu, GTK_TYPE_MENU_ITEM)

static void
ctags_menu_class_init (CtagsMenuClass *klass)
{
  ctags_menu_signals[FIND_TAG] =
    g_signal_new ("find-tag", 
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
                  G_STRUCT_OFFSET (CtagsMenuClass, find_tag),
                  NULL, NULL, 
                  g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);

  ctags_menu_signals[PREVIOUS] =
    g_signal_new ("previous", 
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
                  G_STRUCT_OFFSET (CtagsMenuClass, previous),
                  NULL, NULL, 
                  g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);

  ctags_menu_signals[NEXT] =
    g_signal_new ("next", 
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
                  G_STRUCT_OFFSET (CtagsMenuClass, next),
                  NULL, NULL, 
                  g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);

  G_OBJECT_CLASS (klass)->finalize = (GObjectFinalizeFunc) ctags_menu_finalize;
}

static void
ctags_menu_init (CtagsMenu *menu)
{
  gtk_menu_item_set_label (GTK_MENU_ITEM (menu), _("Ctags"));
}

static void
ctags_menu_finalize (CtagsMenu *menu)
{
  G_OBJECT_CLASS (ctags_menu_parent_class)->finalize (G_OBJECT (menu));
}

GtkWidget*
ctags_menu_new (GtkAccelGroup *accel_group)
{
  GtkWidget *menu;
  GtkWidget *submenu;
  
  menu = g_object_new (ctags_menu_get_type (), NULL);

  submenu = gtk_menu_new ();
  gtk_menu_item_set_submenu (GTK_MENU_ITEM (menu), submenu);
  
  add_menu_items (CTAGS_MENU (menu), submenu, accel_group);

  return menu;
}

static void
add_menu_items (CtagsMenu     *menu,
                GtkWidget     *submenu,
                GtkAccelGroup *accel_group)
{
  GtkWidget *find_item;
  GtkWidget *previous_item;
  GtkWidget *next_item;

  find_item = codeslayer_menu_item_new_with_label (_("Find Tag"));
  gtk_widget_add_accelerator (find_item, "activate", accel_group, 
                              GDK_KEY_F4, 0, GTK_ACCEL_VISIBLE);  
  gtk_menu_shell_append (GTK_MENU_SHELL (submenu), find_item);

  previous_item = codeslayer_menu_item_new_with_label (_("Previous"));
  gtk_widget_add_accelerator (previous_item, "activate", accel_group, 
                              GDK_KEY_Left, GDK_MOD1_MASK, GTK_ACCEL_VISIBLE); 
  gtk_menu_shell_append (GTK_MENU_SHELL (submenu), previous_item);

  next_item = codeslayer_menu_item_new_with_label (_("Next"));
  gtk_widget_add_accelerator (next_item, "activate", accel_group, 
                              GDK_KEY_Right, GDK_MOD1_MASK, GTK_ACCEL_VISIBLE);
  gtk_menu_shell_append (GTK_MENU_SHELL (submenu), next_item);
  
  g_signal_connect_swapped (G_OBJECT (find_item), "activate", 
                            G_CALLBACK (find_tag_action), menu);

  g_signal_connect_swapped (G_OBJECT (previous_item), "activate", 
                            G_CALLBACK (previous_action), menu);
   
  g_signal_connect_swapped (G_OBJECT (next_item), "activate", 
                            G_CALLBACK (next_action), menu);
}

static void 
find_tag_action (CtagsMenu *menu) 
{
  g_signal_emit_by_name ((gpointer) menu, "find-tag");
}

static void 
previous_action (CtagsMenu *menu) 
{
  g_signal_emit_by_name ((gpointer) menu, "previous");
}

static void 
next_action (CtagsMenu *menu) 
{
  g_signal_emit_by_name ((gpointer) menu, "next");
}
