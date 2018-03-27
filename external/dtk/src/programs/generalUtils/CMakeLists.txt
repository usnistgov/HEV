# CMakeLists.txt to build programs/generalUtils

INCLUDE_DIRECTORIES( ${DTK_SOURCE_DIR}/include )

IF( WIN32 )
	SET( EXTRA_TARGETS )
ELSE( WIN32 )
	SET( EXTRA_TARGETS dtk-recorder )
	ADD_EXECUTABLE( dtk-recorder dtk-recorder.cpp )
	TARGET_LINK_LIBRARIES( dtk-recorder dtk )
ENDIF( WIN32 )

ADD_EXECUTABLE( dtk-checkColor dtk-checkColor.cpp )
TARGET_LINK_LIBRARIES( dtk-checkColor dtk )

ADD_EXECUTABLE( dtk-fsleep dtk-fsleep.cpp )
TARGET_LINK_LIBRARIES( dtk-fsleep dtk )

ADD_EXECUTABLE( dtk-printEnv dtk-printEnv.cpp )
TARGET_LINK_LIBRARIES( dtk-printEnv dtk )

ADD_EXECUTABLE( dtk-msgError dtk-msgError.cpp )
TARGET_LINK_LIBRARIES( dtk-msgError dtk )

ADD_EXECUTABLE( dtk-msgDebug dtk-msgError.cpp )
SET_TARGET_PROPERTIES( dtk-msgDebug PROPERTIES COMPILE_FLAGS "-DLEVEL=DTKMSG_DEBUG -DPROGRAM_NAME=\\\"dtk-msgDebug\\\"" )
TARGET_LINK_LIBRARIES( dtk-msgDebug dtk )

ADD_EXECUTABLE( dtk-msgInfo dtk-msgError.cpp )
SET_TARGET_PROPERTIES( dtk-msgInfo PROPERTIES COMPILE_FLAGS "-DLEVEL=DTKMSG_INFO -DPROGRAM_NAME=\\\"dtk-msgInfo\\\"" )
TARGET_LINK_LIBRARIES( dtk-msgInfo dtk )

ADD_EXECUTABLE( dtk-msgWarning dtk-msgError.cpp )
SET_TARGET_PROPERTIES( dtk-msgWarning PROPERTIES COMPILE_FLAGS "-DLEVEL=DTKMSG_WARNING -DPROGRAM_NAME=\\\"dtk-msgWarning\\\"" )
TARGET_LINK_LIBRARIES( dtk-msgWarning dtk )

ADD_EXECUTABLE( dtk-msgNotice dtk-msgError.cpp )
SET_TARGET_PROPERTIES( dtk-msgNotice PROPERTIES COMPILE_FLAGS "-DLEVEL=DTKMSG_NOTICE -DPROGRAM_NAME=\\\"dtk-msgNotice\\\"" )
TARGET_LINK_LIBRARIES( dtk-msgNotice dtk )

INSTALL( TARGETS dtk-checkColor dtk-fsleep ${EXTRA_TARGETS} dtk-printEnv dtk-msgError
	dtk-msgDebug dtk-msgInfo dtk-msgWarning dtk-msgNotice RUNTIME DESTINATION bin
	PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE
	WORLD_READ WORLD_EXECUTE )

