.PHONY: build test install clean distclean man

man1dir = ../man/man1
man1 = $(wildcard $(man1dir)/*.1) 
man1pdf = $(patsubst $(man1dir)/%.1,%.pdf,$(man1))
#$(warning man1 = $(man1) man1pdf = $(man1pdf)  )

odt = $(wildcard *.odt)
odtpdf = $(patsubst %.odt,%.pdf,$(odt))
#$(warning odtpdf = $(odtpdf) )

odg = $(wildcard *.odg)
odgpdf = $(patsubst %.odg,%.pdf,$(odg))
#$(warning odgpdf = $(odgpdf) )

build test: Doxyfile 000refsheet.doxy 100manpages.doxy
	@dtk-msgNotice -p Makefile building doxygen documentation- be patient, it can take a while
	@doxygen

Doxyfile: Doxyfile.in
	@dtk-msgNotice -p Makefile creating Doxyfile
	@sed -e "s|OSGPREFIXDIR|`pkg-config --variable=prefix openscenegraph`|g" \
	    -e "s|DTKPREFIXDIR|`dtk-config --root`|g" < Doxyfile.in > Doxyfile
	doxygen -u


install: build
	@mkdir -p $(IRIS_PREFIX)/doc/html
	@mkdir -p $(IRIS_PREFIX)/doc/pdf
	cp -R html/* $(IRIS_PREFIX)/doc/html
	cp *.pdf $(IRIS_PREFIX)/doc/pdf

clean:
	rm -f Doxyfile $(man1pdf) 000refsheet.doxy 100manpages.doxy

distclean: clean
	rm -rf *~ html \#*

%.pdf:	%.odt %.odg
	@dtk-msgError -p Makefile the file $@ is out of date, please re-export from $<

%.pdf:	$(man1dir)/%.1
	@dtk-msgNotice -p Makefile making pdf file $@ from man1 page $<
	@groff -man $< | ps2pdf - $@

000refsheet.doxy: $(odtpdf) $(odgpdf) 
	@#dtk-msgNotice -p Makefile making $@
	@echo '/** \page refsheet IRIS reference sheets' > $@
	@for f in $(odtpdf) $(odgpdf); do b=`basename $$f .pdf` ; echo '\n <a href=file:../pdf/'$$f' >'$$b'</a>' >> $@ ; done
	@echo '*/' >> $@

100manpages.doxy: $(man1pdf)
	@dtk-msgNotice -p Makefile making $@
	@echo '/** \page manpages IRIS man pages' > $@
	@for f in $(man1pdf) ; do b=`basename $$f .pdf`.1 ; echo '\n <a href=file:../pdf/'$$f' >'$$b'</a>' >> $@ ; done
	@echo '*/' >> $@
