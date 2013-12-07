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

#include "navigation-engine.h"
#include "navigation-node.h"

static void navigation_engine_class_init  (NavigationEngineClass *klass);
static void navigation_engine_init        (NavigationEngine      *engine);
static void navigation_engine_finalize    (NavigationEngine      *engine);

static void path_navigated_action         (NavigationEngine      *engine,
                                           gchar                 *from_file_path,
                                           gint                   from_line_number,
                                           gchar                 *to_file_path,
                                           gint                   to_line_number);
static void previous_action               (NavigationEngine      *engine);
static void next_action                   (NavigationEngine      *engine);
static void clear_path                    (NavigationEngine      *engine);

#define NAVIGATION_ENGINE_GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), NAVIGATION_ENGINE_TYPE, NavigationEnginePrivate))

#define MAIN "main"
#define SHOW_SIDE_PANE "show_side_pane"

typedef struct _NavigationEnginePrivate NavigationEnginePrivate;

struct _NavigationEnginePrivate
{
  CodeSlayer *codeslayer;
  gulong      path_navigated_id;
  GList      *path;
  gint        position;
};

G_DEFINE_TYPE (NavigationEngine, navigation_engine, G_TYPE_OBJECT)

static void
navigation_engine_class_init (NavigationEngineClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  gobject_class->finalize = (GObjectFinalizeFunc) navigation_engine_finalize;
  g_type_class_add_private (klass, sizeof (NavigationEnginePrivate));
}

static void
navigation_engine_init (NavigationEngine *engine) 
{
  NavigationEnginePrivate *priv;
  priv = NAVIGATION_ENGINE_GET_PRIVATE (engine);
  priv->path = NULL;
}

static void
navigation_engine_finalize (NavigationEngine *engine)
{
  NavigationEnginePrivate *priv;
  priv = NAVIGATION_ENGINE_GET_PRIVATE (engine);
  
  g_signal_handler_disconnect (priv->codeslayer, priv->path_navigated_id);

  g_list_foreach (priv->path, (GFunc) g_object_unref, NULL);
  
  if (priv->path != NULL)
    g_list_free (priv->path);
  
  G_OBJECT_CLASS (navigation_engine_parent_class)->finalize (G_OBJECT (engine));
}

NavigationEngine*
navigation_engine_new (CodeSlayer *codeslayer, 
                       GtkWidget  *menu)
{
  NavigationEnginePrivate *priv;
  NavigationEngine *engine;

  engine = NAVIGATION_ENGINE (g_object_new (navigation_engine_get_type (), NULL));
  priv = NAVIGATION_ENGINE_GET_PRIVATE (engine);

  priv->codeslayer = codeslayer;
  
  g_signal_connect_swapped (G_OBJECT (menu), "previous", 
                            G_CALLBACK (previous_action), engine);
  
  g_signal_connect_swapped (G_OBJECT (menu), "next", 
                            G_CALLBACK (next_action), engine);

  priv->path_navigated_id = g_signal_connect_swapped (G_OBJECT (codeslayer), "path-navigated", 
                                                        G_CALLBACK (path_navigated_action), engine);
                                                      
  return engine;
}

static NavigationNode*
create_node (gchar *file_path,
             gint   line_number)
{
  NavigationNode *node;
  node = navigation_node_new ();
  navigation_node_set_file_path (node, file_path); 
  navigation_node_set_line_number (node, line_number); 
  return node;
}

static void
clear_forward_positions (NavigationEngine *engine)
{
  NavigationEnginePrivate *priv;
  gint length;

  priv = NAVIGATION_ENGINE_GET_PRIVATE (engine);
  length = g_list_length (priv->path);
  
  while (priv->position < length - 1)
    {
      NavigationNode *node = g_list_nth_data (priv->path, length - 1);
      priv->path = g_list_remove (priv->path, node);
      g_object_unref (node);
      length = g_list_length (priv->path);
    }
}

static void
path_navigated_action (NavigationEngine *engine,
                       gchar            *from_file_path,
                       gint              from_line_number,
                       gchar            *to_file_path,
                       gint              to_line_number)
{
  NavigationEnginePrivate *priv;
  priv = NAVIGATION_ENGINE_GET_PRIVATE (engine);
  
  if (priv->path == NULL)
    {
      priv->path = g_list_append (priv->path, create_node (from_file_path, from_line_number));
      priv->position = 0;
    }
  else
    {
      NavigationNode *curr_node;
      NavigationNode *from_node;
    
      clear_forward_positions (engine);
      
      curr_node = g_list_nth_data (priv->path, priv->position);      
      from_node = create_node (from_file_path, from_line_number);
      
      if (!navigation_node_equals (curr_node, from_node))
        {
          priv->path = g_list_append (priv->path, from_node);
          priv->position = g_list_length (priv->path) - 1;
        }
      else
        {
          g_object_unref (from_node);        
        }
    }
  
  priv->path = g_list_append (priv->path, create_node (to_file_path, to_line_number));
  priv->position = g_list_length (priv->path) - 1;
  
  while (g_list_length (priv->path) > 25)
    {
      NavigationNode *first_node;
      first_node = g_list_nth_data (priv->path, 0);
      priv->path = g_list_remove (priv->path, first_node);
      priv->position = g_list_length (priv->path) - 1;
    }
}

static void
previous_action (NavigationEngine *engine)
{
  NavigationEnginePrivate *priv;
  NavigationNode *node;
  const gchar *file_path;
  gint line_number;
  
  priv = NAVIGATION_ENGINE_GET_PRIVATE (engine);
  
  if (priv->position <= 0)
    return;
  
  priv->position = priv->position - 1;
  
  node = g_list_nth_data (priv->path, priv->position);
  
  file_path = navigation_node_get_file_path (node);
  line_number = navigation_node_get_line_number (node);
  
  codeslayer_select_document_by_file_path (priv->codeslayer, file_path, line_number);
}

static void
next_action (NavigationEngine *engine)
{
  NavigationEnginePrivate *priv;
  NavigationNode *node;
  const gchar *file_path;
  gint line_number;
  gint length;
  
  priv = NAVIGATION_ENGINE_GET_PRIVATE (engine);
  
  length = g_list_length (priv->path);
  
  if (priv->position >= length - 1)
    return;
  
  priv->position = priv->position + 1;
  
  node = g_list_nth_data (priv->path, priv->position);
  
  file_path = navigation_node_get_file_path (node);
  line_number = navigation_node_get_line_number (node);
  
  if (!codeslayer_select_document_by_file_path (priv->codeslayer, file_path, line_number))
    {
      clear_path (engine);    
    }
}

static void
clear_path (NavigationEngine *engine)
{
  NavigationEnginePrivate *priv;
  priv = NAVIGATION_ENGINE_GET_PRIVATE (engine);
  g_list_foreach (priv->path, (GFunc) g_object_unref, NULL);
  g_list_free (priv->path);
  priv->path = NULL;
  priv->position = 0;
}
