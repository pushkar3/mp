# The name of our project is "mp"

cmake_minimum_required (VERSION 2.6)
project (pallet_planner)

SET(CPACK_PACKAGE_VERSION_MAJOR "0")
SET(CPACK_PACKAGE_VERSION_MINOR "0")
SET(CPACK_PACKAGE_VERSION_PATCH "1")
SET(CPACK_PACKAGE_NAME "pallet_planner")
SET(CPACK_SYSTEM_NAME "i386")

set (CMAKE_VERBOSE_MAKEFILE OFF)

set ( HEADERS
./
./ext
/usr/include
/usr/local/include
)

set ( SRC
3dbpp
3dbpp_settings
3dbpp_test
3dbpp_main
ext/packlist
ext/response
ext/xml_parser
)
 
include_directories (${HEADERS})
add_executable (pallet_planner ${SRC})
target_link_libraries (pallet_planner m pthread)

