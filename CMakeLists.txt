############################################################################
# CMakeLists.txt
# Copyright (C) 2014  Belledonne Communications, Grenoble France
#
############################################################################
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
#
############################################################################

cmake_minimum_required(VERSION 3.0)
project(MSSILK C)
set(VERSION "1.0.1")


find_package(Mediastreamer2 REQUIRED)

set(FLAVOUR "FIX")
file(GLOB SILK_SOURCE_FILES "sdk/SILK_SDK_SRC_v1.0.9/SILK_SDK_SRC_${FLAVOUR}_v1.0.9/src/*.c")


include_directories(
	${MEDIASTREAMER2_INCLUDE_DIRS}
	sdk/SILK_SDK_SRC_v1.0.9/SILK_SDK_SRC_${FLAVOUR}_v1.0.9/interface
)

add_definitions(-DVERSION=\"${VERSION}\")


set(LIBS ${MEDIASTREAMER2_LIBRARIES})

set(SOURCE_FILES
	${SILK_SOURCE_FILES}
	silk_dec.c
	silk_enc.c
)

add_library(mssilk SHARED ${SOURCE_FILES})
set_target_properties(mssilk PROPERTIES VERSION 0)
if(MSVC)
	if(CMAKE_BUILD_TYPE STREQUAL "Debug")
		install(FILES ${CMAKE_CURRENT_BINARY_DIR}/Debug/libmssilk.pdb
			DESTINATION bin
			PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
		)
	endif()
	set_target_properties(mssilk PROPERTIES PREFIX "lib")
endif()
target_link_libraries(mssilk ${LIBS})

install(TARGETS mssilk
	RUNTIME DESTINATION lib/mediastreamer/plugins
	LIBRARY DESTINATION lib/mediastreamer/plugins
	ARCHIVE DESTINATION lib/mediastreamer/plugins
	PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
)
