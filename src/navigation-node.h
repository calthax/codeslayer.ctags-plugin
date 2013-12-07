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

#ifndef __NAVIGATION_NODE_H__
#define __NAVIGATION_NODE_H__

#include <gtk/gtk.h>
#include <codeslayer/codeslayer-project.h>

G_BEGIN_DECLS

#define NAVIGATION_NODE_TYPE            (navigation_node_get_type ())
#define NAVIGATION_NODE(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), NAVIGATION_NODE_TYPE, NavigationNode))
#define NAVIGATION_NODE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), NAVIGATION_NODE_TYPE, NavigationNodeClass))
#define IS_NAVIGATION_NODE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), NAVIGATION_NODE_TYPE))
#define IS_NAVIGATION_NODE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), NAVIGATION_NODE_TYPE))

typedef struct _NavigationNode NavigationNode;
typedef struct _NavigationNodeClass NavigationNodeClass;

struct _NavigationNode
{
  GObject parent_instance;
};

struct _NavigationNodeClass
{
  GObjectClass parent_class;
};

GType navigation_node_get_type (void) G_GNUC_CONST;

NavigationNode*  navigation_node_new (void);

const gchar*     navigation_node_get_file_path    (NavigationNode *node);
void             navigation_node_set_file_path    (NavigationNode *node, 
                                                   const gchar    *file_path);
const gint       navigation_node_get_line_number  (NavigationNode *node);
void             navigation_node_set_line_number  (NavigationNode *node, 
                                                   const gint      line_number);

gboolean         navigation_node_equals           (NavigationNode *node, 
                                                   NavigationNode *that);

G_END_DECLS

#endif /* __NAVIGATION_NODE_H__ */
