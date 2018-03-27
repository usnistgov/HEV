.PHONY: clean distclean
all:
	@echo "all target not defined"

.configure.done:
	@if [ ! -d bullet-2.76 ]; then \
		tar xf bullet-2.76.tgz; \
	fi && \
	cd bullet-2.76 && env CXXFLAGS="-fpermissive" cmake . && cd .. && \
	touch $@

.build.done: .configure.done
	$(MAKE) -C bullet-2.76 && \
	touch $@

build: .build.done

.install.done: build
	@touch $@

install: .install.done

clean:
	$(MAKE) -C bullet-2.76 clean
	$(RM) .build.done

distclean:
	$(RM) -r bullet-2.76 .configure.done .build.done .install.done
