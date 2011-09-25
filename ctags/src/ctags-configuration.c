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

#include "ctags-configuration.h"

static void ctags_configuration_class_init    (CtagsConfigurationClass *klass);
static void ctags_configuration_init          (CtagsConfiguration      *configuration);
static void ctags_configuration_finalize      (CtagsConfiguration      *configuration);
static void ctags_configuration_get_property  (GObject                 *object, 
                                               guint                    prop_id,
                                               GValue                  *value,
                                               GParamSpec              *pspec);
static void ctags_configuration_set_property  (GObject                 *object, 
                                               guint                    prop_id,
                                               const GValue            *value,
                                               GParamSpec              *pspec);

#define CTAGS_CONFIGURATION_GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), CTAGS_CONFIGURATION_TYPE, CtagsConfigurationPrivate))

typedef struct _CtagsConfigurationPrivate CtagsConfigurationPrivate;

struct _CtagsConfigurationPrivate
{
  gchar *project_key;
  gchar *source_directory;
};

enum
{
  PROP_0,
  PROP_PROJECT_KEY,
  PROP_SOURCE_DIRECTORY
};

G_DEFINE_TYPE (CtagsConfiguration, ctags_configuration, G_TYPE_OBJECT)
     
static void 
ctags_configuration_class_init (CtagsConfigurationClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

  gobject_class->finalize = (GObjectFinalizeFunc) ctags_configuration_finalize;

  gobject_class->get_property = ctags_configuration_get_property;
  gobject_class->set_property = ctags_configuration_set_property;

  g_type_class_add_private (klass, sizeof (CtagsConfigurationPrivate));

  g_object_class_install_property (gobject_class, 
                                   PROP_PROJECT_KEY,
                                   g_param_spec_string ("project_key", 
                                                        "Project Key",
                                                        "Project Key Object", "",
                                                        G_PARAM_READWRITE));

  g_object_class_install_property (gobject_class, 
                                   PROP_SOURCE_DIRECTORY,
                                   g_param_spec_string ("source_directory",
                                                        "Source Directory",
                                                        "Source Directory Object",
                                                        "",
                                                        G_PARAM_READWRITE));
}

static void
ctags_configuration_init (CtagsConfiguration *configuration)
{
  CtagsConfigurationPrivate *priv;
  priv = CTAGS_CONFIGURATION_GET_PRIVATE (configuration);
  priv->project_key = NULL;
  priv->source_directory = NULL;
}

static void
ctags_configuration_finalize (CtagsConfiguration *configuration)
{
  CtagsConfigurationPrivate *priv;
  priv = CTAGS_CONFIGURATION_GET_PRIVATE (configuration);
  if (priv->project_key)
    {
      g_free (priv->project_key);
      priv->project_key = NULL;
    }
  if (priv->source_directory)
    {
      g_free (priv->source_directory);
      priv->source_directory = NULL;
    }
  G_OBJECT_CLASS (ctags_configuration_parent_class)->finalize (G_OBJECT (configuration));
}

static void
ctags_configuration_get_property (GObject    *object, 
                                  guint       prop_id,
                                  GValue     *value, 
                                  GParamSpec *pspec)
{
  CtagsConfiguration *configuration;
  CtagsConfigurationPrivate *priv;
  
  configuration = CTAGS_CONFIGURATION (object);
  priv = CTAGS_CONFIGURATION_GET_PRIVATE (configuration);

  switch (prop_id)
    {
    case PROP_PROJECT_KEY:
      g_value_set_string (value, priv->project_key);
      break;
    case PROP_SOURCE_DIRECTORY:
      g_value_set_string (value, priv->source_directory);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
ctags_configuration_set_property (GObject      *object, 
                                  guint         prop_id,
                                  const GValue *value, 
                                  GParamSpec   *pspec)
{
  CtagsConfiguration *configuration;
  configuration = CTAGS_CONFIGURATION (object);

  switch (prop_id)
    {
    case PROP_PROJECT_KEY:
      ctags_configuration_set_project_key (configuration, g_value_get_string (value));
      break;
    case PROP_SOURCE_DIRECTORY:
      ctags_configuration_set_source_directory (configuration, g_value_get_string (value));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

CtagsConfiguration*
ctags_configuration_new (void)
{
  return CTAGS_CONFIGURATION (g_object_new (ctags_configuration_get_type (), NULL));
}

const gchar*
ctags_configuration_get_project_key (CtagsConfiguration *configuration)
{
  return CTAGS_CONFIGURATION_GET_PRIVATE (configuration)->project_key;
}

void
ctags_configuration_set_project_key (CtagsConfiguration *configuration, 
                                     const gchar        *project_key)
{
  CtagsConfigurationPrivate *priv;
  priv = CTAGS_CONFIGURATION_GET_PRIVATE (configuration);
  if (priv->project_key)
    {
      g_free (priv->project_key);
      priv->project_key = NULL;
    }
  priv->project_key = g_strdup (project_key);
}

const gchar*
ctags_configuration_get_source_directory (CtagsConfiguration *configuration)
{
  return CTAGS_CONFIGURATION_GET_PRIVATE (configuration)->source_directory;
}

void
ctags_configuration_set_source_directory (CtagsConfiguration *configuration,
                                          const gchar        *source_directory)
{
  CtagsConfigurationPrivate *priv;
  priv = CTAGS_CONFIGURATION_GET_PRIVATE (configuration);
  if (priv->source_directory)
    {
      g_free (priv->source_directory);
      priv->source_directory = NULL;
    }
  priv->source_directory = g_strdup (source_directory);
}
