.PHONY: clean distclean
all:
	@echo "all target not defined"

.configure.done:
	@mkdir -p build && \
	cd build && cmake \
		-D CMAKE_C_COMIPILER=gcc \
		-D CMAKE_CXX_COMPILER=g++ \
		-D CMAKE_INSTALL_PREFIX=$$OLDPWD/installed \
		-D CMAKE_BUILD_TYPE=Release \
		-D DTK_BUILD_DOCUMENTATION=on \
		-D FLTK_DIR=/usr \
		../src && \
	cd .. && touch $@

.build.done: .configure.done
	$(MAKE) -C build && \
	touch $@

build: .build.done

.install.done: .build.done
	$(MAKE) -C build install && \
	touch $@

install: .install.done

clean:
	$(MAKE) -C build clean
	$(RM) .build.done

distclean:
	$(RM) -r .configure.done build .build.done installed .install.done \
		src/config/config.h

