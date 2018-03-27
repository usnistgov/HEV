# CMakeLists.txt to build augmentDSO/dtkX11

INCLUDE_DIRECTORIES( ${DTK_SOURCE_DIR}/include )
LINK_DIRECTORIES( ${DTK_X11_LINK_DIRS} )

ADD_LIBRARY( keyboardNavControl MODULE keyboardNavControl.cpp )
TARGET_LINK_LIBRARIES( keyboardNavControl dtkX11 )

ADD_LIBRARY( playbackRead MODULE playbackRead.cpp )
TARGET_LINK_LIBRARIES( playbackRead dtkX11 )

ADD_LIBRARY( playbackWrite MODULE playbackWrite.cpp )
TARGET_LINK_LIBRARIES( playbackWrite dtkX11 )

ADD_LIBRARY( shiftButtonsPrint MODULE shiftButtonsPrint.cpp )
TARGET_LINK_LIBRARIES( shiftButtonsPrint dtkX11 )

ADD_LIBRARY( shiftPointerPrint MODULE shiftPointerPrint.cpp )
TARGET_LINK_LIBRARIES( shiftPointerPrint dtkX11 )

ADD_LIBRARY( xkeyboardInput MODULE xkeyboardInput.cpp )
TARGET_LINK_LIBRARIES( xkeyboardInput dtkX11 )

ADD_LIBRARY( xkeyboardPrint MODULE xkeyboardPrint.cpp )
TARGET_LINK_LIBRARIES( xkeyboardPrint dtkX11 )

ADD_LIBRARY( xkeyboardMouseInput MODULE xkeyboardInput.cpp )
TARGET_LINK_LIBRARIES( xkeyboardMouseInput dtkX11 )
SET_TARGET_PROPERTIES( xkeyboardMouseInput PROPERTIES COMPILE_FLAGS "-DMOUSE" )

ADD_LIBRARY( xSimpleWindow MODULE xSimpleWindow.cpp )
TARGET_LINK_LIBRARIES( xSimpleWindow dtkX11 )

SET_TARGET_PROPERTIES( keyboardNavControl playbackRead playbackWrite
	shiftButtonsPrint shiftPointerPrint xkeyboardInput xkeyboardPrint
	xkeyboardMouseInput xSimpleWindow PROPERTIES PREFIX "" )

INSTALL( TARGETS keyboardNavControl playbackRead playbackWrite
	shiftButtonsPrint shiftPointerPrint xkeyboardInput xkeyboardPrint
	xkeyboardMouseInput xSimpleWindow LIBRARY DESTINATION etc/dtk/augmentDSO
	PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE
	WORLD_READ WORLD_EXECUTE )

