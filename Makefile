
MKDIR_P = mkdir -p

CLEANFILES =
PKGS = gtk+-3.0
CFLAGS = $(shell pkg-config --cflags $(PKGS)) -Wall -Werror
LDFLAGS = $(shell pkg-config --libs $(PKGS))

FILES = src/lib/speedwagon-spinner.c src/lib/speedwagon-spinner.h

all: libspeedwagon.so Speedwagon-1.0.typelib speedwagon.gresource

INTROSPECTION_GIRS = Speedwagon-1.0.gir
INTROSPECTION_SCANNER_ARGS = --warn-all --warn-error --no-libtool

Speedwagon-1.0.gir: libspeedwagon.so
Speedwagon_1_0_gir_INCLUDES = Gtk-3.0
Speedwagon_1_0_gir_CFLAGS = $(CFLAGS)
Speedwagon_1_0_gir_LIBS = speedwagon
Speedwagon_1_0_gir_FILES = $(FILES)
CLEANFILES += Speedwagon-1.0.gir Speedwagon-1.0.typelib

include Makefile.introspection

libspeedwagon.so: CFLAGS += -fPIC -shared
libspeedwagon.so: src/lib/speedwagon-spinner.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)
CLEANFILES += libspeedwagon.so speedwagon-spinner.o

resource_files = $(shell glib-compile-resources --generate-dependencies speedwagon.gresource.xml)
speedwagon.gresource: speedwagon.gresource.xml $(resource_files)
	glib-compile-resources --target=$@ $<
CLEANFILES += speedwagon.gresource

clean:
	rm -f $(CLEANFILES)
