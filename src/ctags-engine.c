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
static void find_tag_action                   (CodeSlayer         *codeslayer);
static void editor_saved_action               (CtagsEngine        *engine, 
                                               CodeSlayerEditor   *editor);
static gboolean start_create_tags             (CtagsEngine        *engine);
static void finish_create_tags                (CtagsEngine        *engine);
static void execute_create_tags               (CtagsEngine        *engine);
                                                              
static GList *find_tags                       (CodeSlayer         *codeslayer, 
                                               const char *const   name, 
                                               const int           options);
static gboolean search_active_editor          (CodeSlayer         *codeslayer, 
                                               CodeSlayerEditor   *editor, 
                                               GList              *tags);
static gboolean search_projects               (CodeSlayer         *codeslayer, 
                                               GList              *tags, 
                                               gboolean            search_headers);
static void select_editor                     (CodeSlayer         *codeslayer, 
                                               Tag                *tag);                                                              
                                                   
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
}

static void
ctags_engine_finalize (CtagsEngine *engine)
{
  CtagsEnginePrivate *priv;
  priv = CTAGS_ENGINE_GET_PRIVATE (engine);
    
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
                            G_CALLBACK (find_tag_action), codeslayer);

  priv->properties_opened_id =  g_signal_connect_swapped (G_OBJECT (codeslayer), "project-properties-opened",
                                                          G_CALLBACK (project_properties_opened_action), engine);

  priv->properties_saved_id = g_signal_connect_swapped (G_OBJECT (codeslayer), "project-properties-saved",
                                                        G_CALLBACK (project_properties_saved_action), engine);

  priv->saved_handler_id = g_signal_connect_swapped (G_OBJECT (codeslayer), "editor-saved", 
                                                     G_CALLBACK (editor_saved_action), engine);

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
  GKeyFile *keyfile;
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

  keyfile = codeslayer_utils_get_keyfile (file_path);
  source_folder = g_key_file_get_string (keyfile, MAIN, SOURCE_FOLDER, NULL);
  
  config = ctags_config_new ();
  ctags_config_set_project (config, project);
  ctags_config_set_source_folder (config, source_folder);
  
  g_free (folder_path);
  g_free (file_path);
  g_free (source_folder);
  g_key_file_free (keyfile);
  
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
  GKeyFile *keyfile;
 
  priv = CTAGS_ENGINE_GET_PRIVATE (engine);

  project = ctags_config_get_project (config);  
  folder_path = codeslayer_get_project_config_folder_path (priv->codeslayer, project);
  file_path = codeslayer_utils_get_file_path (folder_path, CTAGS_CONF);
  keyfile = codeslayer_utils_get_keyfile (file_path);

  source_folder = ctags_config_get_source_folder (config);
  g_key_file_set_string (keyfile, MAIN, SOURCE_FOLDER, source_folder);

  codeslayer_utils_save_keyfile (keyfile, file_path);  
  g_key_file_free (keyfile);
  g_free (folder_path);
  g_free (file_path);
  
  execute_create_tags (engine);
}

static void 
editor_saved_action (CtagsEngine      *engine, 
                     CodeSlayerEditor *editor) 
{
  execute_create_tags (engine);
}

/*
 * wait a second before we regenerate the tag file in case 
 * we are saving multiple editors at once.
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
  CodeSlayerGroup *group;
  GList *projects;
  GList *list;
  GString *string;
  gchar *command;
  gchar *group_file_path;

  priv = CTAGS_ENGINE_GET_PRIVATE (engine);
  
  group_file_path = codeslayer_get_group_config_folder_path (priv->codeslayer);
  
  string = g_string_new ("cd ");
  string = g_string_append (string, group_file_path);
  string = g_string_append (string, ";ctags -R --fields=n");
  
  group = codeslayer_get_group (priv->codeslayer);
  projects = codeslayer_group_get_projects (group);
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

  command = g_string_free (string, FALSE);
  file = popen (command, "r");
  
  if (file != NULL)
    pclose (file);
  
  g_free (command);
  g_free (group_file_path);
  
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
	gchar *group_file_path;
	gchar *tag_file_path;
	
  group_file_path = codeslayer_get_group_config_folder_path (codeslayer);
  tag_file_path = g_build_filename (group_file_path, "tags", NULL);
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
  
  g_free (group_file_path);
  g_free (tag_file_path);
  
  return results;
}

static void 
find_tag_action (CodeSlayer *codeslayer)
{
  CodeSlayerEditor *editor;
  GtkTextBuffer *buffer;
  GList *tags;
  GList *tmp;

  GtkTextMark *insert_mark;
  GtkTextMark *selection_mark;
  gchar *text;

  GtkTextIter start, end;

  editor = codeslayer_get_active_editor (codeslayer);
  
  if (editor == NULL)
    return;
  
  buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (editor));

  insert_mark = gtk_text_buffer_get_insert (buffer);    
  selection_mark = gtk_text_buffer_get_selection_bound (buffer);

  gtk_text_buffer_get_iter_at_mark (buffer, &start, insert_mark);
  gtk_text_buffer_get_iter_at_mark (buffer, &end, selection_mark);

  text = gtk_text_buffer_get_text (buffer, &start, &end, FALSE);
  
  if (text != NULL)
    g_strstrip (text);
  
  tags = find_tags (codeslayer, text, 0);
  tmp = tags;
  
  if (tmp != NULL)
    {
      gboolean found;
      found = search_active_editor (codeslayer, editor, tmp);
      if (!found)
        found = search_projects (codeslayer, tmp, FALSE);
      if (!found)
        search_projects (codeslayer, tmp, TRUE);
  
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
search_active_editor (CodeSlayer       *codeslayer, 
                      CodeSlayerEditor *editor, 
                      GList            *tags)
{
  CodeSlayerDocument *document;
	const gchar *document_file_path;
	
	document = codeslayer_editor_get_document (editor);
	document_file_path = codeslayer_document_get_file_path (document);

  while (tags != NULL)
    {
      Tag *tag = tags->data;
      if (g_strcmp0 (document_file_path, tag->file_path) == 0)
        {
          select_editor (codeslayer, tag);
          return TRUE;
        }
      tags = g_list_next (tags);
    }
    
  return FALSE;    
}

static gboolean
search_projects (CodeSlayer *codeslayer, 
                 GList      *tags, 
                 gboolean    search_headers)
{
  while (tags != NULL)
    {
      Tag *tag = tags->data;
      if (search_headers || !g_str_has_suffix (tag->file_path, ".h"))
        {
          select_editor (codeslayer, tag);
          return TRUE;
        }
      tags = g_list_next (tags);
    }
  
  return FALSE;    
}

static void
select_editor (CodeSlayer *codeslayer, 
               Tag        *tag)
{
  CodeSlayerEditor *editor;
  codeslayer_select_editor_by_file_path (codeslayer, tag->file_path, tag->line_number);
  editor = codeslayer_get_active_editor (codeslayer);
  g_signal_emit_by_name((gpointer) codeslayer, "editor-navigated", editor);
}

