## Process this file with automake to generate a Makefile.in
## To build all programs with GTK+ uncomment these lines.

AM_CPPFLAGS = -I$(top_srcdir) -I$(includedir) $(GNOME_INCLUDEDIR) \
         -DG_LOG_DOMAIN=\"tbo\"

bin_PROGRAMS = tbo

SOURCES = tbo.c\
	  tbo-startup.h\
	  tbo-startup.c

AM_CFLAGS = @GTK_CFLAGS@ \
			 $(PACKAGE_CFLAGS) \
			-DGNOMELOCALEDIR=\"$(datadir)/locale\" \
			-DDATA_DIR=\""$(pkgdatadir)"\"

tbo_LDADD = @GTK_LIBS@ \
			$(PACKAGE_LIBS)

tbo_SOURCES = $(SOURCES)

CLEANFILES = *~
