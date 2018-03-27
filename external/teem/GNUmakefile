.PHONY: clean distclean
all:
	@echo "all target not defined"

.build.done:
	@mkdir -p linux.64/bin linux.64/lib linux.64/obj && \
	$(MAKE) -C src && \
	touch $@

build: .build.done

.install.done: .build.done
	@touch .install.done

install: .install.done

clean:
	$(MAKE) -C src clean
	$(RM) .build.done

distclean: clean
	$(RM) -rf linux.64
	$(RM) -r .install.done
