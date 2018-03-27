# CMakeLists.txt to build examples/dtk/tutorial/loadDSO

INCLUDE_DIRECTORIES (${DTK_SOURCE_DIR}/include)

ADD_EXECUTABLE (load load.cpp)
TARGET_LINK_LIBRARIES (load dtk)

ADD_LIBRARY (implement MODULE implement.cpp)
TARGET_LINK_LIBRARIES (implement dtk)

