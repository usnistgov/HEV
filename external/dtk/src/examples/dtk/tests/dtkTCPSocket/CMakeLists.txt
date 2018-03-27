# CMakeLists.txt to build examples/dtk/tests/dtkTCPSocket

INCLUDE_DIRECTORIES (${DTK_SOURCE_DIR}/include)

ADD_EXECUTABLE (test-TCP-client client.cpp)
TARGET_LINK_LIBRARIES (test-TCP-client dtk)

ADD_EXECUTABLE (test-TCP-server server.cpp)
TARGET_LINK_LIBRARIES (test-TCP-server dtk)

