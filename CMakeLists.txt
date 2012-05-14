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
3dbpp
3dbpp_input
3dbpp_output
3dbpp_db
#ext/packlist
#ext/response
#ext/xml_parser
)

set ( SIMPLEX_SRC
simplex_main
)

add_library (3dbpp SHARED ${BPP_SRC})
install (FILES 3dbpp.h DESTINATION include)
install (TARGETS 3dbpp LIBRARY DESTINATION lib)

include_directories (${HEADERS})
add_executable (bpp_stat 3dbpp_stat ${BPP_SRC})
target_link_libraries (bpp_stat m pthread tinyxml2)

include_directories (${HEADERS})
add_executable (bpp_pose 3dbpp_pose ${BPP_SRC})
target_link_libraries (bpp_pose m pthread tinyxml2)

include_directories (${HEADERS})
add_executable (bpp_planner 3dbpp_main ${BPP_SRC})
target_link_libraries (bpp_planner m pthread tinyxml2)

include_directories ($ENV{GUROBI_HOME}/include)
link_directories ($ENV{GUROBI_HOME}/lib)
add_executable (simplex_planner ${SIMPLEX_SRC} ${BPP_SRC})
target_link_libraries (simplex_planner m pthread gurobi_c++ gurobi50 gurobi_g++4.1 gurobi_g++4.2 tinyxml2)