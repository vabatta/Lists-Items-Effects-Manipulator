cmake_minimum_required(VERSION 3.21)

# Standards & Flags
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_DEBUG OFF)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

include(GNUInstallDirs)

# Project information & Sources
project(
	ListsItemsEffectsManipulator
	DESCRIPTION "Manipulate lists, items, effects, ... values at runtime."
	VERSION 1.0.0
	LANGUAGES CXX
)

configure_file(
	${CMAKE_CURRENT_SOURCE_DIR}/cmake/version.rc.in
	${CMAKE_CURRENT_BINARY_DIR}/version.rc
	@ONLY
)

configure_file(
	${CMAKE_CURRENT_SOURCE_DIR}/cmake/Plugin.h.in
	${CMAKE_CURRENT_BINARY_DIR}/Plugin.h
	@ONLY
)

configure_file(
	${CMAKE_CURRENT_SOURCE_DIR}/cmake/Plugin.h.in
	${CMAKE_CURRENT_SOURCE_DIR}/Plugin.h
	@ONLY
)

find_path(SRELL_INCLUDE_DIRS "srell.hpp")
find_path(MERGEMAPPER_INCLUDE_DIRS "MergeMapperPluginAPI.h")

set(headers
			include/Runner.h
			include/Modifier.h
			include/StringFilters.h
			include/FormFilters.h
			include/Traits.h
			include/Defs.h
)

set(sources
			src/Runner.cpp
			src/Modifier.cpp
			src/StringFilters.cpp
			src/FormFilters.cpp
			src/Traits.cpp
			src/Defs.cpp

			${MERGEMAPPER_INCLUDE_DIRS}/MergeMapperPluginAPI.cpp
			${CMAKE_CURRENT_BINARY_DIR}/version.rc
)

source_group(
	TREE ${CMAKE_CURRENT_SOURCE_DIR}
	FILES
		Plugin.h
		Plugin.cpp
		${headers}
		${sources}
)

# Configure DLL build
add_subdirectory($ENV{DKUtilPath} DKUtil)
find_package(CommonLibSSE CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)

add_commonlibsse_plugin(${PROJECT_NAME} SOURCES ${headers} ${sources} Plugin.h Plugin.cpp)
target_precompile_headers(${PROJECT_NAME} PRIVATE PCH.h)

target_include_directories(
	${PROJECT_NAME}
	PRIVATE
		${SRELL_INCLUDE_DIRS}
		${MERGEMAPPER_INCLUDE_DIRS}
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
		$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/src>
		$<INSTALL_INTERFACE:src>
	PUBLIC
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

target_link_libraries(
	${PROJECT_NAME}
	PRIVATE
		DKUtil::DKUtil
		spdlog::spdlog
)


# Use `MO2SkyrimSEPath` environment variable to copy the mod in a sub folder inside "mods"
if(DEFINED ENV{MO2SkyrimSEPath} AND IS_DIRECTORY "$ENV{MO2SkyrimSEPath}")
	# Inside "mods"
	set(OUTPUT_FOLDER "$ENV{MO2SkyrimSEPath}/mods/${PROJECT_NAME}")

	# Copy the SKSE plugin .dll files into the SKSE/Plugins/ folder
	set(DLL_FOLDER "${OUTPUT_FOLDER}/SKSE/Plugins")

	message(STATUS "SKSE plugin output folder: ${DLL_FOLDER}")

	add_custom_command(
		TARGET "${PROJECT_NAME}"
		POST_BUILD
		COMMAND "${CMAKE_COMMAND}" -E make_directory "${DLL_FOLDER}"
		COMMAND "${CMAKE_COMMAND}" -E copy_if_different "$<TARGET_FILE:${PROJECT_NAME}>" "${DLL_FOLDER}/$<TARGET_FILE_NAME:${PROJECT_NAME}>"
		VERBATIM
	)

	# If you perform a "Debug" build, also copy .pdb file (for debug symbols)
	if(CMAKE_BUILD_TYPE STREQUAL "Debug")
		add_custom_command(
			TARGET "${PROJECT_NAME}"
			POST_BUILD
			COMMAND "${CMAKE_COMMAND}" -E copy_if_different "$<TARGET_PDB_FILE:${PROJECT_NAME}>" "${DLL_FOLDER}/$<TARGET_PDB_FILE_NAME:${PROJECT_NAME}>"
			VERBATIM
		)
	endif()
endif()
