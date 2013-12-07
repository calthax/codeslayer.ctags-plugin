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

#include "ctags-path-node.h"

static void ctags_path_node_class_init    (CtagsPathNodeClass *klass);
static void ctags_path_node_init          (CtagsPathNode      *node);
static void ctags_path_node_finalize      (CtagsPathNode      *node);

#define CTAGS_PATH_NODE_GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), CTAGS_PATH_NODE_TYPE, CtagsPathNodePrivate))

typedef struct _CtagsPathNodePrivate CtagsPathNodePrivate;

struct _CtagsPathNodePrivate
{
  gchar *file_path;
  gint   line_number;
};

G_DEFINE_TYPE (CtagsPathNode, ctags_path_node, G_TYPE_OBJECT)

static void 
ctags_path_node_class_init (CtagsPathNodeClass *klass)
{
  G_OBJECT_CLASS (klass)->finalize = (GObjectFinalizeFunc) ctags_path_node_finalize;
  g_type_class_add_private (klass, sizeof (CtagsPathNodePrivate));
}

static void
ctags_path_node_init (CtagsPathNode *node)
{
  CtagsPathNodePrivate *priv;
  priv = CTAGS_PATH_NODE_GET_PRIVATE (node);
  priv->file_path = NULL;
}

static void
ctags_path_node_finalize (CtagsPathNode *node)
{
  CtagsPathNodePrivate *priv;
  priv = CTAGS_PATH_NODE_GET_PRIVATE (node);

  if (priv->file_path != NULL)
    g_free (priv->file_path);
      
  G_OBJECT_CLASS (ctags_path_node_parent_class)->finalize (G_OBJECT (node));
}

CtagsPathNode *
ctags_path_node_new (void)
{
  return CTAGS_PATH_NODE (g_object_new (ctags_path_node_get_type (), NULL));
}

const gint
ctags_path_node_get_line_number (CtagsPathNode *node)
{
  return CTAGS_PATH_NODE_GET_PRIVATE (node)->line_number;
}

void
ctags_path_node_set_line_number (CtagsPathNode *node,
                                 const gint      line_number)
{
  CtagsPathNodePrivate *priv;
  priv = CTAGS_PATH_NODE_GET_PRIVATE (node);
  priv->line_number = line_number;
}

const gchar *
ctags_path_node_get_file_path (CtagsPathNode *node)
{
  return CTAGS_PATH_NODE_GET_PRIVATE (node)->file_path;
}

void
ctags_path_node_set_file_path (CtagsPathNode *node,
                               const gchar    *file_path)
{
  CtagsPathNodePrivate *priv;
  priv = CTAGS_PATH_NODE_GET_PRIVATE (node);
  if (priv->file_path)
    {
      g_free (priv->file_path);
      priv->file_path = NULL;
    }
  priv->file_path = g_strdup (file_path);
}

gboolean
ctags_path_node_equals (CtagsPathNode *node, 
                        CtagsPathNode *that)
{
  CtagsPathNodePrivate *priv;
  const gchar *file_path;
  gint line_number;
  
  priv = CTAGS_PATH_NODE_GET_PRIVATE (node);
  
  file_path = ctags_path_node_get_file_path (that);
  line_number = ctags_path_node_get_line_number (that);
  
  return g_strcmp0 (file_path, priv->file_path) == 0 && 
         line_number == priv->line_number;
}                        
