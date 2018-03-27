# CMakeLists.txt to build examples/dtk/more/tcpEchoService

INCLUDE_DIRECTORIES (${DTK_SOURCE_DIR}/include)
ADD_LIBRARY (tcpEcho MODULE tcpEcho.cpp)
TARGET_LINK_LIBRARIES (tcpEcho dtk libdtk-server)

