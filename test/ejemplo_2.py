# -*- coding: utf-8 -*-
import gtk
import gtk.gdk
import math

class SemiCirculo(gtk.DrawingArea):
	def __init__(self):
		gtk.DrawingArea.__init__(self)
		self.connect("expose-event", self.expose)
	
	def expose(self, widget, event):
		#Creamos un contexto de dibujo cairo
		self.context = widget.window.cairo_create()
		
		#Ajustamos el tamaño del contexto al del widget
		self.context.rectangle(event.area.x, event.area.y,
				event.area.width, event.area.height)
		self.context.clip()
	
		#Llamamos a la función de dibujado
		self.draw(self.context)
		return False	

	def draw(self, context):
		#Adquirimos las coordenadas de origen
		#y el tamaño del rectangulo del widget,
		#situando en las variable x e y
		#el centro del rectangulo.
		rect = self.get_allocation()
		x = rect.x + rect.width / 2
		y = rect.y + rect.height / 2

		#hallamos el radio
		radius = min(rect.width / 2, rect.height / 2) - 5

		#Dibujamos un arco
		context.arc(x, y, radius, 0,(1 * math.pi))

		#Elegimos el color de relleno y lo vertemos
		context.set_source_rgb(0.7, 0.8, 0.1)
		context.fill_preserve()

		#Elegimos el color del borde y lo dibujamos
		context.set_source_rgb(0, 0, 0)
		context.stroke()

def main():
	window = gtk.Window()
	semicirculo = SemiCirculo()

	# Añadimos nuestro widget a la ventana
	window.add(semicirculo) 	
	# Conectamos el evento destroy con la salida del bucle de eventos
	window.connect("destroy", gtk.main_quit)
	# Dibujamos toda la ventana
	window.show_all()

	# Comenzamos el bucle de eventos
	gtk.main()

if __name__ == "__main__":
	main()
