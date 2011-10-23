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

#ifndef __CTAGS_ENGINE_H__
#define	__CTAGS_ENGINE_H__

#include <gtk/gtk.h>
#include <codeslayer/codeslayer.h>

G_BEGIN_DECLS

#define CTAGS_ENGINE_TYPE            (ctags_engine_get_type ())
#define CTAGS_ENGINE(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), CTAGS_ENGINE_TYPE, CtagsEngine))
#define CTAGS_ENGINE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), CTAGS_ENGINE_TYPE, CtagsEngineClass))
#define IS_CTAGS_ENGINE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), CTAGS_ENGINE_TYPE))
#define IS_CTAGS_ENGINE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), CTAGS_ENGINE_TYPE))

typedef struct _CtagsEngine CtagsEngine;
typedef struct _CtagsEngineClass CtagsEngineClass;

struct _CtagsEngine
{
  GObject parent_instance;
};

struct _CtagsEngineClass
{
  GObjectClass parent_class;
};

GType ctags_engine_get_type (void) G_GNUC_CONST;

CtagsEngine*  ctags_engine_new                  (CodeSlayer *codeslayer,
                                                 GtkWidget  *menu);
                                 
void          ctags_engine_load_configurations  (CtagsEngine *engine);                             

G_END_DECLS

#endif /* _CTAGS_ENGINE_H */
