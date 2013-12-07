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

#include "navigation-node.h"

static void navigation_node_class_init    (NavigationNodeClass *klass);
static void navigation_node_init          (NavigationNode      *node);
static void navigation_node_finalize      (NavigationNode      *node);

#define NAVIGATION_NODE_GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), NAVIGATION_NODE_TYPE, NavigationNodePrivate))

typedef struct _NavigationNodePrivate NavigationNodePrivate;

struct _NavigationNodePrivate
{
  gchar *file_path;
  gint   line_number;
};

G_DEFINE_TYPE (NavigationNode, navigation_node, G_TYPE_OBJECT)

enum
{
  PROP_0,
  PROP_FILE_PATH,
  PROP_LINE_NUMBER
};

static void 
navigation_node_class_init (NavigationNodeClass *klass)
{
  G_OBJECT_CLASS (klass)->finalize = (GObjectFinalizeFunc) navigation_node_finalize;
  g_type_class_add_private (klass, sizeof (NavigationNodePrivate));
}

static void
navigation_node_init (NavigationNode *node)
{
  NavigationNodePrivate *priv;
  priv = NAVIGATION_NODE_GET_PRIVATE (node);
  priv->file_path = NULL;
}

static void
navigation_node_finalize (NavigationNode *node)
{
  NavigationNodePrivate *priv;
  priv = NAVIGATION_NODE_GET_PRIVATE (node);

  if (priv->file_path != NULL)
    g_free (priv->file_path);
      
  G_OBJECT_CLASS (navigation_node_parent_class)->finalize (G_OBJECT (node));
}

NavigationNode *
navigation_node_new (void)
{
  return NAVIGATION_NODE (g_object_new (navigation_node_get_type (), NULL));
}

const gint
navigation_node_get_line_number (NavigationNode *node)
{
  return NAVIGATION_NODE_GET_PRIVATE (node)->line_number;
}

void
navigation_node_set_line_number (NavigationNode *node,
                                 const gint      line_number)
{
  NavigationNodePrivate *priv;
  priv = NAVIGATION_NODE_GET_PRIVATE (node);
  priv->line_number = line_number;
}

const gchar *
navigation_node_get_file_path (NavigationNode *node)
{
  return NAVIGATION_NODE_GET_PRIVATE (node)->file_path;
}

void
navigation_node_set_file_path (NavigationNode *node,
                               const gchar    *file_path)
{
  NavigationNodePrivate *priv;
  priv = NAVIGATION_NODE_GET_PRIVATE (node);
  if (priv->file_path)
    {
      g_free (priv->file_path);
      priv->file_path = NULL;
    }
  priv->file_path = g_strdup (file_path);
}

gboolean
navigation_node_equals (NavigationNode *node, 
                        NavigationNode *that)
{
  NavigationNodePrivate *priv;
  const gchar *file_path;
  gint line_number;
  
  priv = NAVIGATION_NODE_GET_PRIVATE (node);
  
  file_path = navigation_node_get_file_path (that);
  line_number = navigation_node_get_line_number (that);
  
  return g_strcmp0 (file_path, priv->file_path) == 0 && 
         line_number == priv->line_number;
}                        
