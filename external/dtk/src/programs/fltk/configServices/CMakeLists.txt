# CMakeLists.txt to build programs/fltk/configServices

INCLUDE_DIRECTORIES( ${DTK_SOURCE_DIR}/include ${DTK_FLTK_INC_DIRS}
		${DTK_FLTK_INC_DIRS} )
LINK_DIRECTORIES( ${DTK_FLTK_LINK_DIRS} )
ADD_EXECUTABLE( dtk-configServices MACOSX_BUNDLE config_widget.cpp
		dtk-configServices.cpp mainWindow.cpp )
TARGET_LINK_LIBRARIES( dtk-configServices dtk ${DTK_FLTK_LIBS} )
SET_TARGET_PROPERTIES( dtk-configServices PROPERTIES
		LINK_FLAGS "${DTK_FLTK_LINK_FLAGS}" )
IF( ${DTK_ARCH_UPPER} MATCHES "DARWIN" )
	SET( CUR_TARGET dtk-configServices )
	CONFIGURE_FILE( ${DTK_SOURCE_DIR}/programs/fltk/prog2app.in
			${CMAKE_CURRENT_BINARY_DIR}/prog2app @ONLY )
	ADD_CUSTOM_COMMAND( TARGET ${CUR_TARGET} POST_BUILD
			COMMAND mv prog2app ${CUR_TARGET}
			COMMAND chmod +x ${CUR_TARGET} VERBATIM )
ENDIF( ${DTK_ARCH_UPPER} MATCHES "DARWIN" )

