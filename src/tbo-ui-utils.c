/*
 * This file is part of TBO, a gnome comic editor
 * Copyright (C) 2010  Daniel Garcia Moreno <dani@danigm.net>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#include <gtk/gtk.h>
#include "tbo-widget.h"
#include "tbo-ui-utils.h"

GtkWidget *
add_spin_with_label (GtkWidget *container, const gchar *string, gint value)
{
        GtkWidget *label;
        GtkWidget *spin;
        GtkAdjustment *adjustment;
        GtkWidget *hpanel;

        hpanel = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 6);
        label = gtk_label_new (string);
        gtk_label_set_xalign (GTK_LABEL (label), 0.0);
        gtk_label_set_yalign (GTK_LABEL (label), 0.5);
        adjustment = gtk_adjustment_new (value, 0, 10000, 1, 1, 0);
        spin = gtk_spin_button_new (GTK_ADJUSTMENT (adjustment), 1, 0);
        tbo_box_pack_start (hpanel, label, TRUE, TRUE, 5);
        tbo_box_pack_start (hpanel, spin, FALSE, FALSE, 5);
        tbo_box_pack_start (container, hpanel, FALSE, FALSE, 5);

        return spin;
}

GtkWidget *
tbo_font_picker_new (void)
{
#if GTK_CHECK_VERSION(4, 10, 0)
        GtkFontDialog *dialog = gtk_font_dialog_new ();
        GtkWidget *picker = gtk_font_dialog_button_new (dialog);

        gtk_font_dialog_button_set_use_size (GTK_FONT_DIALOG_BUTTON (picker), FALSE);
#else
        GtkWidget *picker = gtk_font_button_new ();

        gtk_font_button_set_use_size (GTK_FONT_BUTTON (picker), FALSE);
#endif

        return picker;
}

PangoFontDescription *
tbo_font_picker_dup_font_desc (GtkWidget *picker)
{
#if GTK_CHECK_VERSION(4, 10, 0)
        const PangoFontDescription *font = gtk_font_dialog_button_get_font_desc (GTK_FONT_DIALOG_BUTTON (picker));

        if (font == NULL)
                return NULL;

        return pango_font_description_copy (font);
#else
        return gtk_font_chooser_get_font_desc (GTK_FONT_CHOOSER (picker));
#endif
}

void
tbo_font_picker_set_font_desc (GtkWidget *picker, const PangoFontDescription *description)
{
#if GTK_CHECK_VERSION(4, 10, 0)
        gtk_font_dialog_button_set_font_desc (GTK_FONT_DIALOG_BUTTON (picker), description);
#else
        gtk_font_chooser_set_font_desc (GTK_FONT_CHOOSER (picker), description);
#endif
}

GtkWidget *
tbo_color_picker_new (const GdkRGBA *rgba)
{
#if GTK_CHECK_VERSION(4, 10, 0)
        GtkColorDialog *dialog = gtk_color_dialog_new ();
        GtkWidget *picker = gtk_color_dialog_button_new (dialog);

#else
        GtkWidget *picker = gtk_color_button_new ();
#endif

        tbo_color_picker_set_rgba (picker, rgba);
        return picker;
}

GdkRGBA
tbo_color_picker_get_rgba (GtkWidget *picker)
{
        GdkRGBA color = { 0, 0, 0, 1 };

#if GTK_CHECK_VERSION(4, 10, 0)
        const GdkRGBA *selected = gtk_color_dialog_button_get_rgba (GTK_COLOR_DIALOG_BUTTON (picker));

        if (selected != NULL)
                color = *selected;
#else
        gtk_color_chooser_get_rgba (GTK_COLOR_CHOOSER (picker), &color);
#endif

        return color;
}

void
tbo_color_picker_set_rgba (GtkWidget *picker, const GdkRGBA *rgba)
{
#if GTK_CHECK_VERSION(4, 10, 0)
        gtk_color_dialog_button_set_rgba (GTK_COLOR_DIALOG_BUTTON (picker), rgba);
#else
        gtk_color_chooser_set_rgba (GTK_COLOR_CHOOSER (picker), rgba);
#endif
}

void
tbo_picture_set_contain (GtkPicture *picture)
{
#if GTK_CHECK_VERSION(4, 8, 0)
        gtk_picture_set_content_fit (picture, GTK_CONTENT_FIT_CONTAIN);
#else
        gtk_picture_set_keep_aspect_ratio (picture, TRUE);
#endif
}
