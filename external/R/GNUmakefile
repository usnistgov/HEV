PKGS= \
	rJava_0.9-6.tar.gz \
	RWekajars_3.7.10-1.tar.gz \
	RWeka_0.4-21.tar.gz \
	DBI_0.2-7.tar.gz \
	RSQLite_0.11.4.tar.gz \
	rjson_0.2.13.tar.gz \
	png_0.1-7.tar.gz \
	pixmap_0.4-11.tar.gz \
	slam_0.1-31.tar.gz \
	Snowball_0.0-11.tar.gz \
	SnowballC_0.5.tar.gz \
	tm_0.5-10.tar.gz \
	entropy_1.2.1.tar.gz

.PHONY: clean distclean
all:
	@echo "all target not defined"

build:

.install.done:
	@mkdir -p installed && \
	for pkg in $(PKGS); do \
		echo R CMD INSTALL -l $$PWD/installed $$pkg; \
		R CMD INSTALL -l $$PWD/installed $$pkg; \
	done && \
	touch $@

install: .install.done

distclean:
	$(RM) -r installed .install.done

