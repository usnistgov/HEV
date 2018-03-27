SUBDIRS = lib DSO program doc
MAKETARGETS = build test install
CLEANTARGETS = clean distclean

.PHONY: $(MAKETARGETS) $(CLEANTARGETS) examples tarball

$(MAKETARGETS): iris.pc
	@for dir in $(SUBDIRS) ; do \
		echo ; \
		echo ; \
		echo "# --------------------------------" ; \
		echo ; \
		echo $$dir ; \
		echo ; \
	    $(MAKE) -C $$dir $(MAKECMDGOALS); ret=$$? ; \
	    if [ $$ret -ne 0 ] ; then exit $$ret ; fi  ; \
		echo ; \
	done

$(CLEANTARGETS):
	@for dir in $(SUBDIRS) ; do \
		echo ; \
		echo ; \
		echo "# --------------------------------" ; \
		echo ; \
		echo $$dir ; \
		echo ; \
	    $(MAKE) -C $$dir $(MAKECMDGOALS); ret=$$? ; \
	    if [ $$ret -ne 0 ] ; then exit $$ret ; fi  ; \
		echo ; \
	done
	@for dir in examples/* ; do \
		echo ; \
		echo ; \
		echo "# --------------------------------" ; \
		echo ; \
		echo $$dir ; \
		echo ; \
		$(MAKE) -C $$dir $(MAKECMDGOALS); ret=$$? ; \
		if [ $$ret -ne 0 ] ; then exit $$ret ; fi  ; \
		echo ; \
	done

# not all examples need to be compiled
examples:
	@for dir in `find examples -name GNUmakefile | sed -e 's/.GNUmakefile//'`; do \
		echo ; \
		echo ; \
		echo "# --------------------------------" ; \
		echo ; \
		echo $$dir ; \
		echo ; \
		$(MAKE) -C $$dir ; ret=$$? ; \
		if [ $$ret -ne 0 ] ; then exit $$ret ; fi  ; \
		echo ; \
	done

DSO: lib

program: lib

iris.pc: iris.pc.input
  $(shell sh -c "sed -e \"/^prefix=/s|$$|$(IRIS_PREFIX)|\" -e \"/^Libs: /s|$$| -L`dtk-config --lib-dir` -ldtk|\" -e \"/^Cflags: /s|$$| `dtk-config --cflags` `dtk-config --include`|\" < iris.pc.input > iris.pc")

ifeq "$(MAKECMDGOALS)" "install"
  $(shell sh -c "mkdir -p $(IRIS_PREFIX)/bin")
  $(shell sh -xc "cp -r iris.pc include $(IRIS_PREFIX); ")
  $(shell sh -c "mkdir -p $(IRIS_PREFIX)/bin ; cd script; echo iris-* | xargs -I '{}' sh -xc \"cp '{}' $(IRIS_PREFIX)/bin\" ; ")
  $(shell sh -c "find $(IRIS_PREFIX) -name \*~ -print0 | xargs -0 rm -f")
endif


tarball:
	@make distclean; \
	rm -f iris.pc; \
	i=`find . -type f -name libiris.so | grep -v /src/ | sed -e 's|/lib/libiris.so||'`; \
	if [ "$$i" != "" ] ; then rm -rf $$i ; fi ;\
	find . -name \*~ | xargs rm -f; \
	b=`basename $$PWD`; \
	cd .. ; \
	tar -zchf /tmp/$$b.tar.gz $$b ; ls -l /tmp/$$b.tar.gz

