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

#ifndef __CTAGS_PATH_NODE_H__
#define __CTAGS_PATH_NODE_H__

#include <gtk/gtk.h>
#include <codeslayer/codeslayer-project.h>

G_BEGIN_DECLS

#define CTAGS_PATH_NODE_TYPE            (ctags_path_node_get_type ())
#define CTAGS_PATH_NODE(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), CTAGS_PATH_NODE_TYPE, CtagsPathNode))
#define CTAGS_PATH_NODE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), CTAGS_PATH_NODE_TYPE, CtagsPathNodeClass))
#define IS_CTAGS_PATH_NODE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), CTAGS_PATH_NODE_TYPE))
#define IS_CTAGS_PATH_NODE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), CTAGS_PATH_NODE_TYPE))

typedef struct _CtagsPathNode CtagsPathNode;
typedef struct _CtagsPathNodeClass CtagsPathNodeClass;

struct _CtagsPathNode
{
  GObject parent_instance;
};

struct _CtagsPathNodeClass
{
  GObjectClass parent_class;
};

GType ctags_path_node_get_type (void) G_GNUC_CONST;

CtagsPathNode*  ctags_path_node_new (void);

const gchar*     ctags_path_node_get_file_path    (CtagsPathNode *node);
void             ctags_path_node_set_file_path    (CtagsPathNode *node, 
                                                   const gchar    *file_path);
const gint       ctags_path_node_get_line_number  (CtagsPathNode *node);
void             ctags_path_node_set_line_number  (CtagsPathNode *node, 
                                                   const gint      line_number);

gboolean         ctags_path_node_equals           (CtagsPathNode *node, 
                                                   CtagsPathNode *that);

G_END_DECLS

#endif /* __CTAGS_PATH_NODE_H__ */
