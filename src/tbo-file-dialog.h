/*
 * This file is part of TBO, a gnome comic editor
 * Copyright (C) 2010  Daniel Garcia Moreno <dani@danigm.net>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */

#ifndef __TBO_FILE_DIALOG_H__
#define __TBO_FILE_DIALOG_H__

#include <gtk/gtk.h>
#include "tbo-window.h"

gchar *tbo_file_dialog_open_project (TboWindow *window);
gchar *tbo_file_dialog_save_project (TboWindow *window, const gchar *suggested_name);
gchar *tbo_file_dialog_open_image (TboWindow *window);
gchar *tbo_file_dialog_save_export (TboWindow *window, const gchar *current_text);

#endif
