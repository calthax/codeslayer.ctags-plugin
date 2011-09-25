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

#ifndef __CTAGS_MENU_H__
#define	__CTAGS_MENU_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define CTAGS_MENU_TYPE            (ctags_menu_get_type ())
#define CTAGS_MENU(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), CTAGS_MENU_TYPE, CtagsMenu))
#define CTAGS_MENU_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), CTAGS_MENU_TYPE, CtagsMenuClass))
#define IS_CTAGS_MENU(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), CTAGS_MENU_TYPE))
#define IS_CTAGS_MENU_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), CTAGS_MENU_TYPE))

typedef struct _CtagsMenu CtagsMenu;
typedef struct _CtagsMenuClass CtagsMenuClass;

struct _CtagsMenu
{
  GtkMenuItem parent_instance;
};

struct _CtagsMenuClass
{
  GtkMenuItemClass parent_class;

  void (*find_tag) (CtagsMenu *menu);
};

GType ctags_menu_get_type (void) G_GNUC_CONST;
  
GtkWidget*  ctags_menu_new  (GtkAccelGroup *accel_group);
                                            
G_END_DECLS

#endif /* __CTAGS_MENU_H__ */
