INCLUDE(FindPkgConfig)
PKG_CHECK_MODULES(PC_SCADA scada)

FIND_PATH(
    SCADA_INCLUDE_DIRS
    NAMES scada/api.h
    HINTS $ENV{SCADA_DIR}/include
        ${PC_SCADA_INCLUDEDIR}
    PATHS ${CMAKE_INSTALL_PREFIX}/include
          /usr/local/include
          /usr/include
)

FIND_LIBRARY(
    SCADA_LIBRARIES
    NAMES gnuradio-scada
    HINTS $ENV{SCADA_DIR}/lib
        ${PC_SCADA_LIBDIR}
    PATHS ${CMAKE_INSTALL_PREFIX}/lib
          ${CMAKE_INSTALL_PREFIX}/lib64
          /usr/local/lib
          /usr/local/lib64
          /usr/lib
          /usr/lib64
)

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(SCADA DEFAULT_MSG SCADA_LIBRARIES SCADA_INCLUDE_DIRS)
MARK_AS_ADVANCED(SCADA_LIBRARIES SCADA_INCLUDE_DIRS)

