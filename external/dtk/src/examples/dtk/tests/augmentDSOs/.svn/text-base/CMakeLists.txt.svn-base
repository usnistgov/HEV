# CMakeLists.txt to build examples/dtk/tests/augmentDSOs

INCLUDE_DIRECTORIES (${DTK_SOURCE_DIR}/include)

ADD_EXECUTABLE (test-dtk-augmentManager dtk-augmentManager.cpp)
TARGET_LINK_LIBRARIES (test-dtk-augmentManager dtk)

ADD_LIBRARY (test test.cpp)
TARGET_LINK_LIBRARIES (test dtk)

