GTK = `pkg-config --cflags --libs gtk+-2.0`

all: tbo

tbo: *.o tbo.c
	gcc tbo.c -o tbo *.o $(GTK)

ui-menu.o: ui-menu.c ui-menu.h
	gcc -c ui-menu.c $(GTK)

ui-toolbar.o: ui-toolbar.c ui-toolbar.h
	gcc -c ui-toolbar.c $(GTK)
