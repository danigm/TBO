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
	ui-drawing.c \
	ui-menu.c \
	ui-toolbar.c \
	frame-tool.c \
	selector-tool.c \
	doodle-tool.c \
	custom-stock.c \
	doodle-treeview.c \
	tbo-tooltip.c \
	text-tool.c \
	textobj.c \
	export.c \
	svgimage.c \
	piximage.c \
	dnd.c \
	tbo-object.c \
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
	ui-drawing.h \
	ui-menu.h \
	ui-toolbar.h \
	frame-tool.h \
	selector-tool.h \
	doodle-tool.h \
	doodle-treeview.h \
	text-tool.h \
	svgimage.h \
	piximage.h \
	textobj.h \
	tbo-tooltip.h \
	dnd.h \
	tbo-object.h \
	comic-load.h \
	tbo-utils.h \
	export.h \
	tbo-ui-utils.h \
	tbo-object-base.h \
	tbo-object-base.c \
	tbo-object-svg.h \
	tbo-object-svg.c \
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
