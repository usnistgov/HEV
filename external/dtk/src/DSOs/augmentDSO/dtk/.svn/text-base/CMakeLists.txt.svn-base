# CMakeLists.txt to build augmentDSO/dtk

INCLUDE_DIRECTORIES( ${DTK_SOURCE_DIR}/include )

ADD_LIBRARY( buttonNavControl MODULE buttonNavControl.cpp )
TARGET_LINK_LIBRARIES( buttonNavControl dtk )

ADD_LIBRARY( caveDTKInput MODULE caveDTKInput.cpp )
TARGET_LINK_LIBRARIES( caveDTKInput dtk )

ADD_LIBRARY( caveDTKOutput MODULE caveDTKOutput.cpp )
TARGET_LINK_LIBRARIES( caveDTKOutput dtk )

ADD_LIBRARY( caveInputPrint MODULE caveInputPrint.cpp )
TARGET_LINK_LIBRARIES( caveInputPrint dtk )

ADD_LIBRARY( caveInputTestGroup MODULE caveInputTestGroup.cpp )
TARGET_LINK_LIBRARIES( caveInputTestGroup dtk )

ADD_LIBRARY( desktopFlyNav MODULE desktopFlyNav.cpp )
TARGET_LINK_LIBRARIES( desktopFlyNav dtk )
SET_TARGET_PROPERTIES( desktopFlyNav PROPERTIES COMPILE_FLAGS -DFLY )

ADD_LIBRARY( desktopHoverNav MODULE desktopFlyNav.cpp )
TARGET_LINK_LIBRARIES( desktopHoverNav dtk )
SET_TARGET_PROPERTIES( desktopHoverNav PROPERTIES COMPILE_FLAGS -DHOVER )

ADD_LIBRARY( desktopOriginNav MODULE desktopFlyNav.cpp )
TARGET_LINK_LIBRARIES( desktopOriginNav dtk )
SET_TARGET_PROPERTIES( desktopOriginNav PROPERTIES COMPILE_FLAGS -DORIGIN )

ADD_LIBRARY( dtkShmNav MODULE dtkShmNav.cpp )
TARGET_LINK_LIBRARIES( dtkShmNav dtk )

ADD_LIBRARY( dtkShmPivotNavMod MODULE dtkShmPivotNavMod.cpp )
TARGET_LINK_LIBRARIES( dtkShmPivotNavMod dtk )

ADD_LIBRARY( dtkShmLocationNavMod MODULE dtkShmLocationNavMod.cpp )
TARGET_LINK_LIBRARIES( dtkShmLocationNavMod dtk )

ADD_LIBRARY( dummyNav MODULE dummyNav.cpp )
TARGET_LINK_LIBRARIES( dummyNav dtk )

ADD_LIBRARY( dummyPrint MODULE dummyPrint.cpp )
TARGET_LINK_LIBRARIES( dummyPrint dtk )

ADD_LIBRARY( dummyPrintDisplay MODULE dummyPrintDisplay.cpp )
TARGET_LINK_LIBRARIES( dummyPrintDisplay dtk )

ADD_LIBRARY( iconSleepKeyboardMod MODULE iconSleepKeyboardMod.cpp )
TARGET_LINK_LIBRARIES( iconSleepKeyboardMod dtk )

ADD_LIBRARY( navDTKOutput MODULE navDTKOutput.cpp )
TARGET_LINK_LIBRARIES( navDTKOutput dtk )

ADD_LIBRARY( sleepSyncDisplay MODULE sleepSyncDisplay.cpp )
TARGET_LINK_LIBRARIES( sleepSyncDisplay dtk )

ADD_LIBRARY( trackballNav MODULE trackballNav.cpp )
TARGET_LINK_LIBRARIES( trackballNav dtk )

ADD_LIBRARY( wandButtonNav MODULE wandButtonNav.cpp )
TARGET_LINK_LIBRARIES( wandButtonNav dtk )

