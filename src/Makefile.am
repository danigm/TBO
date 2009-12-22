## Process this file with automake to generate a Makefile.in
## To build all programs with GTK+ uncomment these lines.
##AM_CPPFLAGS = @GTK_CFLAGS@
##AM_LDADD = @GTK_LIBS@

AM_CPPFLAGS = -I$(top_srcdir) -I$(includedir) $(GNOME_INCLUDEDIR) \
         -DG_LOG_DOMAIN=\"tbo\"

bin_PROGRAMS = tbo

tbo_SOURCES = \
	comic.c \
	comic-new-dialog.c \
	frame.c \
	page.c \
	tbo.c \
	ui-drawing.c \
	ui-menu.c \
	ui-toolbar.c \
	comic.h \
	comic-new-dialog.h \
	frame.h \
	page.h \
	tbo-types.h \
	ui-drawing.h \
	ui-menu.h \
	ui-toolbar.h

tbo_CFLAGS = @GTK_CFLAGS@ \
    		   -DDATA_DIR=\""$(pkgdatadir)"\"

tbo_LDADD = @GTK_LIBS@

CLEANFILES = *~
