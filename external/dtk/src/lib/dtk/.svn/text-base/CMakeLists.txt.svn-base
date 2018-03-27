# CMakeLists.txt to build libdtk

INCLUDE_DIRECTORIES( ${DTK_SOURCE_DIR}/include/dtk ${DTK_SOURCE_DIR}/config )
SET( DTK_SOURCES DSOLoader.cpp FLTKOptions.cpp augment.cpp
		base.cpp basicDisplay.cpp button.cpp client.cpp color.cpp
		configure.cpp coord.cpp dequeuer.cpp display.cpp distribution.cpp
		filter.cpp filterList.cpp inCave.cpp input.cpp locator.cpp manager.cpp
		manager_callbackers.cpp manager_load.cpp matrix.cpp message.cpp
		nav.cpp navList.cpp printEnv.cpp readBuffer.cpp record.cpp rwlock.cpp
		segAddrList.cpp selector.cpp service.cpp configService.cpp
		calibration.cpp sharedMem.cpp sharedMem_alloc.cpp
		sharedMem_connect.cpp sharedMem_create.cpp sharedMem_destroy.cpp
		sharedMem_filter.cpp sharedMem_mapFile.cpp sharedMem_misc.cpp
		sharedMem_network.cpp sharedMem_recreate.cpp sharedMem_rw.cpp
		sharedMem_writeList.cpp sharedTime.cpp socket.cpp socketList.cpp
		splice.cpp tcpSocket.cpp tempname.cpp time.cpp udpSocket.cpp
		usbInput.cpp util_getCommonTCPAddress.cpp utils.cpp valuator.cpp
		vec3.cpp conditional.cpp
)

IF( WIN32 )
	SET( DTK_SOURCES ${DTK_SOURCES} gettimeofday.cpp )
	ADD_DEFINITIONS( -DLIBDTK_EXPORTS )
	SET( DTK_WIN32_LIBS ws2_32.lib )
ELSE( WIN32 )
	SET( DTK_SOURCES ${DTK_SOURCES} realtime.cpp VRCOSharedMem.cpp )
	SET( DTK_WIN32_LIBS )
ENDIF( WIN32 )

ADD_LIBRARY( dtk ${DTK_SOURCES} )
TARGET_LINK_LIBRARIES( dtk ${DTK_OTHER_LIBS} ${DTK_WIN32_LIBS} )
SET_TARGET_PROPERTIES( dtk PROPERTIES VERSION ${LIBDTK_VERSION} SOVERSION
		${LIBDTK_MAJOR_VERSION} )

INSTALL( TARGETS dtk RUNTIME DESTINATION bin LIBRARY DESTINATION lib ARCHIVE
		DESTINATION lib PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
		GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE )

