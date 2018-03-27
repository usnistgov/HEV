# CMakeLists.txt to build programs/augmentManager

INCLUDE_DIRECTORIES( ${DTK_SOURCE_DIR}/include )
ADD_EXECUTABLE( dtk-augmentManager dtk-augmentManager.cpp )
TARGET_LINK_LIBRARIES( dtk-augmentManager dtk )
INSTALL( TARGETS dtk-augmentManager RUNTIME DESTINATION bin PERMISSIONS
		OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE
		WORLD_READ WORLD_EXECUTE )

