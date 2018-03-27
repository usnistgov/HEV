# CMakeLists.txt to build examples/dtk/tests/dtkDSO_

INCLUDE_DIRECTORIES (${DTK_SOURCE_DIR}/include)

ADD_EXECUTABLE (load load.cpp)
TARGET_LINK_LIBRARIES (load dtk)

ADD_LIBRARY (foo MODULE foo.cpp)
TARGET_LINK_LIBRARIES (foo dtk)

ADD_LIBRARY (base MODULE base.cpp)
TARGET_LINK_LIBRARIES (base dtk)

