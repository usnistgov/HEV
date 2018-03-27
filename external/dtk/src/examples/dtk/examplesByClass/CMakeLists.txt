# CMakeLists.txt to build examples/dtk/examplesByClass

ADD_SUBDIRECTORY (dtkAugment)

IF (NOT WIN32)
	IF (NOT APPLE)
		ADD_SUBDIRECTORY (dtkDSO_)
	ENDIF (NOT APPLE)
	ADD_SUBDIRECTORY (dtkSharedMem)
	ADD_SUBDIRECTORY (dtkTCPSocket)
	ADD_SUBDIRECTORY (dtkUDPSocket)
ENDIF (NOT WIN32)

INCLUDE_DIRECTORIES (${DTK_SOURCE_DIR}/include)

IF (NOT WIN32)
	ADD_EXECUTABLE (dtkRealtime dtkRealtime.cpp)
	TARGET_LINK_LIBRARIES (dtkRealtime dtk)
ENDIF (NOT WIN32)

ADD_EXECUTABLE (dtkBase dtkBase.cpp)
TARGET_LINK_LIBRARIES (dtkBase dtk)

ADD_EXECUTABLE (dtkDiffEq dtkDiffEq.cpp)
TARGET_LINK_LIBRARIES (dtkDiffEq dtk)

ADD_EXECUTABLE (dtk_getFileFromPath dtk_getFileFromPath.cpp)
TARGET_LINK_LIBRARIES (dtk_getFileFromPath dtk)

ADD_EXECUTABLE (dtkTime dtkTime.cpp)
TARGET_LINK_LIBRARIES (dtkTime dtk)

ADD_EXECUTABLE (dtkColor dtkColor.cpp)
TARGET_LINK_LIBRARIES (dtkColor dtk)

ADD_EXECUTABLE (dtkDistribution dtkDistribution.cpp)
TARGET_LINK_LIBRARIES (dtkDistribution dtk)

ADD_EXECUTABLE (dtkMatrix dtkMatrix.cpp)
TARGET_LINK_LIBRARIES (dtkMatrix dtk)

ADD_EXECUTABLE (dtkRK4 dtkRK4.cpp)
TARGET_LINK_LIBRARIES (dtkRK4 dtk)

ADD_EXECUTABLE (dtkVec3 dtkVec3.cpp)
TARGET_LINK_LIBRARIES (dtkVec3 dtk)

ADD_EXECUTABLE (dtkCoord dtkCoord.cpp)
TARGET_LINK_LIBRARIES (dtkCoord dtk)

ADD_EXECUTABLE (dtkFLTKOptions dtkFLTKOptions.cpp)
TARGET_LINK_LIBRARIES (dtkFLTKOptions dtk)

ADD_EXECUTABLE (dtkMessage dtkMessage.cpp)
TARGET_LINK_LIBRARIES (dtkMessage dtk)

ADD_EXECUTABLE (dtk_tempname dtk_tempname.cpp)
TARGET_LINK_LIBRARIES (dtk_tempname dtk)

