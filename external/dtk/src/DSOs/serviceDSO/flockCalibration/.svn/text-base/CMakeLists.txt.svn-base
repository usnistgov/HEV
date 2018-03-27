# CMakeLists.txt to build flockCalibration

INCLUDE_DIRECTORIES( ${DTK_SOURCE_DIR}/include SWA/SphereMean SWA/VrMath )
ADD_LIBRARY( flockCalibration MODULE flockCalibration.cpp tetraUtils.cpp
		rigidXform.cpp tetraUtils.h rigidXform.h
		SWA/SphereMean/SphereTriDiag.cpp SWA/SphereMean/try.cpp
		SWA/SphereMean/SphereMean.cpp
		SWA/SphereMean/sphericalWeightedAverage.cpp SWA/VrMath/LinearR3.cpp
		SWA/VrMath/LinearR4.cpp SWA/VrMath/Quaternion.cpp
		SWA/VrMath/LinearR2.cpp SWA/VrMath/PolynomialRC.cpp )
TARGET_LINK_LIBRARIES( flockCalibration dtk )
SET_TARGET_PROPERTIES( flockCalibration PROPERTIES PREFIX "" )
INSTALL( TARGETS flockCalibration LIBRARY DESTINATION etc/dtk/serviceDSO
		PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ
		GROUP_EXECUTE WORLD_READ WORLD_EXECUTE )
INSTALL( FILES configFiles/rainbow.correction.dat
		configFiles/rainbow.trueXYZ.measuredXYZ.AxAngCorr.dat
		DESTINATION etc/dtk/serviceDSO PERMISSIONS OWNER_READ OWNER_WRITE
		GROUP_READ WORLD_READ )

