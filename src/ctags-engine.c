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

#include <codeslayer/codeslayer-utils.h>
#include "ctags-engine.h"
#include "ctags-path-node.h"
#include "ctags-config.h"
#include "ctags-project-properties.h"
#include "readtags.h"

typedef struct
{
  gchar         *file_path;
  unsigned long  line_number;
} Tag;


#define MAIN "main"
#define SOURCE_FOLDER "source_folder"
#define CTAGS_CONF "ctags.conf"

static void ctags_engine_class_init           (CtagsEngineClass   *klass);
static void ctags_engine_init                 (CtagsEngine        *engine);
static void ctags_engine_finalize             (CtagsEngine        *engine);

static CtagsConfig* get_config_by_project     (CtagsEngine        *engine, 
                                               CodeSlayerProject  *project);
static void project_properties_opened_action  (CtagsEngine        *engine,
                                               CodeSlayerProject  *project);
static void project_properties_saved_action   (CtagsEngine        *engine,
                                               CodeSlayerProject  *project);                                                                        
static void save_config_action                (CtagsEngine        *engine,
                                               CtagsConfig        *config);
static void find_tag_action                   (CtagsEngine        *engine);
static void document_saved_action             (CtagsEngine        *engine, 
                                               CodeSlayerDocument *document);
static gboolean start_create_tags             (CtagsEngine        *engine);
static void finish_create_tags                (CtagsEngine        *engine);
static void execute_create_tags               (CtagsEngine        *engine);
                                                              
static GList *find_tags                       (CodeSlayer        *codeslayer, 
                                               const char *const   name, 
                                               const int           options);
static gboolean search_active_document        (CtagsEngine        *engine, 
                                               CodeSlayerDocument *document, 
                                               GList              *tags);
static gboolean search_projects               (CtagsEngine        *engine, 
                                               GList              *tags, 
                                               gboolean            search_headers);
static void select_document                   (CtagsEngine        *engine, 
                                               Tag                *tag);                                                              
static void previous_action                   (CtagsEngine        *engine);
static void next_action                       (CtagsEngine        *engine);
static void add_path                          (CtagsEngine        *engine,
                                               const gchar        *from_file_path,
                                               gint                from_line_number,
                                               const gchar        *to_file_path,
                                               gint                to_line_number);
static void clear_path                        (CtagsEngine        *engine);
                                                   
#define CTAGS_ENGINE_GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), CTAGS_ENGINE_TYPE, CtagsEnginePrivate))

typedef struct _CtagsEnginePrivate CtagsEnginePrivate;

struct _CtagsEnginePrivate
{
  CodeSlayer *codeslayer;
  GtkWidget  *menu;
  GtkWidget  *project_properties;
  gulong      properties_opened_id;
  gulong      properties_saved_id;
  gulong      saved_handler_id;
  guint       event_source_id;
  GList      *path;
  gint        position;
};

G_DEFINE_TYPE (CtagsEngine, ctags_engine, G_TYPE_OBJECT)

static void
ctags_engine_class_init (CtagsEngineClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  gobject_class->finalize = (GObjectFinalizeFunc) ctags_engine_finalize;
  g_type_class_add_private (klass, sizeof (CtagsEnginePrivate));
}

static void
ctags_engine_init (CtagsEngine *engine) 
{
  CtagsEnginePrivate *priv;
  priv = CTAGS_ENGINE_GET_PRIVATE (engine);
  priv->path = NULL;
}

static void
ctags_engine_finalize (CtagsEngine *engine)
{
  CtagsEnginePrivate *priv;
  priv = CTAGS_ENGINE_GET_PRIVATE (engine);
    
  g_list_foreach (priv->path, (GFunc) g_object_unref, NULL);
  
  if (priv->path != NULL)
    g_list_free (priv->path);
    
  g_signal_handler_disconnect (priv->codeslayer, priv->properties_opened_id);
  g_signal_handler_disconnect (priv->codeslayer, priv->properties_saved_id);
  g_signal_handler_disconnect (priv->codeslayer, priv->saved_handler_id);
  
  G_OBJECT_CLASS (ctags_engine_parent_class)->finalize (G_OBJECT(engine));
}