ADD_LIBRARY( wandJoystickHPRNav MODULE wandJoystickNav.cpp )
TARGET_LINK_LIBRARIES( wandJoystickHPRNav dtk )
SET_TARGET_PROPERTIES( wandJoystickHPRNav PROPERTIES COMPILE_FLAGS -DHPR )

ADD_LIBRARY( wandJoystickNav MODULE wandJoystickNav.cpp )
TARGET_LINK_LIBRARIES( wandJoystickNav dtk )
SET_TARGET_PROPERTIES( wandJoystickNav PROPERTIES COMPILE_FLAGS -DHEADING )

ADD_LIBRARY( wandJoystickPitchNav MODULE wandJoystickNav.cpp )
TARGET_LINK_LIBRARIES( wandJoystickPitchNav dtk )
SET_TARGET_PROPERTIES( wandJoystickPitchNav PROPERTIES COMPILE_FLAGS -DPITCH )

ADD_LIBRARY( wandJoystickRollNav MODULE wandJoystickNav.cpp )
TARGET_LINK_LIBRARIES( wandJoystickRollNav dtk )
SET_TARGET_PROPERTIES( wandJoystickRollNav PROPERTIES COMPILE_FLAGS -DROLL )

ADD_LIBRARY( wandJoystickXYNav MODULE wandJoystickNav.cpp )
TARGET_LINK_LIBRARIES( wandJoystickXYNav dtk )
SET_TARGET_PROPERTIES( wandJoystickXYNav PROPERTIES COMPILE_FLAGS -DXY )

ADD_LIBRARY( wandJoystickOriginNav MODULE wandJoystickNav.cpp )
TARGET_LINK_LIBRARIES( wandJoystickOriginNav dtk )
SET_TARGET_PROPERTIES( wandJoystickOriginNav PROPERTIES COMPILE_FLAGS -DORIGIN )

ADD_LIBRARY( wandJoystickScaleNav MODULE wandJoystickScaleNav.cpp )
TARGET_LINK_LIBRARIES( wandJoystickScaleNav dtk )

ADD_LIBRARY( gameNav MODULE gameNav.cpp )
TARGET_LINK_LIBRARIES( gameNav dtk )

ADD_LIBRARY( joystickXYtransNav MODULE joystickXYtransNav.cpp )
TARGET_LINK_LIBRARIES( joystickXYtransNav dtk )

ADD_LIBRARY( wandShmNav MODULE wandShmNav.cpp )
TARGET_LINK_LIBRARIES( wandShmNav dtk )

SET_TARGET_PROPERTIES( buttonNavControl caveDTKInput caveDTKOutput
		caveInputPrint caveInputTestGroup desktopFlyNav desktopHoverNav
		desktopOriginNav dtkShmNav dtkShmPivotNavMod dtkShmLocationNavMod
		dummyNav dummyPrint dummyPrintDisplay iconSleepKeyboardMod navDTKOutput
		sleepSyncDisplay trackballNav wandButtonNav wandJoystickHPRNav
		wandJoystickNav wandJoystickPitchNav wandJoystickRollNav
		wandJoystickXYNav wandJoystickOriginNav wandJoystickScaleNav gameNav
		joystickXYtransNav wandShmNav PROPERTIES PREFIX "" )

INSTALL( TARGETS buttonNavControl caveDTKInput caveDTKOutput
		caveInputPrint caveInputTestGroup desktopFlyNav desktopHoverNav
		desktopOriginNav dtkShmNav dtkShmPivotNavMod dtkShmLocationNavMod
		dummyNav dummyPrint dummyPrintDisplay iconSleepKeyboardMod navDTKOutput
		sleepSyncDisplay trackballNav wandButtonNav wandJoystickHPRNav
		wandJoystickNav wandJoystickPitchNav wandJoystickRollNav
		wandJoystickXYNav wandJoystickOriginNav wandJoystickScaleNav gameNav
		joystickXYtransNav wandShmNav
		LIBRARY DESTINATION etc/dtk/augmentDSO PERMISSIONS OWNER_READ
		OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE )

