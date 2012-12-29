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

#ifndef __CTAGS_CONFIG_H__
#define	__CTAGS_CONFIG_H__

#include <gtk/gtk.h>
#include <codeslayer/codeslayer.h>

G_BEGIN_DECLS

#define CTAGS_CONFIG_TYPE            (ctags_config_get_type ())
#define CTAGS_CONFIG(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), CTAGS_CONFIG_TYPE, CtagsConfig))
#define CTAGS_CONFIG_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), CTAGS_CONFIG_TYPE, CtagsConfigClass))
#define IS_CTAGS_CONFIG(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), CTAGS_CONFIG_TYPE))
#define IS_CTAGS_CONFIG_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), CTAGS_CONFIG_TYPE))

typedef struct _CtagsConfig CtagsConfig;
typedef struct _CtagsConfigClass CtagsConfigClass;

struct _CtagsConfig
{
  GObject parent_instance;
};

struct _CtagsConfigClass
{
  GObjectClass parent_class;
};

GType ctags_config_get_type (void) G_GNUC_CONST;

CtagsConfig*        ctags_config_new                (void);

CodeSlayerProject*  ctags_config_get_project        (CtagsConfig        *config);
void                ctags_config_set_project        (CtagsConfig        *config,
                                                     CodeSlayerProject  *project);
const gchar*        ctags_config_get_source_folder  (CtagsConfig        *config);
void                ctags_config_set_source_folder  (CtagsConfig        *config,
                                                     const gchar        *source_folder);

G_END_DECLS

#endif /* __CTAGS_CONFIG_H__ */