CtagsEngine*
ctags_engine_new (CodeSlayer *codeslayer,
                  GtkWidget  *menu, 
                  GtkWidget  *project_properties)
{
  CtagsEnginePrivate *priv;
  CtagsEngine *engine;

  engine = CTAGS_ENGINE (g_object_new (ctags_engine_get_type (), NULL));
  priv = CTAGS_ENGINE_GET_PRIVATE (engine);

  priv->codeslayer = codeslayer;
  priv->menu = menu;
  priv->project_properties = project_properties;
  priv->event_source_id = 0;
  
  g_signal_connect_swapped (G_OBJECT (menu), "find-tag",
                            G_CALLBACK (find_tag_action), engine);

  g_signal_connect_swapped (G_OBJECT (menu), "previous", 
                            G_CALLBACK (previous_action), engine);
  
  g_signal_connect_swapped (G_OBJECT (menu), "next", 
                            G_CALLBACK (next_action), engine);

  priv->properties_opened_id =  g_signal_connect_swapped (G_OBJECT (codeslayer), "project-properties-opened",
                                                          G_CALLBACK (project_properties_opened_action), engine);

  priv->properties_saved_id = g_signal_connect_swapped (G_OBJECT (codeslayer), "project-properties-saved",
                                                        G_CALLBACK (project_properties_saved_action), engine);

  priv->saved_handler_id = g_signal_connect_swapped (G_OBJECT (codeslayer), "document-saved", 
                                                     G_CALLBACK (document_saved_action), engine);

  g_signal_connect_swapped (G_OBJECT (project_properties), "save-config",
                            G_CALLBACK (save_config_action), engine);

  return engine;
}

static CtagsConfig*
get_config_by_project (CtagsEngine       *engine, 
                       CodeSlayerProject *project)
{
  CtagsEnginePrivate *priv;
  CtagsConfig *config;
  GKeyFile *key_file;
  gchar *source_folder;
  gchar *folder_path;
  gchar *file_path;
  
  priv = CTAGS_ENGINE_GET_PRIVATE (engine);

  folder_path = codeslayer_get_project_config_folder_path (priv->codeslayer, project);
  file_path = g_build_filename (folder_path, CTAGS_CONF, NULL);
  
  if (!codeslayer_utils_file_exists (file_path))
    {
      g_free (folder_path);
      g_free (file_path);
      return NULL;
    }

  key_file = codeslayer_utils_get_key_file (file_path);
  source_folder = g_key_file_get_string (key_file, MAIN, SOURCE_FOLDER, NULL);
  
  config = ctags_config_new ();
  ctags_config_set_project (config, project);
  ctags_config_set_source_folder (config, source_folder);
  
  g_free (folder_path);
  g_free (file_path);
  g_free (source_folder);
  g_key_file_free (key_file);
  
  return config;
}

static void
project_properties_opened_action (CtagsEngine       *engine,
                                  CodeSlayerProject *project)
{
  CtagsEnginePrivate *priv;
  CtagsConfig *config;
  priv = CTAGS_ENGINE_GET_PRIVATE (engine);
  config = get_config_by_project (engine, project);
  ctags_project_properties_opened (CTAGS_PROJECT_PROPERTIES (priv->project_properties),
                                   config, project);
  if (config != NULL)
    g_object_unref (config);
}

static void
project_properties_saved_action (CtagsEngine       *engine,
                                 CodeSlayerProject *project)
{
  CtagsEnginePrivate *priv;
  CtagsConfig *config;
  priv = CTAGS_ENGINE_GET_PRIVATE (engine);
  config = get_config_by_project (engine, project);
  ctags_project_properties_saved (CTAGS_PROJECT_PROPERTIES (priv->project_properties),
                                  config, project);
  if (config != NULL)
    g_object_unref (config);
}

