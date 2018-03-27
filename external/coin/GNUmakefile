.PHONY: clean distclean
all:
	@echo "all target not defined"

build: .coin-build.done .soxt-build.done .simage-build.done

install: .coin-install.done .soxt-install.done .simage-install.done

clean:
	$(MAKE) -C Coin-3.1.3 clean
	$(RM) .coin-build.done
	$(MAKE) -C SoXt-1.3.0 clean
	$(RM) .soxt-build.done
	$(MAKE) -C simage-1.7.0 clean
	$(RM) .simage-build.done

distclean:
	$(RM) -r Coin-3.1.3 .coin-configure.done .coin-build.done installed .coin-install.done
	$(RM) -r SoXt-1.3.0 .soxt-configure.done .soxt-build.done installed .soxt-install.done
	$(RM) -r simage-1.7.0 .simage-configure.done .simage-build.done installed .simage-install.done


.coin-configure.done:
	@if [ ! -d Coin-3.1.3 ]; then \
		tar xf Coin-3.1.3.tar.gz; \
	fi && \
	cd Coin-3.1.3 && env CXXFLAGS="-O3 -fpermissive" ./configure \
		--prefix=$$OLDPWD/installed \
		--enable-threadsafe \
		--disable-dl-freetype && \
	cd .. && touch $@

.coin-build.done: .coin-configure.done
	$(MAKE) -C Coin-3.1.3 && \
	touch $@

.coin-install.done: .coin-build.done
	$(MAKE) -C Coin-3.1.3 install && \
	touch $@


.soxt-configure.done: .coin-install.done
	@if [ ! -d SoXt-1.3.0 ]; then \
		tar xf SoXt-1.3.0.tar.gz; \
	fi && \
	cd SoXt-1.3.0 && \
		env CXXFLAGS="-O3 -fpermissive" PATH=$$PATH:$$OLDPWD/installed/bin \
		./configure \
		--prefix=$$OLDPWD/installed \
		--with-motif=no && \
	cd .. && touch $@

.soxt-build.done: .soxt-configure.done
	$(MAKE) -C SoXt-1.3.0 && \
	touch $@

.soxt-install.done: .soxt-build.done
	$(MAKE) -C SoXt-1.3.0 install && \
	touch $@


.simage-configure.done: .coin-install.done
	@if [ ! -d simage-1.7.0 ]; then \
		tar xf simage-1.7.0.tar.gz; \
		fi && \
	cd simage-1.7.0 && \
		env CXXFLAGS="-O3 -fpermissive" PATH=$$PATH:$$OLDPWD/installed/bin \
		./configure \
		--prefix=$$OLDPWD/installed \
		--enable-threadsafe \
		--with-png=no && \
	cd .. && touch $@

.simage-build.done: .simage-configure.done
	$(MAKE) -C simage-1.7.0 && \
	touch $@

.simage-install.done: .simage-build.done
	$(MAKE) -C simage-1.7.0 install && \
	touch $@
