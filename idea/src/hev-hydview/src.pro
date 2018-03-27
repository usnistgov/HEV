win32 {
  CONFIG += debug_and_release
  CONFIG += build_all
}
unix|macx {
  CONFIG += debug
}

QT += widgets opengl svg

INCLUDEPATH += ../qwt/src
DEFINES += NO_HEV

win32 {
  CONFIG += force_debug_info
  CONFIG(debug, debug|release) {
    LIBS += ../qwt/lib/qwtd.lib $$PWD/../OpenCL.lib
  } else {
    LIBS += ../qwt/lib/qwt.lib $$PWD/../OpenCL.lib
  }
  DEFINES += _CRT_SECURE_NO_WARNINGS=1
}

unix|macx {
  LIBS += ../qwt/lib/libqwt.a -lOpenCL -lrt
}

win32 {
  QMAKE_CXXFLAGS += /openmp
}

unix|macx {
  QMAKE_CXXFLAGS += -g -fopenmp -Wall -Wextra -Wlogical-op -Wold-style-cast -Wuseless-cast -Wdouble-promotion
  QMAKE_LFLAGS += -fopenmp
}

include($$PWD/src.pri)
