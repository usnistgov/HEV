# CMakeLists.txt to build examples/dtk/tests/shmWriteSignalCatcher

INCLUDE_DIRECTORIES (${DTK_SOURCE_DIR}/include)

ADD_EXECUTABLE (dtkConditional_signal dtkConditional_signal.cpp)
TARGET_LINK_LIBRARIES (dtkConditional_signal dtk)

ADD_EXECUTABLE (dtkConditional_wait dtkConditional_wait.cpp)
TARGET_LINK_LIBRARIES (dtkConditional_wait dtk)

ADD_EXECUTABLE (pause_reader pause_reader.cpp)
TARGET_LINK_LIBRARIES (pause_reader dtk rt)

ADD_EXECUTABLE (sem_open sem_open.cpp)
TARGET_LINK_LIBRARIES (sem_open dtk)

ADD_EXECUTABLE (dtkConditional_signal_fast dtkConditional_signal_fast.cpp)
TARGET_LINK_LIBRARIES (dtkConditional_signal_fast dtk)

ADD_EXECUTABLE (mmap_mprotect mmap_mprotect.cpp)
TARGET_LINK_LIBRARIES (mmap_mprotect dtk)

ADD_EXECUTABLE (reader reader.cpp)
TARGET_LINK_LIBRARIES (reader dtk rt)

ADD_EXECUTABLE (writer writer.cpp)
TARGET_LINK_LIBRARIES (writer dtk rt)

ADD_EXECUTABLE (dtkConditional_signal_wait dtkConditional_signal_wait.cpp)
TARGET_LINK_LIBRARIES (dtkConditional_signal_wait dtk)

ADD_EXECUTABLE (mprotect mprotect.cpp)
TARGET_LINK_LIBRARIES (mprotect dtk)

ADD_EXECUTABLE (readFloats readFloats.cpp)
TARGET_LINK_LIBRARIES (readFloats dtk)

ADD_EXECUTABLE (writeRead writeRead.cpp)
TARGET_LINK_LIBRARIES (writeRead dtk)

