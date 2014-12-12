## Process this file with automake to produce Makefile.in

## Created by Anjuta


uidir = $(pkgdatadir)/ui
ui_DATA = elec590_ofs.ui


AM_CPPFLAGS = \
	-DPACKAGE_LOCALE_DIR=\""$(localedir)"\" \
	-DPACKAGE_SRC_DIR=\""$(srcdir)"\" \
	-DPACKAGE_DATA_DIR=\""$(pkgdatadir)"\" \
	$(ELEC590_OFS_CFLAGS)

AM_CFLAGS =\
	 -Wall\
	 -g

bin_PROGRAMS = elec590_ofs

elec590_ofs_SOURCES = \
	main.c

elec590_ofs_CFLAGS = -lssl \
	-lcrypto \
	`pkg-config \
	--cflags \
	--libs \
	libssh` 


elec590_ofs_LDFLAGS = \
	-Wl,--export-dynamic

elec590_ofs_LDADD = $(ELEC590_OFS_LIBS)




EXTRA_DIST = $(ui_DATA)

# Remove ui directory on uninstall
uninstall-local:
	-rm -r $(uidir)
	-rm -r $(pkgdatadir)

