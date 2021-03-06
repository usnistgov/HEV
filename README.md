High End Visualiation (HEV)
===

This is the source code repository for the HEV software that is used to create
immersive visualization applications that run on both the desktop and in the
NIST CAVE.

### Build Requirements

We have developed this software only on CentOS 7 and NVIDIA GPUs.

Additional packages:

- centos-release-scl
- python33
- cmake
- fltk-\*
- mesa-\*
- freeglut-devel
- alsa-lib-devel
- libsndfile-devel
- portaudio-devel
- tk-devel
- epel-release
- R
- GraphicsMagick
- ImageMagick

### Build Instructions

After a fresh clone:
```
$ cd <directory where cloned>
$ source .bashhev
$ hevhere
$ make install
```

Wesley Griffin
Updated: 2018-03-29

