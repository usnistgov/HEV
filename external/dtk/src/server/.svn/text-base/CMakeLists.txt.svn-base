# CMakeLists.txt to build dtk-server

IF( WIN32 )
	SET( DTK_WIN32_LIBS ws2_32.lib )
ENDIF( WIN32 )

INCLUDE_DIRECTORIES( ${DTK_SOURCE_DIR}/include ${DTK_SOURCE_DIR}/config )
SET( LIBDTKSERVER_SOURCES  parseRequest.cpp serviceInetTCP.cpp
		serviceInetUDP.cpp serviceSlaveTCP.cpp serviceClientTCP.cpp
		serviceManager.cpp serverRespond.cpp globels.cpp serverMutex.cpp
		condition.cpp configParser.cpp get_dso_path.cpp )

IF( WIN32 )
	SET( LIBDTKSERVER_SOURCES ${LIBDTKSERVER_SOURCES} threadManager.cpp )
ENDIF( WIN32 )

ADD_LIBRARY( libdtk-server ${LIBDTKSERVER_SOURCES} )
TARGET_LINK_LIBRARIES( libdtk-server dtk ${DTK_WIN32_LIBS} )

IF( WIN32 )
	SET_TARGET_PROPERTIES( libdtk-server PROPERTIES COMPILE_FLAGS
			-DDTK_SERVER_DSO_EXPORTS PREFIX "" )
ELSE( WIN32 )
	SET_TARGET_PROPERTIES( libdtk-server PROPERTIES PREFIX "" )
ENDIF( WIN32 )

SET( DTKSERVER_SOURCES dtk-server.cpp usage.cpp parse_args.cpp fileSignal.cpp
		signal.cpp )

IF( UNIX )
	SET( DTKSERVER_SOURCES ${DTKSERVER_SOURCES} daemonize.cpp )
ENDIF( UNIX )

ADD_EXECUTABLE( dtk-server ${DTKSERVER_SOURCES} )
TARGET_LINK_LIBRARIES( dtk-server dtk libdtk-server )

IF( UNIX )
	ADD_EXECUTABLE( fileSignal_test fileSignal.cpp fileSignal_test.cpp )
	TARGET_LINK_LIBRARIES( fileSignal_test dtk )

	ADD_EXECUTABLE( pthread_test pthread_test.cpp )
	TARGET_LINK_LIBRARIES( pthread_test dtk )
ENDIF( UNIX )

ADD_EXECUTABLE( server_test server_test.cpp )
TARGET_LINK_LIBRARIES( server_test dtk )

INSTALL( TARGETS dtk-server libdtk-server RUNTIME DESTINATION bin LIBRARY
		DESTINATION lib ARCHIVE DESTINATION lib PERMISSIONS OWNER_READ
		OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ
		WORLD_EXECUTE )

