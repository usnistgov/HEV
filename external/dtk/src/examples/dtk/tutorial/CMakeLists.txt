# CMakeLists.txt to build examples/dtk/tutorial

ADD_SUBDIRECTORY (dtkSharedMem)
ADD_SUBDIRECTORY (helloService)
ADD_SUBDIRECTORY (loadDSO)

IF (NOT WIN32)
	ADD_SUBDIRECTORY (balanceStick)
ENDIF (NOT WIN32)

