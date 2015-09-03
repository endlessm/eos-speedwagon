
bin_PROGRAMS =
BUILT_SOURCES =
CLEANFILES =
DISTCLEANFILES =
EXTRA_DIST =

resource_files = $(shell $(GLIB_COMPILE_RESOURCES) --sourcedir=$(srcdir) --generate-dependencies $(srcdir)/speedwagon.gresource.xml)
src/speedwagon-resources.h: speedwagon.gresource.xml Makefile $(resource_files)
	$(AM_V_GEN)$(GLIB_COMPILE_RESOURCES) --target=$@ --sourcedir=$(srcdir) --generate-header --c-name eos_speedwagon $<
src/speedwagon-resources.c: speedwagon.gresource.xml Makefile $(resource_files)
	$(AM_V_GEN)$(GLIB_COMPILE_RESOURCES) --target=$@ --sourcedir=$(srcdir) --generate-source --c-name eos_speedwagon $<

EXTRA_DIST += \
	$(resource_files) \
	speedwagon.gresource.xml \
	$(NULL)
CLEANFILES += \
	src/speedwagon-resources.h \
	src/speedwagon-resources.c \
	$(NULL)
BUILT_SOURCES += \
	src/speedwagon-resources.h \
	src/speedwagon-resources.c \
	$(NULL)

src/speedwagon-service.h: Makefile $(top_srcdir)/data/com.endlessm.Speedwagon.xml
	$(AM_V_GEN) $(MKDIR_P) src && \
	$(GDBUS_CODEGEN) \
		--interface-prefix com.endlessm. \
		--generate-c-code src/speedwagon-service \
		--c-namespace Eos \
		$(top_srcdir)/data/com.endlessm.Speedwagon.xml
src/speedwagon-service.c: src/speedwagon-service.h

EXTRA_DIST += data/com.endlessm.Speedwagon.xml
CLEANFILES += \
	src/speedwagon-service.h \
	src/speedwagon-service.c \
	$(NULL)
BUILT_SOURCES += \
	src/speedwagon-service.h \
	src/speedwagon-service.c \
	$(NULL)

bin_PROGRAMS += eos-speedwagon
eos_speedwagon_SOURCES = \
	src/eos-speedwagon.c \
	$(BUILT_SOURCES) \
	$(NULL)
eos_speedwagon_CFLAGS = $(SPEEDWAGON_CFLAGS)
eos_speedwagon_LDADD = $(SPEEDWAGON_LIBS)

servicedir = $(datadir)/dbus-1/services
service_DATA = com.endlessm.Speedwagon.service

com.endlessm.Speedwagon.service: Makefile
	$(AM_V_GEN) \
	( echo "[D-BUS Service]" ; \
	  echo "Name=com.endlessm.Speedwagon" ; \
	  echo "Exec=${bindir}/eos-speedwagon" ) > $@.tmp && \
	mv $@.tmp $@
CLEANFILES += com.endlessm.Speedwagon.service
