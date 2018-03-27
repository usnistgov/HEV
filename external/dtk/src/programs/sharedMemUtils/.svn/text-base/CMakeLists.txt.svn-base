# CMakeLists.txt to build programs/sharedMemUtils

INCLUDE_DIRECTORIES( ${DTK_SOURCE_DIR}/include )

ADD_EXECUTABLE( dtk-destroySharedMem dtk-destroySharedMem.cpp )
TARGET_LINK_LIBRARIES( dtk-destroySharedMem dtk )

ADD_EXECUTABLE( dtk-getSharedMem dtk-getSharedMem.cpp )
TARGET_LINK_LIBRARIES( dtk-getSharedMem dtk )

ADD_EXECUTABLE( dtk-listFilters dtk-listFilters.cpp )
TARGET_LINK_LIBRARIES( dtk-listFilters dtk )

ADD_EXECUTABLE( dtk-loadFilter dtk-loadFilter.cpp )
TARGET_LINK_LIBRARIES( dtk-loadFilter dtk )

ADD_EXECUTABLE( dtk-printSharedMemAddresses dtk-printSharedMemAddresses.cpp )
TARGET_LINK_LIBRARIES( dtk-printSharedMemAddresses dtk )

ADD_EXECUTABLE( dtk-readButtons dtk-readFloats.cpp floatsOptions.cpp )
SET_TARGET_PROPERTIES( dtk-readButtons PROPERTIES
		COMPILE_FLAGS "-DHEADER_FILE=\\\"buttonsOptions.h\\\" -DBUTTONS" )
TARGET_LINK_LIBRARIES( dtk-readButtons dtk )

ADD_EXECUTABLE( dtk-readCAVEDevices dtk-readCAVEDevices.cpp )
TARGET_LINK_LIBRARIES( dtk-readCAVEDevices dtk )

ADD_EXECUTABLE( dtk-readChars dtk-readFloats.cpp floatsOptions.cpp )
SET_TARGET_PROPERTIES( dtk-readChars PROPERTIES
		COMPILE_FLAGS "-DHEADER_FILE=\\\"charOptions.h\\\"" )
TARGET_LINK_LIBRARIES( dtk-readChars dtk )

ADD_EXECUTABLE( dtk-readDoubles dtk-readFloats.cpp floatsOptions.cpp )
SET_TARGET_PROPERTIES( dtk-readDoubles PROPERTIES
		COMPILE_FLAGS "-DHEADER_FILE=\\\"doublesOptions.h\\\"" )
TARGET_LINK_LIBRARIES( dtk-readDoubles dtk )

ADD_EXECUTABLE( dtk-readFloats dtk-readFloats.cpp floatsOptions.cpp )
SET_TARGET_PROPERTIES( dtk-readFloats PROPERTIES
		COMPILE_FLAGS "-DHEADER_FILE=\\\"floatsOptions.h\\\"" )
TARGET_LINK_LIBRARIES( dtk-readFloats dtk )

ADD_EXECUTABLE( dtk-readStrings dtk-readStrings.cpp )
TARGET_LINK_LIBRARIES( dtk-readStrings dtk )

ADD_EXECUTABLE( dtk-unloadFilter dtk-unloadFilter.cpp )
TARGET_LINK_LIBRARIES( dtk-unloadFilter dtk )

ADD_EXECUTABLE( dtk-writeDoubles dtk-writeFloats.cpp )
SET_TARGET_PROPERTIES( dtk-writeDoubles PROPERTIES
		COMPILE_FLAGS "-DTYPE=double" )
TARGET_LINK_LIBRARIES( dtk-writeDoubles dtk )

ADD_EXECUTABLE( dtk-writeFloats dtk-writeFloats.cpp )
TARGET_LINK_LIBRARIES( dtk-writeFloats dtk )

ADD_EXECUTABLE( dtk-noiseFloats dtk-noiseFloats.cpp )
TARGET_LINK_LIBRARIES( dtk-noiseFloats dtk )

ADD_EXECUTABLE( dtk-writeStrings dtk-writeStrings.cpp )
TARGET_LINK_LIBRARIES( dtk-writeStrings dtk )

ADD_EXECUTABLE( dtk-copyShm dtk-copyShm.cpp )
TARGET_LINK_LIBRARIES( dtk-copyShm dtk )

ADD_EXECUTABLE( dtk-getSize dtk-getSize.cpp )
TARGET_LINK_LIBRARIES( dtk-getSize dtk )

ADD_EXECUTABLE( dtk-zeroShm dtk-zeroShm.cpp )
TARGET_LINK_LIBRARIES( dtk-zeroShm dtk )

ADD_EXECUTABLE( dtk-readLine dtk-readLine.cpp )
TARGET_LINK_LIBRARIES( dtk-readLine dtk )

ADD_EXECUTABLE( dtk-writeLine dtk-writeLine.cpp )
TARGET_LINK_LIBRARIES( dtk-writeLine dtk )

ADD_EXECUTABLE( dtk-writeShm dtk-writeShm.cpp )
TARGET_LINK_LIBRARIES( dtk-writeShm dtk )

IF( UNIX )
	ADD_EXECUTABLE( dtk-readVRCOSysV dtk-readVRCOSysV.cpp )
	TARGET_LINK_LIBRARIES( dtk-readVRCOSysV dtk )
	INSTALL( TARGETS dtk-readVRCOSysV RUNTIME DESTINATION bin PERMISSIONS
			OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE
			WORLD_READ WORLD_EXECUTE )
ENDIF( UNIX )

INSTALL( TARGETS dtk-destroySharedMem dtk-getSharedMem dtk-listFilters
		dtk-loadFilter dtk-printSharedMemAddresses dtk-readButtons
		dtk-readCAVEDevices dtk-readChars dtk-readDoubles dtk-readFloats
		dtk-readStrings dtk-unloadFilter dtk-writeDoubles dtk-writeFloats
		dtk-noiseFloats dtk-writeStrings dtk-copyShm dtk-getSize dtk-readLine
		dtk-writeLine dtk-writeShm dtk-zeroShm RUNTIME DESTINATION bin
		PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ
		GROUP_EXECUTE WORLD_READ WORLD_EXECUTE )

