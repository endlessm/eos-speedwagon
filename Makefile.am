
bin_SCRIPTS =
CLEANFILES =
DISTCLEANFILES =
EXTRA_DIST =

resource_files = $(shell $(GLIB_COMPILE_RESOURCES) --sourcedir=$(srcdir) --generate-dependencies $(srcdir)/speedwagon.gresource.xml)
speedwagon.gresource: speedwagon.gresource.xml $(resource_files)
	$(AM_V_GEN)$(GLIB_COMPILE_RESOURCES) --target=$@ --sourcedir=$(srcdir)  $<
CLEANFILES += speedwagon.gresource

do_subst = sed \
	-e 's|@bindir[@]|$(bindir)|g' \
	-e 's|@pkglibdir[@]|$(pkglibdir)|g' \
	-e 's|@pkgdatadir[@]|$(pkgdatadir)|g' \
	-e 's|@GJS_CONSOLE[@]|$(GJS_CONSOLE)|g'

src/config.js: src/config.js.in Makefile
	$(AM_V_GEN) $(MKDIR_P) $(builddir)/src && \
	$(do_subst) $< > $@
CLEANFILES += src/config.js

js_sources = \
	src/config.js \
	src/eos_speedwagon.js \
	$(NULL)

nobase_pkgdata_DATA = \
	$(js_sources) \
	speedwagon.gresource \
	$(NULL)

eos-speedwagon: eos-speedwagon.in Makefile
	$(AM_V_GEN) $(do_subst) $< > $@
	chmod +x $@
CLEANFILES += eos-speedwagon
bin_SCRIPTS += eos-speedwagon

servicedir = $(datadir)/dbus-1/services
service_DATA = com.endlessm.Speedwagon.service

com.endlessm.Speedwagon.service: Makefile
	$(AM_V_GEN) \
	( echo "[D-BUS Service]" ; \
	  echo "Name=com.endlessm.Speedwagon" ; \
	  echo "Exec=${bindir}/eos-speedwagon" ) > $@.tmp && \
	mv $@.tmp $@
CLEANFILES += com.endlessm.Speedwagon.service

EXTRA_DIST += \
	$(js_sources) \
	src/config.js.in \
	$(resource_files) \
	speedwagon.gresource.xml \
	eos-speedwagon.in \
	$(NULL)
