.PHONY: clean distclean
all:
	@echo "all target not defined"

.configure.done:
	@if [ ! -d OpenSceneGraph-3.2.0 ]; then \
		unzip OpenSceneGraph-3.2.0.zip && \
		cd OpenSceneGraph-3.2.0 && \
		patch -p1 < ../osg-HEV.patch && \
		cd ..; \
	fi && \
	mkdir -p build && \
	cd build && cmake \
		-D CMAKE_BUILD_TYPE=Release \
		-D CMAKE_INSTALL_PREFIX=$$OLDPWD/installed \
		-D BUILD_DOCUMENTATION=on \
		-D BUILD_REF_DOCS_TAGFILE=off \
		-D BUILD_OSG_EXAMPLES=off \
		-D OSG_USE_AGGRESSIVE_WARNINGS=off \
		-D OSG_NOTIFY_DISABLED=off \
		-D OSG_USE_QT=off \
		-D INVENTOR_INCLUDE_DIR=$$HEVROOT/external/coin/installed/include \
		-D INVENTOR_LIBRARY=$$HEVROOT/external/coin/installed/lib/libCoin.so \
		../OpenSceneGraph-3.2.0 && \
	cd .. && touch $@

.build.done: .configure.done
	$(MAKE) -C build && \
	$(MAKE) -C build doc_openscenegraph && \
	touch $@

build: .build.done

.install.done: .build.done
	$(MAKE) -C build install && \
	if [ ! -d OpenSceneGraph-3.0.0.data ]; then \
		unzip OpenSceneGraph-Data-3.0.0.zip; \
		ln -sf OpenSceneGraph-Data-3.0.0 OpenSceneGraph-Data; \
	fi && \
	touch $@

install: .install.done

clean:
	$(MAKE) -C build clean
	$(RM) .build.done

distclean:
	$(RM) -r OpenSceneGraph-3.2.0 OpenSceneGraph-Data-3.0.0 OpenSceneGraph-Data
	$(RM) -r .configure.done build .build.done installed .install.done
