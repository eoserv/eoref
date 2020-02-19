cmake_minimum_required(VERSION 3.8)
project(eoref-client)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(EOREF_STATIC_LIBS "Use static libraries" OFF)

# -----------

if (CMAKE_BUILD_TYPE EQUAL "Debug" OR CMAKE_BUILD_TYPE EQUAL "RelWithDebInfo")
	set(DEBUG_BUILD 1)
else()
	set(DEBUG_BUILD 0)
endif()


# -----------
# endless.exe

set(ALSMART_SOURCES
	lib/alsmart/alsmart.hpp
	lib/alsmart/font.hpp
	lib/alsmart/memfile.hpp
	lib/alsmart/native_dialog.hpp
)

set(CIO_SOURCES
	lib/cio/cio.cpp
	lib/cio/cio.hpp
)

set(UTIL_SOURCES
	lib/util/ascii.hpp
	lib/util/bind_front.hpp
	lib/util/int_pack.hpp
	lib/util/overload.hpp
	lib/util/signal.hpp
)

add_executable(endless WIN32
	${ALSMART_SOURCES}
	${CIO_SOURCES}
	${UTIL_SOURCES}

	src/data/dib_reader.cpp
	src/data/dib_reader.hpp
	src/data/eo_pub_protocol.hpp
	src/data/eo_stream.cpp
	src/data/eo_stream.hpp
	src/data/eo_types.hpp
	src/data/full_emf.cpp
	src/data/full_emf.hpp
	src/data/pe_reader.cpp
	src/data/pe_reader.hpp

	src/engine/engine_allegro.cpp
	src/engine/engine_allegro.hpp

	src/gfx/color.hpp
	src/gfx/draw_buffer.cpp
	src/gfx/draw_buffer.hpp
	src/gfx/draw_buffer_debug.cpp
	src/gfx/draw_buffer_debug.hpp
	src/gfx/gfx_pool.cpp
	src/gfx/gfx_pool.hpp

	src/packet/eo_packets.cpp
	src/packet/eo_packets.hpp
	src/packet/eo_protocol.hpp
	src/packet/packet_processor.cpp
	src/packet/packet_processor.hpp
	src/packet/packet_base.cpp
	src/packet/packet_base.hpp

	src/app.cpp
	src/app.hpp
	src/config.cpp
	src/config.hpp
	src/data.cpp
	src/data.hpp
	src/engine.cpp
	src/engine.hpp
	src/game.cpp
	src/game.hpp
	src/main.cpp
	src/netclient.cpp
	src/netclient.hpp
	src/trace.hpp
)

target_include_directories(endless PUBLIC src lib)

# -----------

if (CMAKE_CROSSCOMPILING AND WIN32)
	# CMake is too daft to just try running the executable
	set(CMAKE_CROSSCOMPILING_EMULATOR wine)
endif()

