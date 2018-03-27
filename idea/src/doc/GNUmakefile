#!gmake

# This is a hack.

# Use -W break to surpress annoying warning messages
# warning [p 1, 2.0i]: cannot adjust line
# for long URL's that don't fit the page nicely



%.1.pdf: $(HEVROOT)/idea/man/man1/%.1
	@groff -W break -mandoc $^ |  ps2pdf - > $@

%.2.pdf: $(HEVROOT)/idea/man/man2/%.2
	@groff -W break -mandoc $^ |  ps2pdf - > $@

%.3.pdf: $(HEVROOT)/idea/man/man3/%.3
	@groff -W break -mandoc $^ |  ps2pdf - > $@

%.4.pdf: $(HEVROOT)/idea/man/man4/%.4
	@groff -W break -mandoc $^ |  ps2pdf - > $@

%.5.pdf: $(HEVROOT)/idea/man/man5/%.5
	@groff -W break -mandoc $^ |  ps2pdf - > $@

%.6.pdf: $(HEVROOT)/idea/man/man6/%.6
	@groff -W break -mandoc $^ |  ps2pdf - > $@

%.7.pdf: $(HEVROOT)/idea/man/man7/%.7
	@groff -W break -mandoc $^ |  ps2pdf - > $@

%.8.pdf: $(HEVROOT)/idea/man/man8/%.8
	@groff -W break -mandoc $^ |  ps2pdf - > $@

%.9.pdf: $(HEVROOT)/idea/man/man9/%.9
	@groff -W break -mandoc $^ |  ps2pdf - > $@


%.1.html: $(HEVROOT)/idea/man/man1/%.1
	@groff -W break -mandoc -Thtml $^ > $@

%.2.html: $(HEVROOT)/idea/man/man2/%.2
	@groff -W break -mandoc -Thtml $^ > $@

%.3.html: $(HEVROOT)/idea/man/man3/%.3
	@groff -W break -mandoc -Thtml $^ > $@

%.4.html: $(HEVROOT)/idea/man/man4/%.4
	@groff -W break -mandoc -Thtml $^ > $@

%.5.html: $(HEVROOT)/idea/man/man5/%.5
	@groff -W break -mandoc -Thtml $^ > $@

%.6.html: $(HEVROOT)/idea/man/man6/%.6
	@groff -W break -mandoc -Thtml $^ > $@

%.7.html: $(HEVROOT)/idea/man/man7/%.7
	@groff -W break -mandoc -Thtml $^ > $@

%.8.html: $(HEVROOT)/idea/man/man8/%.8
	@groff -W break -mandoc -Thtml $^ > $@

%.9.html: $(HEVROOT)/idea/man/man9/%.9
	@groff -W break -mandoc -Thtml $^ > $@


PAGES:=$(shell find $(HEVROOT)/idea/man -type f)
LOCAL:=$(notdir $(PAGES))
PDF:=$(addsuffix .pdf, $(LOCAL))
HTML:=$(addsuffix .html, $(LOCAL))

all: $(PDF) $(HTML)

install: all
	mkdir -p $(HEV_IDEA_DIR)/doc/pdf
	mkdir -p $(HEV_IDEA_DIR)/doc/html
	cp -f *.pdf $(HEV_IDEA_DIR)/doc/pdf
	cp -f index.html.src $(HEV_IDEA_DIR)/doc/pdf/index.html
	cp -f *.html $(HEV_IDEA_DIR)/doc/html
	rm -f *.pdf *.html

clean:
	rm -f *.pdf *.html

distclean: clean


