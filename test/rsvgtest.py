# -*- coding: utf-8 -*-
import cairo, gtk, rsvg, sys

class myApp:
    def __init__(self, filename):
        mw = gtk.Window(gtk.WINDOW_TOPLEVEL)
        mw.connect("delete_event", gtk.main_quit)

        svg = rsvg.Handle(filename)
        
        da = gtk.DrawingArea()
        da.set_size_request(svg.props.width, svg.props.height)
        da.connect("expose_event", self.expose, svg)

        mw.add(da)
        mw.show_all()


    def expose(self, da, event, svg):
        ctx = da.window.cairo_create()
        ctx.scale(2, 2)
        svg.render_cairo(ctx)


if __name__ == "__main__":
    if len(sys.argv) != 2:
        print "Uso: %s fichero.svg" % sys.argv[0]
    else:
        try:
            app = myApp(sys.argv[1])
            gtk.main()
        except KeyboardInterrupt:
            pass

