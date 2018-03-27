.PHONY: clean distclean
all:
	@echo "all target not defined"

build:

.install.done:
	@if [ ! -d node-v0.12.15-linux-x64 ] ; then \
		echo tar xf node-v0.12.15-linux-x64.tar.xz; \
		tar xf node-v0.12.15-linux-x64.tar.xz; \
	fi && \
	ln -sf node-v0.12.15-linux-x64 node-v0.12.15 && \
	npm install -g $$HEVROOT/idea/src/iris-node && \
	touch $@

install: .install.done

clean:

distclean:
	$(RM) -r node-v0.12.15-linux-x64 node-v0.12.15 .build.done installed .install.done