static void
save_config_action (CtagsEngine *engine,
                    CtagsConfig *config)
{
  CtagsEnginePrivate *priv;
  CodeSlayerProject *project;
  gchar *folder_path;
  gchar *file_path;
  const gchar *source_folder;
  GKeyFile *key_file;
 
  priv = CTAGS_ENGINE_GET_PRIVATE (engine);

  project = ctags_config_get_project (config);  
  folder_path = codeslayer_get_project_config_folder_path (priv->codeslayer, project);
  file_path = codeslayer_utils_get_file_path (folder_path, CTAGS_CONF);
  key_file = codeslayer_utils_get_key_file (file_path);

  source_folder = ctags_config_get_source_folder (config);
  g_key_file_set_string (key_file, MAIN, SOURCE_FOLDER, source_folder);

  codeslayer_utils_save_key_file (key_file, file_path);  
  g_key_file_free (key_file);
  g_free (folder_path);
  g_free (file_path);
  
  execute_create_tags (engine);
}

static void 
document_saved_action (CtagsEngine        *engine, 
                       CodeSlayerDocument *document) 
{
  execute_create_tags (engine);
}

/*
 * wait a second before we regenerate the tag file in case 
 * we are saving multiple documents at once.
 */
static void
execute_create_tags (CtagsEngine *engine) 
{
  CtagsEnginePrivate *priv;
  priv = CTAGS_ENGINE_GET_PRIVATE (engine);

  if (priv->event_source_id == 0)
    {
      priv->event_source_id = g_timeout_add_seconds_full (G_PRIORITY_DEFAULT, 2,
                                                          (GSourceFunc ) start_create_tags,
                                                          engine,
                                                          (GDestroyNotify) finish_create_tags);
    }
}

static gboolean
start_create_tags (CtagsEngine *engine)
{
  CtagsEnginePrivate *priv;
  FILE *file;
  GList *projects;
  GList *list;
  GString *string;
  gchar *command;
  gchar *profile_folder_path;

  priv = CTAGS_ENGINE_GET_PRIVATE (engine);
  
  profile_folder_path = codeslayer_get_profile_config_folder_path (priv->codeslayer);
  
  string = g_string_new ("cd ");
  string = g_string_append (string, profile_folder_path);
  string = g_string_append (string, ";ctags -R --fields=n");
  
  projects = codeslayer_get_projects (priv->codeslayer);
  list = projects;
  while (list != NULL)
    {
      CodeSlayerProject *project = list->data;
      CtagsConfig *config = get_config_by_project (engine, project);
      if (config != NULL)
        {
          const gchar *source_folder;
          source_folder = ctags_config_get_source_folder (config);
          string = g_string_append (string, " ");
          string = g_string_append (string, source_folder);
          g_object_unref (config);
        }
      list = g_list_next (list);
    }
  g_list_free (projects);

  command = g_string_free (string, FALSE);
  file = popen (command, "r");
  
  if (file != NULL)
    pclose (file);
  
  g_free (command);
  g_free (profile_folder_path);
  
  return FALSE;  
}

static void
finish_create_tags (CtagsEngine *engine)
{
  CtagsEnginePrivate *priv;
  priv = CTAGS_ENGINE_GET_PRIVATE (engine);
  priv->event_source_id = 0;
}

