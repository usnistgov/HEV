# CMakeLists.txt to build examples/dtk/tests/dtkUDPSocket

INCLUDE_DIRECTORIES (${DTK_SOURCE_DIR}/include)

ADD_EXECUTABLE (test-UDP-client client.cpp)
TARGET_LINK_LIBRARIES (test-UDP-client dtk)

ADD_EXECUTABLE (test-UDP-server server.cpp)
TARGET_LINK_LIBRARIES (test-UDP-server dtk)

