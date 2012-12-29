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

#include "ctags-project-properties.h"

static void ctags_project_properties_class_init  (CtagsProjectPropertiesClass *klass);
static void ctags_project_properties_init        (CtagsProjectProperties      *project_properties);
static void ctags_project_properties_finalize    (CtagsProjectProperties      *project_properties);

static void add_form                             (CtagsProjectProperties      *project_properties);

static void source_folder_icon_action            (GtkEntry                    *source_folder_entry, 
                                                  GtkEntryIconPosition         icon_pos, 
                                                  GdkEvent                    *event,
                                                  CtagsProjectProperties      *project_properties);
static gboolean entry_has_text                   (GtkWidget                   *entry);

#define CTAGS_PROJECT_PROPERTIES_GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), CTAGS_PROJECT_PROPERTIES_TYPE, CtagsProjectPropertiesPrivate))

typedef struct _CtagsProjectPropertiesPrivate CtagsProjectPropertiesPrivate;

struct _CtagsProjectPropertiesPrivate
{
  CodeSlayerProject *project;
  GtkWidget         *source_folder_entry;
};

enum
{
  SAVE_CONFIGURATION,
  LAST_SIGNAL
};

static guint ctags_project_properties_signals[LAST_SIGNAL] = { 0 };

G_DEFINE_TYPE (CtagsProjectProperties, ctags_project_properties, GTK_TYPE_VBOX)

static void
ctags_project_properties_class_init (CtagsProjectPropertiesClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

  ctags_project_properties_signals[SAVE_CONFIGURATION] =
    g_signal_new ("save-config", 
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
                  G_STRUCT_OFFSET (CtagsProjectPropertiesClass, save_config), 
                  NULL, NULL,
                  g_cclosure_marshal_VOID__OBJECT, G_TYPE_NONE, 1, CTAGS_CONFIG_TYPE);

  gobject_class->finalize = (GObjectFinalizeFunc) ctags_project_properties_finalize;
  g_type_class_add_private (klass, sizeof (CtagsProjectPropertiesPrivate));
}

static void
ctags_project_properties_init (CtagsProjectProperties *project_properties) {}

static void
ctags_project_properties_finalize (CtagsProjectProperties *project_properties)
{
  G_OBJECT_CLASS (ctags_project_properties_parent_class)->finalize (G_OBJECT(project_properties));
}

GtkWidget*
ctags_project_properties_new (void)
{
  GtkWidget *project_properties;
  project_properties = g_object_new (ctags_project_properties_get_type (), NULL);
  add_form (CTAGS_PROJECT_PROPERTIES (project_properties));
  return project_properties;
}

static void 
add_form (CtagsProjectProperties *project_properties)
{
  CtagsProjectPropertiesPrivate *priv;
  GtkWidget *grid;

  GtkWidget *source_folder_label;
  GtkWidget *source_folder_entry;

  priv = CTAGS_PROJECT_PROPERTIES_GET_PRIVATE (project_properties);

  grid = gtk_grid_new ();
  gtk_grid_set_row_spacing (GTK_GRID (grid), 2);

  source_folder_label = gtk_label_new ("Source Folder:");
  gtk_misc_set_alignment (GTK_MISC (source_folder_label), 1, .5);
  gtk_misc_set_padding (GTK_MISC (source_folder_label), 4, 0);
  gtk_grid_attach (GTK_GRID (grid), source_folder_label, 0, 0, 1, 1);
  
  source_folder_entry = gtk_entry_new ();
  priv->source_folder_entry = source_folder_entry;
  gtk_entry_set_width_chars (GTK_ENTRY (source_folder_entry), 50);
  gtk_entry_set_icon_from_stock (GTK_ENTRY (source_folder_entry), 
                                 GTK_ENTRY_ICON_SECONDARY, GTK_STOCK_DIRECTORY);
  gtk_grid_attach_next_to (GTK_GRID (grid), source_folder_entry, source_folder_label, 
                           GTK_POS_RIGHT, 1, 1);
                      
  gtk_box_pack_start (GTK_BOX (project_properties), grid, FALSE, FALSE, 3);
  
  g_signal_connect (G_OBJECT (source_folder_entry), "icon-press",
                    G_CALLBACK (source_folder_icon_action), project_properties);
}