static GList*
find_tags (CodeSlayer        *codeslayer, 
           const char *const  name, 
           const int          options)
{
  GList *results = NULL; 

	tagFileInfo info;
	tagFile *tag_file;
	tagEntry entry;
	gchar *profile_folder_path;
	gchar *tag_file_path;
	
  profile_folder_path = codeslayer_get_profile_config_folder_path (codeslayer);
  tag_file_path = g_build_filename (profile_folder_path, "tags", NULL);
	tag_file = tagsOpen (tag_file_path, &info);

	if (tag_file == NULL)
  	{
  	  g_warning ("Could not open the tags file");
		  return NULL;
	  }
	
  if (tagsFind (tag_file, &entry, name, options) == TagSuccess)
    {
      do
	      {
	        Tag *tag;
	        tag = g_malloc (sizeof (Tag));
	        tag->file_path = g_strdup (entry.file);
	        tag->line_number = entry.address.lineNumber;
          results = g_list_prepend (results, tag);
			  } while (tagsFindNext (tag_file, &entry) == TagSuccess);
			  results = g_list_reverse (results);
    }
    
  tagsClose (tag_file);
  
  g_free (profile_folder_path);
  g_free (tag_file_path);
  
  return results;
}

static void 
find_tag_action (CtagsEngine *engine)
{
  CtagsEnginePrivate *priv;
  CodeSlayerDocument *document;
  GtkSourceView *source_view;
  GtkTextBuffer *buffer;
  GList *tags;
  GList *tmp;

  GtkTextMark *insert_mark;
  GtkTextMark *selection_mark;
  gchar *text;

  GtkTextIter start, end;

  priv = CTAGS_ENGINE_GET_PRIVATE (engine);

  document = codeslayer_get_active_document (priv->codeslayer);
  
  if (document == NULL)
    return;
  
  source_view = codeslayer_document_get_source_view (document);
  buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (source_view));

  insert_mark = gtk_text_buffer_get_insert (buffer);    
  selection_mark = gtk_text_buffer_get_selection_bound (buffer);

  gtk_text_buffer_get_iter_at_mark (buffer, &start, insert_mark);
  gtk_text_buffer_get_iter_at_mark (buffer, &end, selection_mark);

  text = gtk_text_buffer_get_text (buffer, &start, &end, FALSE);
  
  if (text != NULL)
    g_strstrip (text);
  
  tags = find_tags (priv->codeslayer, text, 0);
  tmp = tags;
  
  if (tmp != NULL)
    {
      gboolean found;
      found = search_active_document (engine, document, tmp);
      if (!found)
        found = search_projects (engine, tmp, FALSE);
      if (!found)
        search_projects (engine, tmp, TRUE);
  
      while (tmp != NULL)
        {
          Tag *tag = tmp->data;
          g_free (tag->file_path);
          tmp = g_list_next (tmp);
        }
      g_list_foreach (tags, (GFunc) g_free, NULL);
      g_list_free (tags);
    }
    
  if (text != NULL)
    g_free (text);
}

static gboolean
search_active_document (CtagsEngine        *engine, 
                        CodeSlayerDocument *document, 
                        GList              *tags)
{
	const gchar *document_file_path;
	
	document_file_path = codeslayer_document_get_file_path (document);

  while (tags != NULL)
    {
      Tag *tag = tags->data;
      if (g_strcmp0 (document_file_path, tag->file_path) == 0)
        {
          select_document (engine, tag);
          return TRUE;
        }
      tags = g_list_next (tags);
    }
    
  return FALSE;    
}

static gboolean
search_projects (CtagsEngine *engine, 
                 GList       *tags, 
                 gboolean     search_headers)
{
  while (tags != NULL)
    {
      Tag *tag = tags->data;
      if (search_headers || !g_str_has_suffix (tag->file_path, ".h"))
        {
          select_document (engine, tag);
          return TRUE;
        }
      tags = g_list_next (tags);
    }
  
  return FALSE;    
}

static void
select_document (CtagsEngine *engine, 
                 Tag         *tag)
{
  CtagsEnginePrivate *priv;
  CodeSlayerDocument *from;
  const gchar* from_file_path;
  gint from_line_number;
  
  priv = CTAGS_ENGINE_GET_PRIVATE (engine);
  
  from = codeslayer_get_active_document (priv->codeslayer);
  from_file_path = codeslayer_document_get_file_path (from);
  from_line_number = codeslayer_document_get_line_number (from);

  if (codeslayer_select_document_by_file_path (priv->codeslayer, tag->file_path, tag->line_number))
    {
      CodeSlayerDocument *to;
      const gchar* to_file_path;
      gint to_line_number;
      
      to = codeslayer_get_active_document (priv->codeslayer);
      to_file_path = codeslayer_document_get_file_path (to);
      to_line_number = codeslayer_document_get_line_number (to);
      
      add_path (engine, from_file_path, from_line_number, to_file_path, to_line_number);
    }
}

