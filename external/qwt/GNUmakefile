.PHONY: clean distclean
all:
	@echo "all target not defined"

.configure.done:
	@if [ ! -d qwt-6.1.3 ]; then \
		tar xf qwt-6.1.3.tar.bz2 && \
		cd qwt-6.1.3 && \
		patch -p1 < ../qwt-HEV.patch && \
		cd ..; \
	fi && \
	cd qwt-6.1.3 && qmake-qt5 && \
	cd .. && touch $@

.build.done: .configure.done
	$(MAKE) -C qwt-6.1.3 && \
	touch $@

build: .build.done

.install.done: .build.done
	$(MAKE) -C qwt-6.1.3 install && \
	touch $@

install: .install.done

clean:
	$(MAKE) -C qwt-6.1.3 clean
	$(RM) .build.done

distclean:
	$(RM) -r qwt-6.1.3 .configure.done .build.done installed .install.done
