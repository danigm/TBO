#include <stdio.h>
#include "tbo-object-base.h"
#include "tbo-object-svg.h"

void
print_tbo_object (TboObjectBase *obj)
{
    printf ("obj:\n x, y: (%d, %d)\n w, h: (%d, %d)\n",
            obj->x, obj->y, obj->width, obj->height);
}

int
main (int argc, char **argv)
{
    g_type_init ();

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

    return 0;
}