static CtagsPathNode*
create_node (const gchar *file_path,
             gint         line_number)
{
  CtagsPathNode *node;
  node = ctags_path_node_new ();
  ctags_path_node_set_file_path (node, file_path); 
  ctags_path_node_set_line_number (node, line_number); 
  return node;
}

static void
clear_forward_positions (CtagsEngine *engine)
{
  CtagsEnginePrivate *priv;
  gint length;

  priv = CTAGS_ENGINE_GET_PRIVATE (engine);
  length = g_list_length (priv->path);
  
  while (priv->position < length - 1)
    {
      CtagsPathNode *node = g_list_nth_data (priv->path, length - 1);
      priv->path = g_list_remove (priv->path, node);
      g_object_unref (node);
      length = g_list_length (priv->path);
    }
}

static void
add_path (CtagsEngine *engine,
          const gchar *from_file_path,
          gint         from_line_number,
          const gchar *to_file_path,
          gint         to_line_number)
{
  CtagsEnginePrivate *priv;
  priv = CTAGS_ENGINE_GET_PRIVATE (engine);
  
  if (priv->path == NULL)
    {
      priv->path = g_list_append (priv->path, create_node (from_file_path, from_line_number));
      priv->position = 0;
    }
  else
    {
      CtagsPathNode *curr_node;
      CtagsPathNode *from_node;
    
      clear_forward_positions (engine);
      
      curr_node = g_list_nth_data (priv->path, priv->position);      
      from_node = create_node (from_file_path, from_line_number);
      
      if (!ctags_path_node_equals (curr_node, from_node))
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
      CtagsPathNode *first_node;
      first_node = g_list_nth_data (priv->path, 0);
      priv->path = g_list_remove (priv->path, first_node);
      priv->position = g_list_length (priv->path) - 1;
    }
}

static void
previous_action (CtagsEngine *engine)
{
  CtagsEnginePrivate *priv;
  CtagsPathNode *node;
  const gchar *file_path;
  gint line_number;
  
  priv = CTAGS_ENGINE_GET_PRIVATE (engine);
  
  if (priv->position <= 0)
    return;
  
  priv->position = priv->position - 1;
  
  node = g_list_nth_data (priv->path, priv->position);
  
  file_path = ctags_path_node_get_file_path (node);
  line_number = ctags_path_node_get_line_number (node);
  
  codeslayer_select_document_by_file_path (priv->codeslayer, file_path, line_number);
}

static void
next_action (CtagsEngine *engine)
{
  CtagsEnginePrivate *priv;
  CtagsPathNode *node;
  const gchar *file_path;
  gint line_number;
  gint length;
  
  priv = CTAGS_ENGINE_GET_PRIVATE (engine);
  
  length = g_list_length (priv->path);
  
  if (priv->position >= length - 1)
    return;
  
  priv->position = priv->position + 1;
  
  node = g_list_nth_data (priv->path, priv->position);
  
  file_path = ctags_path_node_get_file_path (node);
  line_number = ctags_path_node_get_line_number (node);
  
  if (!codeslayer_select_document_by_file_path (priv->codeslayer, file_path, line_number))
    {
      clear_path (engine);    
    }
}

static void
clear_path (CtagsEngine *engine)
{
  CtagsEnginePrivate *priv;
  priv = CTAGS_ENGINE_GET_PRIVATE (engine);
  g_list_foreach (priv->path, (GFunc) g_object_unref, NULL);
  g_list_free (priv->path);
  priv->path = NULL;
  priv->position = 0;
}
