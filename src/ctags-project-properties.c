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

static void source_directory_icon_action         (GtkEntry                    *source_directory_entry, 
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
  GtkWidget         *source_directory_entry;
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
    g_signal_new ("save-configuration", 
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
                  G_STRUCT_OFFSET (CtagsProjectPropertiesClass, save_configuration), 
                  NULL, NULL,
                  g_cclosure_marshal_VOID__OBJECT, G_TYPE_NONE, 1, CTAGS_CONFIGURATION_TYPE);

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

  GtkWidget *source_directory_label;
  GtkWidget *source_directory_entry;

  priv = CTAGS_PROJECT_PROPERTIES_GET_PRIVATE (project_properties);

  grid = gtk_grid_new ();
  gtk_grid_set_row_spacing (GTK_GRID (grid), 2);

  source_directory_label = gtk_label_new ("Source Directory:");
  gtk_misc_set_alignment (GTK_MISC (source_directory_label), 1, .5);
  gtk_misc_set_padding (GTK_MISC (source_directory_label), 4, 0);
  gtk_grid_attach (GTK_GRID (grid), source_directory_label, 0, 0, 1, 1);
  
  source_directory_entry = gtk_entry_new ();
  priv->source_directory_entry = source_directory_entry;
  gtk_entry_set_width_chars (GTK_ENTRY (source_directory_entry), 50);
  gtk_entry_set_icon_from_stock (GTK_ENTRY (source_directory_entry), 
                                 GTK_ENTRY_ICON_SECONDARY, GTK_STOCK_DIRECTORY);
  gtk_grid_attach_next_to (GTK_GRID (grid), source_directory_entry, source_directory_label, 
                           GTK_POS_RIGHT, 1, 1);
                      
  gtk_box_pack_start (GTK_BOX (project_properties), grid, FALSE, FALSE, 3);
  
  g_signal_connect (G_OBJECT (source_directory_entry), "icon-press",
                    G_CALLBACK (source_directory_icon_action), project_properties);
}

static void 
source_directory_icon_action (GtkEntry               *source_directory_entry, 
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
      gtk_entry_set_text (source_directory_entry, file_path);
      g_free (file_path);
      g_object_unref (file);
    }

  gtk_widget_destroy (GTK_WIDGET (dialog));  
}                        


void 
ctags_project_properties_opened (CtagsProjectProperties *project_properties,
                                 CtagsConfiguration     *configuration, 
                                 CodeSlayerProject      *project)
{
  CtagsProjectPropertiesPrivate *priv;

  priv = CTAGS_PROJECT_PROPERTIES_GET_PRIVATE (project_properties);
  priv->project = project;
  
  if (configuration)
    {
      const gchar *source_directory;
    
      source_directory = ctags_configuration_get_source_directory (configuration);

      gtk_entry_set_text (GTK_ENTRY (priv->source_directory_entry), source_directory);
    }
  else
    {
      gtk_entry_set_text (GTK_ENTRY (priv->source_directory_entry), "");
    }
}

void 
ctags_project_properties_saved (CtagsProjectProperties *project_properties,
                                CtagsConfiguration     *configuration,
                                CodeSlayerProject      *project)
{
  CtagsProjectPropertiesPrivate *priv;
  gchar *source_directory;
  
  priv = CTAGS_PROJECT_PROPERTIES_GET_PRIVATE (project_properties);
  
  source_directory = g_strdup (gtk_entry_get_text (GTK_ENTRY (priv->source_directory_entry)));
  g_strstrip (source_directory);
  
  if (configuration)
    {
      if (g_strcmp0 (source_directory, ctags_configuration_get_source_directory (configuration)) == 0)
        {
          g_free (source_directory);
          return;
        }
        
      ctags_configuration_set_source_directory (configuration, source_directory);
      g_signal_emit_by_name((gpointer)project_properties, "save-configuration", NULL);        
    }
  else if (entry_has_text (priv->source_directory_entry))
    {
      CtagsConfiguration *configuration;
      const gchar *project_key;
      configuration = ctags_configuration_new ();
      project_key = codeslayer_project_get_key (project);
      ctags_configuration_set_project_key (configuration, project_key);
      ctags_configuration_set_source_directory (configuration, source_directory);
      g_signal_emit_by_name((gpointer)project_properties, "save-configuration", configuration);        
    }
    
  g_free (source_directory);
}

static gboolean
entry_has_text (GtkWidget *entry)
{
  return gtk_entry_buffer_get_length (gtk_entry_get_buffer (GTK_ENTRY (entry))) > 0;
}
