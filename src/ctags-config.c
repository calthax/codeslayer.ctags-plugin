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

#include "ctags-config.h"

static void ctags_config_class_init (CtagsConfigClass *klass);
static void ctags_config_init       (CtagsConfig      *config);
static void ctags_config_finalize   (CtagsConfig      *config);

#define CTAGS_CONFIG_GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), CTAGS_CONFIG_TYPE, CtagsConfigPrivate))

typedef struct _CtagsConfigPrivate CtagsConfigPrivate;

struct _CtagsConfigPrivate
{
  CodeSlayerProject *project;
  gchar             *project_key;
  gchar             *source_folder;
};

enum
{
  PROP_0,
  PROP_PROJECT_KEY,
  PROP_SOURCE_DIRECTORY
};

G_DEFINE_TYPE (CtagsConfig, ctags_config, G_TYPE_OBJECT)
     
static void 
ctags_config_class_init (CtagsConfigClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  gobject_class->finalize = (GObjectFinalizeFunc) ctags_config_finalize;
  g_type_class_add_private (klass, sizeof (CtagsConfigPrivate));
}

static void
ctags_config_init (CtagsConfig *config)
{
  CtagsConfigPrivate *priv;
  priv = CTAGS_CONFIG_GET_PRIVATE (config);
  priv->project_key = NULL;
  priv->source_folder = NULL;
}

static void
ctags_config_finalize (CtagsConfig *config)
{
  CtagsConfigPrivate *priv;
  priv = CTAGS_CONFIG_GET_PRIVATE (config);
  if (priv->project_key)
    {
      g_free (priv->project_key);
      priv->project_key = NULL;
    }
  if (priv->source_folder)
    {
      g_free (priv->source_folder);
      priv->source_folder = NULL;
    }
  G_OBJECT_CLASS (ctags_config_parent_class)->finalize (G_OBJECT (config));
}

CtagsConfig*
ctags_config_new (void)
{
  return CTAGS_CONFIG (g_object_new (ctags_config_get_type (), NULL));
}

CodeSlayerProject*   
ctags_config_get_project (CtagsConfig *config)
{
  CtagsConfigPrivate *priv;
  priv = CTAGS_CONFIG_GET_PRIVATE (config);
  return priv->project;
}

void                 
ctags_config_set_project (CtagsConfig *config,
                          CodeSlayerProject  *project)
{
  CtagsConfigPrivate *priv;
  priv = CTAGS_CONFIG_GET_PRIVATE (config);
  priv->project = project;
}

const gchar*
ctags_config_get_source_folder (CtagsConfig *config)
{
  return CTAGS_CONFIG_GET_PRIVATE (config)->source_folder;
}

void
ctags_config_set_source_folder (CtagsConfig *config,
                                const gchar *source_folder)
{
  CtagsConfigPrivate *priv;
  priv = CTAGS_CONFIG_GET_PRIVATE (config);
  if (priv->source_folder)
    {
      g_free (priv->source_folder);
      priv->source_folder = NULL;
    }
  priv->source_folder = g_strdup (source_folder);
}
