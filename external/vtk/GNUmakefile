.PHONY: clean distclean
all:
	@echo "all target not defined"

.configure.done:
	@if [ ! -d VTK5.10.1 ]; then \
		tar xf vtk-5.10.1.tar.gz && \
		cd VTK5.10.1 && \
		patch -p1 < ../vtk-HEV.patch && \
		cd ..; \
	fi && \
	mkdir -p build && \
	cd build && cmake \
		-D CMAKE_INSTALL_PREFIX=$$OLDPWD/installed \
		-D CMAKE_BUILD_TYPE=Release \
		-D BUILD_SHARED_LIBS=on \
		-D VTK_BUILD_FORWARDING_EXECUTABLES=on \
		-D VTK_WRAP_TCL=on \
		../VTK5.10.1 && \
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
	$(RM) -r VTK5.10.1 .configure.done build .build.done installed .install.done
