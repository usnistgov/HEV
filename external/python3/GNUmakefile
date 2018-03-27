PKGS= \
	networkx-1.7.tar.gz \

.PHONY: clean distclean
all:
	@echo "all target not defined"

build:

.install.done:
	@mkdir -p installed && \
	for pkg in $(PKGS); do \
		bn=`basename $$pkg .tar.gz`; \
		if [ ! -d $$bn ]; then \
			echo tar xf $$pkg; \
			tar xf $$pkg; \
		fi; \
		cd $$bn && python3 setup.py install --prefix=$$OLDPWD/installed && cd .. ; \
	done && \
	touch $@

install: .install.done

distclean:
	@for pkg in $(PKGS); do \
		echo $(RM) -r `basename $$pkg .tar.gz`; \
		$(RM) -r `basename $$pkg .tar.gz`; \
	done
	$(RM) -r installed .install.done

