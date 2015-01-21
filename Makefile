
CLEANFILES =

all: speedwagon.gresource

resource_files = $(shell glib-compile-resources --generate-dependencies speedwagon.gresource.xml)
speedwagon.gresource: speedwagon.gresource.xml $(resource_files)
	glib-compile-resources --target=$@ $<
CLEANFILES += speedwagon.gresource

clean:
	rm -f $(CLEANFILES)
