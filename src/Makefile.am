## Process this file with automake to generate a Makefile.in
## To build all programs with GTK+ uncomment these lines.

AM_CPPFLAGS = -I$(top_srcdir) -I$(includedir) $(GNOME_INCLUDEDIR) \
         -DG_LOG_DOMAIN=\"tbo\"

bin_PROGRAMS = tbo
noinst_PROGRAMS = typestest

SOURCES = \
	tbo-window.c \
	comic.c \
	comic-new-dialog.c \
	comic-saveas-dialog.c \
	comic-open-dialog.c \
	frame.c \
	page.c \
	ui-menu.c \
	custom-stock.c \
	doodle-treeview.c \
	tbo-tooltip.c \
	export.c \
	dnd.c \
	comic-load.c \
	tbo-utils.c \
	tbo-files.c \
	tbo-ui-utils.c \
	tbo-files.h \
	tbo-window.h \
	comic.h \
	comic-new-dialog.h \
	comic-saveas-dialog.h \
	comic-open-dialog.h \
	frame.h \
	page.h \
	tbo-types.h \
	ui-menu.h \
	doodle-treeview.h \
	tbo-tooltip.h \
	dnd.h \
	comic-load.h \
	tbo-utils.h \
	export.h \
	tbo-ui-utils.h \
	tbo-object-base.h \
	tbo-object-base.c \
	tbo-object-svg.h \
	tbo-object-svg.c \
	tbo-object-text.h \
	tbo-object-text.c \
	tbo-object-pixmap.h \
	tbo-object-pixmap.c \
	tbo-tool-base.h \
	tbo-tool-base.c \
	tbo-tool-selector.h \
	tbo-tool-selector.c \
	tbo-tool-frame.h \
	tbo-tool-frame.c \
	tbo-tool-doodle.h \
	tbo-tool-doodle.c \
	tbo-tool-bubble.h \
	tbo-tool-bubble.c \
	tbo-tool-text.h \
	tbo-tool-text.c \
	tbo-toolbar.h \
	tbo-toolbar.c \
	tbo-drawing.h \
	tbo-drawing.c \
	custom-stock.h

AM_CFLAGS = @GTK_CFLAGS@ \
			 $(PACKAGE_CFLAGS) \
			-DGNOMELOCALEDIR=\"$(datadir)/locale\" \
			-DDATA_DIR=\""$(pkgdatadir)"\"
tbo_LDADD = @GTK_LIBS@ \
			$(PACKAGE_LIBS)
typestest_LDADD = @GTK_LIBS@ \
				$(PACKAGE_LIBS)

typestest_SOURCES = $(SOURCES) typestest.c
tbo_SOURCES = $(SOURCES) tbo.c

CLEANFILES = *~
