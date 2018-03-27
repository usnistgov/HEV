# CMakeLists.txt to build examples/dtk/examplesByClass/dtkDSO_

INCLUDE_DIRECTORIES (${DTK_SOURCE_DIR}/include)

ADD_EXECUTABLE (load load.cpp)
TARGET_LINK_LIBRARIES (load dtk)

ADD_LIBRARY (fooDSO MODULE fooDSO.cpp)
TARGET_LINK_LIBRARIES (fooDSO dtk)

ADD_LIBRARY (barDSO MODULE barDSO.cpp)
TARGET_LINK_LIBRARIES (barDSO dtk)

