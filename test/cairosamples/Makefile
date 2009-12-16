GTK = `pkg-config --cflags --libs gtk+-2.0`

topng: topng.c
	gcc topng.c -o topng $(GTK)

topdf: topdf.c
	gcc topdf.c -o topdf $(GTK)

togtk: 
	gcc togtk.c -o togtk $(GTK)