set(GENERATED_SRC_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated")

# -----------
# Convert resources

add_subdirectory(tools/bin2c)

set(EOFONT1_BMP "${CMAKE_CURRENT_SOURCE_DIR}/resources/eofont1.bmp")
set(EOFONT1_CPP "${GENERATED_SRC_DIR}/resources/eofont1.cpp")
set(EOFONT2_BMP "${CMAKE_CURRENT_SOURCE_DIR}/resources/eofont2.bmp")
set(EOFONT2_CPP "${GENERATED_SRC_DIR}/resources/eofont2.cpp")

file(MAKE_DIRECTORY "${GENERATED_SRC_DIR}/resources")

add_custom_command(
	OUTPUT ${EOFONT1_CPP}
	COMMAND bin2c "${EOFONT1_BMP}" "${EOFONT1_CPP}" eofont1
	DEPENDS "${EO_PROTOCOL_TXT}" "${GENERATED_SRC_DIR}/resources"
	WORKING_DIRECTORY "${GENERATED_SRC_DIR}/resources"
)

add_custom_command(
	OUTPUT ${EOFONT2_CPP}
	COMMAND bin2c "${EOFONT2_BMP}" "${EOFONT2_CPP}" eofont2
	DEPENDS "${EO_PROTOCOL_TXT}" "${GENERATED_SRC_DIR}/resources"
	WORKING_DIRECTORY "${GENERATED_SRC_DIR}/resources"
)

add_custom_target(eofont1 DEPENDS ${EOFONT1_CPP})
add_custom_target(eofont2 DEPENDS ${EOFONT2_CPP})

add_library(eo_resources STATIC
	${EOFONT1_CPP}
	${EOFONT2_CPP}
	resources/eo_resources.hpp
)

# Link to endless.exe
target_link_libraries(endless PUBLIC eo_resources)

# -----------
# Generate the protocol files using eo_protocol_parser

add_subdirectory(tools/eo_protocol_parser)

set(EO_PROTOCOL_TXT "${CMAKE_CURRENT_SOURCE_DIR}/protocol/eo_protocol.txt")
set(EO_PUB_PROTOCOL_TXT "${CMAKE_CURRENT_SOURCE_DIR}/protocol/pub_protocol.txt")

set(GENERATED_EO_PROTOCOL_FILES
	"${GENERATED_SRC_DIR}/eo_protocol/enums.hpp"
	"${GENERATED_SRC_DIR}/eo_protocol/structs.hpp"
	"${GENERATED_SRC_DIR}/eo_protocol/structs.cpp"
	"${GENERATED_SRC_DIR}/eo_protocol/client.hpp"
	"${GENERATED_SRC_DIR}/eo_protocol/client.cpp"
	"${GENERATED_SRC_DIR}/eo_protocol/client_packets.tpp"
	"${GENERATED_SRC_DIR}/eo_protocol/server.hpp"
	"${GENERATED_SRC_DIR}/eo_protocol/server.cpp"
	"${GENERATED_SRC_DIR}/eo_protocol/server_packets.tpp"
)

set(GENERATED_EO_PUB_PROTOCOL_FILES
	"${GENERATED_SRC_DIR}/eo_protocol/pub_enums.hpp"
	"${GENERATED_SRC_DIR}/eo_protocol/pub_structs.hpp"
	"${GENERATED_SRC_DIR}/eo_protocol/pub_structs.cpp"
)

file(MAKE_DIRECTORY "${GENERATED_SRC_DIR}/eo_protocol")

add_custom_command(
	OUTPUT ${GENERATED_EO_PROTOCOL_FILES}
	COMMAND eo_protocol_parser -net "${EO_PROTOCOL_TXT}"
	DEPENDS "${EO_PROTOCOL_TXT}" "${GENERATED_SRC_DIR}/eo_protocol"
	WORKING_DIRECTORY "${GENERATED_SRC_DIR}/eo_protocol"
)

add_custom_command(
	OUTPUT ${GENERATED_EO_PUB_PROTOCOL_FILES}
	COMMAND eo_protocol_parser -pub "${EO_PUB_PROTOCOL_TXT}"
	DEPENDS "${EO_PUB_PROTOCOL_TXT}" "${GENERATED_SRC_DIR}/eo_protocol"
	WORKING_DIRECTORY "${GENERATED_SRC_DIR}/eo_protocol"
)

add_custom_target(eo_protocol_generated
	DEPENDS ${GENERATED_EO_PROTOCOL_FILES}
)

add_custom_target(eo_pub_protocol_generated
	DEPENDS ${GENERATED_EO_PUB_PROTOCOL_FILES}
)

add_library(eo_protocol STATIC
	${GENERATED_EO_PROTOCOL_FILES}
	${EO_PROTOCOL_TXT}
)

add_dependencies(eo_protocol eo_protocol_generated)

target_include_directories(eo_protocol PRIVATE src "${GENERATED_SRC_DIR/eo_protocol}")
target_include_directories(eo_protocol PUBLIC src/packet "${GENERATED_SRC_DIR}")

add_library(eo_pub_protocol STATIC
	${GENERATED_EO_PUB_PROTOCOL_FILES}
	${EO_PUB_PROTOCOL_TXT}
)

add_dependencies(eo_pub_protocol eo_pub_protocol_generated)

target_include_directories(eo_pub_protocol PRIVATE src "${GENERATED_SRC_DIR/eo_protocol}")
target_include_directories(eo_pub_protocol PUBLIC src/packet "${GENERATED_SRC_DIR}")

# Link to endless.exe
target_link_libraries(endless PUBLIC eo_protocol eo_pub_protocol)

# -----------
# CMake-based dependencies

# TODO: Make installed version usable
add_subdirectory(third-party/fmt)

# Link to endless.exe
target_link_libraries(endless PUBLIC fmt)

# -----------
# find_package / pkg-config dependencies

set(USE_PKGCONFIG 1)

# Dependency: Threads

find_package(Threads REQUIRED)

target_link_libraries(endless PRIVATE Threads::Threads)

# Dependency: Asio

find_package(Asio 1.12 REQUIRED)

target_include_directories(endless PRIVATE ${ASIO_INCLUDE_DIR})
target_compile_definitions(endless PRIVATE ASIO_STANDALONE)

if (WIN32)
	target_link_libraries(endless PRIVATE ws2_32)
endif()

# Dependency: Allegro

set(ALLEGRO_ADDONS dialog image primitives memfile)

if(USE_PKGCONFIG)

	find_package(PkgConfig)

	if (EOREF_STATIC_LIBS)
		if (DEBUG_BUILD)
			pkg_search_module(ALLEGRO5 REQUIRED allegro-static-debug-5 allegro-static-5)
			foreach (ADDON ${ALLEGRO_ADDONS})
				string(TOUPPER ${ADDON} ADDON_UPPER)
				pkg_search_module(ALLEGRO5_${ADDON_UPPER} REQUIRED allegro_${ADDON}-static-debug-5 allegro_${ADDON}-static-5)
			endforeach()
		else()
			pkg_search_module(ALLEGRO5 REQUIRED allegro-static-5)
			foreach (ADDON ${ALLEGRO_ADDONS})
				string(TOUPPER ${ADDON} ADDON_UPPER)
				pkg_search_module(ALLEGRO5_${ADDON_UPPER} REQUIRED allegro_${ADDON}-static-5)
			endforeach()
		endif()
	else()
		if (DEBUG_BUILD)
			pkg_search_module(ALLEGRO5 REQUIRED allegro-debug-5 allegro-static-debug-5 allegro-5 allegro-static-5)
			foreach (ADDON ${ALLEGRO_ADDONS})
				string(TOUPPER ${ADDON} ADDON_UPPER)
				pkg_search_module(ALLEGRO5_${ADDON_UPPER} REQUIRED allegro_${ADDON}-debug-5 allegro_${ADDON}-static-debug-5 allegro_${ADDON}-5 allegro_${ADDON}-static-5)
			endforeach()
		else()
			pkg_search_module(ALLEGRO5 REQUIRED allegro-5 allegro-static-5)
			foreach (ADDON ${ALLEGRO_ADDONS})
				string(TOUPPER ${ADDON} ADDON_UPPER)
				pkg_search_module(ALLEGRO5_${ADDON_UPPER} REQUIRED allegro_${ADDON}-5 allegro_${ADDON}-static-5)
			endforeach()
		endif()
	endif()

	target_include_directories(endless SYSTEM PRIVATE ${ALLEGRO5_INCLUDE_DIRS})
	target_link_directories(endless PRIVATE ${ALLEGRO5_LIBRARY_DIRS})

	if (EOREF_STATIC_LIBS)
		target_link_libraries(endless PRIVATE ${ALLEGRO5_STATIC_LIBRARIES} ${ALLEGRO5_DIALOG_STATIC_LIBRARIES} ${ALLEGRO5_IMAGE_STATIC_LIBRARIES} ${ALLEGRO5_PRIMITIVES_STATIC_LIBRARIES} ${ALLEGRO5_MEMFILE_STATIC_LIBRARIES})
	else()
		target_link_libraries(endless PRIVATE ${ALLEGRO5_LIBRARIES} ${ALLEGRO5_DIALOG_LIBRARIES} ${ALLEGRO5_IMAGE_LIBRARIES} ${ALLEGRO5_PRIMITIVES_LIBRARIES} ${ALLEGRO5_MEMFILE_LIBRARIES})
	endif()

else()

# pkg-config not generally available on Windows
# however FindAllegro5 is yet to be adapted to include appropriate static libs

# find_package(Allegro5 REQUIRED)

# target_include_directories(endless SYSTEM PRIVATE ${ALLEGRO5_INCLUDE_DIR})
# target_link_libraries(endless PRIVATE Threads::Threads ${ALLEGRO5_LIBRARY} ${ALLEGRO5_DIALOG_LIBRARY} ${ALLEGRO5_IMAGE_LIBRARY} ${ALLEGRO5_PRIMITIVES_LIBRARY})

endif()

# -----------
# Link required data files in build directory

include(symlink_datafiles)

symlink_datafiles(endless ${CMAKE_CURRENT_SOURCE_DIR}/config ${CMAKE_CURRENT_BINARY_DIR}/config)
symlink_datafiles(endless ${CMAKE_CURRENT_SOURCE_DIR}/gfx ${CMAKE_CURRENT_BINARY_DIR}/gfx)

