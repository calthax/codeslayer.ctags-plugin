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

#ifndef __CTAGS_PROJECT_PROPERTIES_H__
#define	__CTAGS_PROJECT_PROPERTIES_H__

#include <gtk/gtk.h>
#include <codeslayer/codeslayer.h>
#include "ctags-config.h"

G_BEGIN_DECLS

#define CTAGS_PROJECT_PROPERTIES_TYPE            (ctags_project_properties_get_type ())
#define CTAGS_PROJECT_PROPERTIES(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), CTAGS_PROJECT_PROPERTIES_TYPE, CtagsProjectProperties))
#define CTAGS_PROJECT_PROPERTIES_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), CTAGS_PROJECT_PROPERTIES_TYPE, CtagsProjectPropertiesClass))
#define IS_CTAGS_PROJECT_PROPERTIES(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), CTAGS_PROJECT_PROPERTIES_TYPE))
#define IS_CTAGS_PROJECT_PROPERTIES_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), CTAGS_PROJECT_PROPERTIES_TYPE))

typedef struct _CtagsProjectProperties CtagsProjectProperties;
typedef struct _CtagsProjectPropertiesClass CtagsProjectPropertiesClass;

struct _CtagsProjectProperties
{
  GtkVBox parent_instance;
};

struct _CtagsProjectPropertiesClass
{
  GtkVBoxClass parent_class;
  
  void (*save_config) (CtagsProjectProperties *project_properties);  
};

GType ctags_project_properties_get_type (void) G_GNUC_CONST;
     
GtkWidget*  ctags_project_properties_new     (void);

void        ctags_project_properties_opened  (CtagsProjectProperties *project_properties,
                                              CtagsConfig            *config, 
                                              CodeSlayerProject      *project);

void        ctags_project_properties_saved   (CtagsProjectProperties *project_properties,
                                              CtagsConfig            *config, 
                                              CodeSlayerProject      *project);

G_END_DECLS

#endif /* __CTAGS_PROJECT_PROPERTIES_H__ */
