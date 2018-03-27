# CMakeLists.txt to build programs/clientServer

INCLUDE_DIRECTORIES( ${DTK_SOURCE_DIR}/include )
ADD_EXECUTABLE( dtk-checkService dtk-checkService.cpp )
TARGET_LINK_LIBRARIES( dtk-checkService dtk )

ADD_EXECUTABLE( dtk-connectRemoteSharedMem dtk-connectRemoteSharedMem.cpp )
TARGET_LINK_LIBRARIES( dtk-connectRemoteSharedMem dtk )

ADD_EXECUTABLE( dtk-connectServer dtk-connectServer.cpp )
TARGET_LINK_LIBRARIES( dtk-connectServer dtk )

ADD_EXECUTABLE( dtk-loadService dtk-loadService.cpp )
TARGET_LINK_LIBRARIES( dtk-loadService dtk )

ADD_EXECUTABLE( dtk-pingServer dtk-pingServer.cpp )
TARGET_LINK_LIBRARIES( dtk-pingServer dtk )

ADD_EXECUTABLE( dtk-resetService dtk-resetService.cpp )
TARGET_LINK_LIBRARIES( dtk-resetService dtk )

ADD_EXECUTABLE( dtk-shutdownServer dtk-shutdownServer.cpp )
TARGET_LINK_LIBRARIES( dtk-shutdownServer dtk )

ADD_EXECUTABLE( dtk-unloadService dtk-unloadService.cpp )
TARGET_LINK_LIBRARIES( dtk-unloadService dtk )

INSTALL( TARGETS dtk-checkService dtk-connectRemoteSharedMem dtk-connectServer
		dtk-loadService dtk-pingServer dtk-resetService dtk-shutdownServer
		dtk-unloadService RUNTIME DESTINATION bin PERMISSIONS OWNER_READ
		OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ
		WORLD_EXECUTE )

