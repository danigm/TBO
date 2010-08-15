#include <stdio.h>
#include "tbo-object-base.h"
#include "tbo-object-svg.h"
#include "tbo-object-text.h"
#include "tbo-object-pixmap.h"

void
print_tbo_object (TboObjectBase *obj)
{
    printf ("obj:\n x, y: (%d, %d)\n w, h: (%d, %d)\nangle: %f\n",
            obj->x, obj->y, obj->width, obj->height, obj->angle);
}

void
test_object_svg ()
{
    /* simple svg object */
    TboObjectSvg *svg = TBO_OBJECT_SVG (tbo_object_svg_new ());

    print_tbo_object (TBO_OBJECT_BASE (svg));
    printf ("path: '%s'\n", svg->path->str);

    g_object_unref (svg);

    /* svg object with params */
    svg = TBO_OBJECT_SVG (tbo_object_svg_new_with_params (100, 200,
                                150, 300, "/path/to/svgfile.svg"));

    print_tbo_object (TBO_OBJECT_BASE (svg));
    printf ("path: '%s'\n", svg->path->str);

    g_object_unref (svg);
}

void
test_object_text ()
{
    /* text object with params */
    TboObjectText *text;
    GdkColor color = { 0, 0xffff, 0xffff, 0xffff };
    text = TBO_OBJECT_TEXT (tbo_object_text_new_with_params (100, 200,
                                    150, 300, "text", "", &color));

    print_tbo_object (TBO_OBJECT_BASE (text));
    printf ("text: '%s'\n", text->text->str);
    printf ("color: '%d, %d, %d'\n", text->font_color->red,
                                     text->font_color->green,
                                     text->font_color->blue);

    g_object_unref (text);
}

void
test_object_pixmap ()
{
    TboObjectPixmap *pixmap = TBO_OBJECT_PIXMAP (tbo_object_pixmap_new ());

    /* pixmap object with params */
    pixmap = TBO_OBJECT_PIXMAP (tbo_object_pixmap_new_with_params (100, 200,
                                150, 300, "/path/to/pngfile.png"));

    print_tbo_object (TBO_OBJECT_BASE (pixmap));
    printf ("path: '%s'\n", pixmap->path->str);

    g_object_unref (pixmap);
}

int
main (int argc, char **argv)
{
    g_type_init ();

    printf ("\nobject svg\n---------------\n");
    test_object_svg ();
    printf ("\nobject text\n--------------\n");
    test_object_text ();
    printf ("\nobject pixmap\n--------------\n");
    test_object_pixmap ();

    return 0;
}
