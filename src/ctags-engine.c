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
#include "readtags.h"

typedef struct
{
  gchar         *file_path;
  unsigned long  line_number;
} Tag;


static void ctags_engine_class_init                          (CtagsEngineClass   *klass);
static void ctags_engine_init                                (CtagsEngine        *engine);
static void ctags_engine_finalize                            (CtagsEngine        *engine);

static void find_tag_action                                  (CodeSlayer         *codeslayer);
static void editor_saved_action                              (CtagsEngine        *engine, 
                                                              CodeSlayerEditor   *editor);
static gboolean start_create_tags                            (CtagsEngine        *engine);
static void finish_create_tags                               (CtagsEngine        *engine);
static void execute_create_tags                              (CtagsEngine        *engine);
                                                              
static GList *find_tags                                      (CodeSlayer         *codeslayer, 
                                                              const char *const   name, 
                                                              const int           options);
static gboolean search_active_editor                         (CodeSlayer         *codeslayer, 
                                                              CodeSlayerEditor   *editor, 
                                                              GList              *tags);
static gboolean search_projects                              (CodeSlayer         *codeslayer, 
                                                              GList              *tags, 
                                                              gboolean            search_headers);
static void select_editor                                    (CodeSlayer         *codeslayer, 
                                                              Tag                *tag);                                                              
                                                   
#define CTAGS_ENGINE_GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), CTAGS_ENGINE_TYPE, CtagsEnginePrivate))

typedef struct _CtagsEnginePrivate CtagsEnginePrivate;

struct _CtagsEnginePrivate
{
  CodeSlayer *codeslayer;
  GtkWidget  *menu;
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
ctags_engine_init (CtagsEngine *engine) {}

static void
ctags_engine_finalize (CtagsEngine *engine)
{
  G_OBJECT_CLASS (ctags_engine_parent_class)->finalize (G_OBJECT(engine));
}

CtagsEngine*
ctags_engine_new (CodeSlayer *codeslayer,
                  GtkWidget  *menu)
{
  CtagsEnginePrivate *priv;
  CtagsEngine *engine;

  engine = CTAGS_ENGINE (g_object_new (ctags_engine_get_type (), NULL));
  priv = CTAGS_ENGINE_GET_PRIVATE (engine);

  priv->codeslayer = codeslayer;
  priv->menu = menu;
  priv->event_source_id = 0;
  
  priv->saved_handler_id = g_signal_connect_swapped (G_OBJECT (codeslayer), "editor-saved", 
                                                     G_CALLBACK (editor_saved_action), engine);  
  
  g_signal_connect_swapped (G_OBJECT (menu), "find-tag",
                            G_CALLBACK (find_tag_action), codeslayer);

  return engine;
}

static void 
editor_saved_action (CtagsEngine      *engine, 
                     CodeSlayerEditor *editor) 
{
  CodeSlayerDocument *document;
  CodeSlayerProject *project;
  const gchar *source_folders_path;

  document = codeslayer_editor_get_document (editor);
  project = codeslayer_document_get_project (document);
  source_folders_path = codeslayer_project_get_source_folders_path (project);
  
  if (codeslayer_utils_has_text (source_folders_path))
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
  gchar *group_folder_path;
  GList *projects;
  GString *string;
  gchar *command;

  priv = CTAGS_ENGINE_GET_PRIVATE (engine);
  
  group = codeslayer_get_active_group (priv->codeslayer);
  group_folder_path = codeslayer_get_active_group_folder_path (priv->codeslayer);
  
  projects = codeslayer_group_get_projects (group);
  
  string = g_string_new ("cd ");
  string = g_string_append (string, group_folder_path);
  string = g_string_append (string, ";ctags -R --fields=n");
  
  while (projects != NULL)
    {
      CodeSlayerProject *project = projects->data;
      const gchar *source_folders_path;
      source_folders_path = codeslayer_project_get_source_folders_path (project);
      
      if (codeslayer_utils_has_text (source_folders_path))
        {
          gchar *source_folders_replace;
          source_folders_replace = codeslayer_utils_strreplace (source_folders_path, ",", " ");
          string = g_string_append (string, " ");
          string = g_string_append (string, source_folders_replace);
          g_free (source_folders_replace);
        }
      
      projects = g_list_next (projects);
    }

  command = g_string_free (string, FALSE);
  file = popen (command, "r");
  
  if (file != NULL)
    pclose (file);
  
  g_free (command);
  g_free (group_folder_path);
  
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
	gchar *group_folder_path;
	gchar *tag_file_path;
	
  group_folder_path = codeslayer_get_active_group_folder_path (codeslayer);
  tag_file_path = g_build_filename (group_folder_path, "tags", NULL);
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
  
  g_free (group_folder_path);
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
  CodeSlayerDocument *document;
  CodeSlayerProject *project;

  document = codeslayer_document_new ();
  project = codeslayer_get_project_by_file_path (codeslayer, tag->file_path);

  codeslayer_document_set_file_path (document, tag->file_path);
  codeslayer_document_set_line_number (document, tag->line_number);
  codeslayer_document_set_project (document, project);

  codeslayer_select_editor (codeslayer, document);
  
  g_object_unref (document);
}

