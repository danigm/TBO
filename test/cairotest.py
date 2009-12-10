# -*- coding: utf-8 -*-
import pygtk
pygtk.require('2.0')
import gtk, gobject, cairo
import math
import rsvg
pi = math.pi

BLACK = (0.0,0.0,0.0)
WHITE = (1.0,1.0,1.0)

class TBObject:
    def __init__(self, x=0, y=0, w=0, h=0):
        self.width = w
        self.height = h
        self.x = x
        self.y = y

    def scale(self, w=0, h=0):
        self.width = w
        self.height = h

    def move(self, x, y):
        self.x = x
        self.y = y

class Text(TBObject):
    def __init__(self, text, color=BLACK, font="Sans", fontsize=12, **kwargs):
        TBObject.__init__(self, **kwargs)
        self.text = text.split("\n")
        self.color = color
        self.font = font
        self.fontsize = fontsize

    def draw(self, cr):
        x, y = self.x, self.y
        for line in self.text:
            cr.set_source_rgb(*self.color)
            cr.select_font_face(self.font, cairo.FONT_SLANT_NORMAL, cairo.FONT_WEIGHT_BOLD)
            cr.set_font_size(self.fontsize)
            x_bearing, y_bearing, width, height = cr.text_extents(line)[:4]
            cr.move_to(x - width / 2 - x_bearing, y - height / 2 - y_bearing)
            cr.show_text(line)
            y -= y_bearing - 5

class SVG(TBObject):
    def __init__(self, file, **kwargs):
        TBObject.__init__(self, **kwargs)
        self.file = file 
        self.svg = rsvg.Handle(file)

    def draw(self, cr):
        x, y = self.x, self.y
        ws, hs = self.scale_svg()
        cr.translate(x, y)
        cr.scale(ws, hs)
        self.svg.render_cairo(cr)
        cr.scale(1/ws,1/hs)
        cr.translate(-x, -y)

    def scale_svg(self):
        w, h = self.svg.props.width, self.svg.props.height
        if not self.width or not self.height:
            self.width, self.height = w, h

        w_scale = self.width / float(w)
        h_scale = self.height / float(h)
        return w_scale, h_scale

class Rectangle(TBObject):
    def __init__(self, color=BLACK, fill=WHITE, line_width=1, **kwargs):
        TBObject.__init__(self, **kwargs)
        self.color = color
        self.fill = fill
        self.line_width = line_width

    def draw(self, cr):
        cr.set_line_width(self.line_width)
        cr.set_source_rgb(*self.color)
        cr.rectangle(self.x, self.y, self.width, self.height)
        cr.stroke()
        cr.set_source_rgb(*self.fill)
        cr.rectangle(self.x, self.y, self.width, self.height)
        cr.fill()

class DArea(gtk.DrawingArea):
    def __init__(self):
        gtk.DrawingArea.__init__(self)
        self.connect("expose-event", self.expose)

        self.add_events(gtk.gdk.BUTTON_PRESS_MASK |
                        gtk.gdk.BUTTON1_MOTION_MASK)

        self.connect("expose_event", self.expose)
        self.connect("button_press_event", self.pressing)
        self.connect("motion_notify_event", self.moving)

    def expose(self, widget, event):
        self.context = self.window.cairo_create()

        self.context.rectangle(event.area.x, event.area.y,
                        event.area.width, event.area.height)
        self.context.clip()

        self.draw(self.context, *self.window.get_size())
    
    def draw(self, cr, width, height):
        r = Rectangle(x=10, y=10, w=width-20, h=height-20, line_width=10)
        r.draw(cr)

        # draw lines
        cr.set_source_rgb(0.0, 0.0, 0.8)
        cr.move_to(width / 3.0, height / 3.0)
        cr.rel_line_to(0, height / 6.0)
        cr.move_to(2 * width / 3.0, height / 3.0)
        cr.rel_line_to(0, height / 6.0)
        cr.stroke()

        # and a circle
        cr.set_source_rgb(1.0, 0.0, 0.0)
        radius = min(width, height)
        cr.arc(width / 2.0, height / 2.0, radius / 2.0 - 20, 0, 2 * pi)
        cr.stroke()
        cr.arc(width / 2.0, height / 2.0, radius / 3.0 - 10, pi / 3, 2 * pi / 3)
        cr.stroke()

        globo = SVG("globo.svg", x=width-300, y=40, w=200, h=120)
        globo.draw(cr)

        text = '''
este texto
está escrito
en varias
líneas
        '''

        tbotext = Text(text, x=width-200, y=60, fontsize=12, font="Kid Kosmic")
        tbotext.draw(cr)

    def pressing(self, widget, event):
        print "pressing", event.x, event.y

    def moving(self, widget, event):
        print "moving", event.x, event.y

def main():
    w = gtk.Window()
    w.set_title("TBO")
    w.set_default_size(800, 500)
    darea = DArea()
    w.add(darea)
    w.show_all()
    w.connect('destroy', gtk.main_quit)
    gtk.main()

if __name__ == '__main__':
    main()
