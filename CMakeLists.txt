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
/usr/include/c++/4.4.3
)

set ( BPP_SRC
3dbpp_main
ext/packlist
ext/response
ext/xml_parser
)

set ( SIMPLEX_SRC
simplex_main
)

include_directories (${HEADERS})

add_executable (bpp_planner ${BPP_SRC})
target_link_libraries (bpp_planner m pthread)

add_executable (simplex_planner ${SIMPLEX_SRC})
target_link_libraries (simplex_planner m pthread)