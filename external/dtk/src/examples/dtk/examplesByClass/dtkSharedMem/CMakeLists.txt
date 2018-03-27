# CMakeLists.txt to build examples/dtk/examplesByClass/dtkSharedMem

INCLUDE_DIRECTORIES (${DTK_SOURCE_DIR}/include)

ADD_EXECUTABLE (countPollRead countPollRead.cpp)
TARGET_LINK_LIBRARIES (countPollRead dtk)

ADD_EXECUTABLE (queueRead queueRead.cpp)
TARGET_LINK_LIBRARIES (queueRead dtk)

ADD_EXECUTABLE (runSecond runSecond.cpp)
TARGET_LINK_LIBRARIES (runSecond dtk)

ADD_EXECUTABLE (timePollRead timePollRead.cpp)
TARGET_LINK_LIBRARIES (timePollRead dtk)

ADD_EXECUTABLE (countQueueRead countQueueRead.cpp)
TARGET_LINK_LIBRARIES (countQueueRead dtk)

ADD_EXECUTABLE (pollRead pollRead.cpp)
TARGET_LINK_LIBRARIES (pollRead dtk)

ADD_EXECUTABLE (runFirst runFirst.cpp)
TARGET_LINK_LIBRARIES (runFirst dtk)

ADD_EXECUTABLE (sinWrite sinWrite.cpp)
TARGET_LINK_LIBRARIES (sinWrite dtk)

ADD_EXECUTABLE (timeQueueRead timeQueueRead.cpp)
TARGET_LINK_LIBRARIES (timeQueueRead dtk)

