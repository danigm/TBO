#include <cairo/cairo.h>
#include <cairo/cairo-pdf.h>

int main() {

    cairo_surface_t *surface;
    cairo_t *cr;

    surface = cairo_pdf_surface_create("pdffile.pdf", 504, 648);
    cr = cairo_create(surface);

    cairo_set_source_rgb(cr, 0, 0, 0);
    cairo_select_font_face (cr, "Sans", CAIRO_FONT_SLANT_NORMAL,
            CAIRO_FONT_WEIGHT_NORMAL);
    cairo_set_font_size (cr, 40.0);

    cairo_move_to(cr, 10.0, 50.0);
    cairo_show_text(cr, "Disziplin ist Macht.");

    cairo_show_page(cr);

    cairo_surface_destroy(surface);
    cairo_destroy(cr);

    return 0;
}

