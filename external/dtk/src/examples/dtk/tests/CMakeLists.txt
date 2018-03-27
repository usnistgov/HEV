# CMakeLists.txt to build examples/dtk/tests

ADD_SUBDIRECTORY( augmentDSOs )
ADD_SUBDIRECTORY( blockingRead )
ADD_SUBDIRECTORY( dtkTCPSocket )
ADD_SUBDIRECTORY( dtkUDPSocket )

IF( NOT WIN32 )
	ADD_SUBDIRECTORY( dtkFilter )
	IF( NOT APPLE )
		ADD_SUBDIRECTORY( dtkDSO_ )
		ADD_SUBDIRECTORY( shmWriteSignalCatcher )
	ENDIF( NOT APPLE )
ENDIF( NOT WIN32 )

INCLUDE_DIRECTORIES( ${DTK_SOURCE_DIR}/include )

IF( NOT WIN32 )
	ADD_EXECUTABLE( test-getAddress getAddress.cpp )
	TARGET_LINK_LIBRARIES( test-getAddress dtk )

	ADD_EXECUTABLE( test-getAddress2 getAddress2.cpp )
	TARGET_LINK_LIBRARIES( test-getAddress2 dtk )

	ADD_EXECUTABLE( test-pthread_rwlock_test pthread_rwlock_test.cpp )
	TARGET_LINK_LIBRARIES( test-pthread_rwlock_test dtk )
ENDIF( NOT WIN32 )

ADD_EXECUTABLE( test-dtkBase dtkBase.cpp )
TARGET_LINK_LIBRARIES( test-dtkBase dtk )

ADD_EXECUTABLE( test-dtkMatrix dtkMatrix.cpp )
TARGET_LINK_LIBRARIES( test-dtkMatrix dtk )

ADD_EXECUTABLE( test-sharedMem_print sharedMem_print.cpp )
TARGET_LINK_LIBRARIES( test-sharedMem_print dtk )

ADD_EXECUTABLE( test-dtkClient dtkClient.cpp )
TARGET_LINK_LIBRARIES( test-dtkClient dtk )

ADD_EXECUTABLE( test-dtkMessage dtkMessage.cpp )
TARGET_LINK_LIBRARIES( test-dtkMessage dtk )

ADD_EXECUTABLE( test-dtkColor dtkColor.cpp )
TARGET_LINK_LIBRARIES( test-dtkColor dtk )

ADD_EXECUTABLE( test-dtkSharedMem dtkSharedMem.cpp )
TARGET_LINK_LIBRARIES( test-dtkSharedMem dtk )

ADD_EXECUTABLE( test-old_sharedMem_print old_sharedMem_print.cpp )
TARGET_LINK_LIBRARIES( test-old_sharedMem_print dtk )

ADD_EXECUTABLE( test-dtk_getCommonAddressPort dtk_getCommonAddressPort.cpp )
TARGET_LINK_LIBRARIES( test-dtk_getCommonAddressPort dtk )

ADD_EXECUTABLE( test-dtkTime dtkTime.cpp )
TARGET_LINK_LIBRARIES( test-dtkTime dtk )

IF( WIN32 )
	SET( WIN32_EXTRA_LIBS "comctl32.lib ws2_32.lib" )
ENDIF( WIN32 )

INCLUDE_DIRECTORIES( ${DTK_SOURCE_DIR}/include ${DTK_FLTK_INC_DIRS} )
LINK_DIRECTORIES( ${DTK_FLTK_LINK_DIRS} )
ADD_EXECUTABLE( test-sliderStrings sliderStrings.cpp )
TARGET_LINK_LIBRARIES( test-sliderStrings dtk ${DTK_FLTK_LIBS} )
SET_TARGET_PROPERTIES( test-sliderStrings PROPERTIES
		LINK_FLAGS "${DTK_FLTK_LINK_FLAGS} ${WIN32_EXTRA_LIBS}" )

