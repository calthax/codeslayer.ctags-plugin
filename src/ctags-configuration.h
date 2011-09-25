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

#ifndef __CTAGS_CONFIGURATION_H__
#define	__CTAGS_CONFIGURATION_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define CTAGS_CONFIGURATION_TYPE            (ctags_configuration_get_type ())
#define CTAGS_CONFIGURATION(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), CTAGS_CONFIGURATION_TYPE, CtagsConfiguration))
#define CTAGS_CONFIGURATION_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), CTAGS_CONFIGURATION_TYPE, CtagsConfigurationClass))
#define IS_CTAGS_CONFIGURATION(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), CTAGS_CONFIGURATION_TYPE))
#define IS_CTAGS_CONFIGURATION_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), CTAGS_CONFIGURATION_TYPE))

typedef struct _CtagsConfiguration CtagsConfiguration;
typedef struct _CtagsConfigurationClass CtagsConfigurationClass;

struct _CtagsConfiguration
{
  GObject parent_instance;
};

struct _CtagsConfigurationClass
{
  GObjectClass parent_class;
};

GType ctags_configuration_get_type (void) G_GNUC_CONST;

CtagsConfiguration*  ctags_configuration_new                   (void);

const gchar*         ctags_configuration_get_project_key       (CtagsConfiguration *configuration);
void                 ctags_configuration_set_project_key       (CtagsConfiguration *configuration,
                                                                const gchar        *project_key);
const gchar*         ctags_configuration_get_source_directory  (CtagsConfiguration *configuration);
void                 ctags_configuration_set_source_directory  (CtagsConfiguration *configuration,
                                                                const gchar        *source_directory);

G_END_DECLS

#endif /* __CTAGS_CONFIGURATION_H__ */
