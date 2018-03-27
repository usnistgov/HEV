CUDA_RELEASE=8.0.44
CUDA_FILE=cuda_$(CUDA_RELEASE)_linux-run
CUDA_URL=https://developer.nvidia.com/compute/cuda/8.0/prod/local_installers/$(CUDA_FILE)

.PHONY: clean distclean
all:
	@echo "all target not defined"

build:

.install.done:
	@if [ ! -f $(CUDA_FILE) ]; then \
		echo "Downloading CUDA..."; \
		wget $(CUDA_URL) -O $(CUDA_FILE); \
	fi && \
	if [ ! -d installed ]; then \
		echo "Installing CUDA..."; \
		/bin/sh $(CUDA_FILE) --silent --toolkit --toolkitpath=$$PWD/installed; \
	fi && \
	touch $@

install: .install.done

clean:

distclean:
	$(RM) -r installed .install.done $(CUDA_FILE)