static void 
source_folder_icon_action (GtkEntry               *source_folder_entry, 
                           GtkEntryIconPosition    icon_pos, 
                           GdkEvent               *event,
                           CtagsProjectProperties *project_properties)
{
  CtagsProjectPropertiesPrivate *priv;
  GtkWidget *dialog;
  gint response;
  const gchar *folder_path;
  
  priv = CTAGS_PROJECT_PROPERTIES_GET_PRIVATE (project_properties);

  dialog = gtk_file_chooser_dialog_new ("Select Source Directory", 
                                        NULL,
                                        GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
                                        GTK_STOCK_CANCEL,
                                        GTK_RESPONSE_CANCEL,
                                        GTK_STOCK_OPEN,
                                        GTK_RESPONSE_OK, 
                                        NULL);

  gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_OK);
  
  folder_path = codeslayer_project_get_folder_path (priv->project);
  gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER(dialog), folder_path);

  response = gtk_dialog_run (GTK_DIALOG (dialog));
  if (response == GTK_RESPONSE_OK)
    {
      GFile *file;
      char *file_path;
      file = gtk_file_chooser_get_file (GTK_FILE_CHOOSER (dialog));
      file_path = g_file_get_path (file);
      gtk_entry_set_text (source_folder_entry, file_path);
      g_free (file_path);
      g_object_unref (file);
    }

  gtk_widget_destroy (GTK_WIDGET (dialog));  
}                        


void 
ctags_project_properties_opened (CtagsProjectProperties *project_properties,
                                 CtagsConfig            *config, 
                                 CodeSlayerProject      *project)
{
  CtagsProjectPropertiesPrivate *priv;

  priv = CTAGS_PROJECT_PROPERTIES_GET_PRIVATE (project_properties);
  priv->project = project;
  
  if (config != NULL)
    {
      const gchar *source_folder;    
      source_folder = ctags_config_get_source_folder (config);
      if (source_folder != NULL)
        gtk_entry_set_text (GTK_ENTRY (priv->source_folder_entry), source_folder);
    }
  else
    {
      gtk_entry_set_text (GTK_ENTRY (priv->source_folder_entry), "");
    }
}

void 
ctags_project_properties_saved (CtagsProjectProperties *project_properties,
                                CtagsConfig            *config,
                                CodeSlayerProject      *project)
{
  CtagsProjectPropertiesPrivate *priv;
  gchar *source_folder;
  
  priv = CTAGS_PROJECT_PROPERTIES_GET_PRIVATE (project_properties);
  
  source_folder = g_strdup (gtk_entry_get_text (GTK_ENTRY (priv->source_folder_entry)));
  g_strstrip (source_folder);
  
  if (config != NULL)
    {
      if (g_strcmp0 (source_folder, ctags_config_get_source_folder (config)) == 0)
        {
          g_free (source_folder);
          return;
        }
        
      ctags_config_set_source_folder (config, source_folder);
      ctags_config_set_project (config, project);
      g_signal_emit_by_name((gpointer)project_properties, "save-config", config);        
    }
  else if (entry_has_text (priv->source_folder_entry))
    {
      CtagsConfig *config;
      config = ctags_config_new ();
      ctags_config_set_source_folder (config, source_folder);
      ctags_config_set_project (config, project);
      g_signal_emit_by_name((gpointer)project_properties, "save-config", config);
      g_object_unref (config);      
    }
    
  g_free (source_folder);
}

static gboolean
entry_has_text (GtkWidget *entry)
{
  return gtk_entry_buffer_get_length (gtk_entry_get_buffer (GTK_ENTRY (entry))) > 0;
}
