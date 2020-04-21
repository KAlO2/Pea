#file(GLOB GROUP_NAMES RELATIVE ${CMAKE_CURRENT_LIST_FILE} ${CMAKE_CURRENT_LIST_FILE}/*)
set(GROUP_NAMES content math geometry graphics io physics scene opengl util view widget)

foreach(group_name ${GROUP_NAMES})
	# file(GLOB variable [RELATIVE path] [globbing expressions]...)
	# file(GLOB_RECURSE variable [RELATIVE path] [FOLLOW_SYMLINKS] [globbing expressions]...)
	file(GLOB_RECURSE GROUP_INCLUDE  ./${group_name}/*.h ./${group_name}/*.inl)
	file(GLOB_RECURSE GROUP_SOURCE   ./${group_name}/*.cpp)
	
	source_group(${group_name} FILES ${GROUP_INCLUDE})
	source_group(${group_name} FILES ${GROUP_SOURCE})
	
	set(PEA_HEADER ${PEA_HEADER} ${GROUP_INCLUDE})
	set(PEA_SOURCE ${PEA_SOURCE} ${GROUP_INCLUDE} ${GROUP_SOURCE})
endforeach()

# https://cmake.org/cmake/help/v3.0/command/link_directories.html
# Note that link_directories command is rarely necessary.
# The LINK_DIRECTORIES has to be before the ADD_EXECUTABLE. http://public.kitware.com/pipermail/cmake/2006-March/008599.html
#link_directories(${OPENCV_LIBDIR})

install(FILES ${HEADER_FILES}
	DESTINATION include/${PROJECT_NAME}/
)

if(JPEG_FOUND)
	add_definitions(-DHAVE_JPEG)
	include_directories(${JPEG_INCLUDE_DIR})
else()
	remove_definitions(-DHAVE_JPEG)
	list(REMOVE_ITEM SOURCE graphics/Image_JPG.cpp)
endif()


include_directories(
	${PNG_INCLUDE_DIR}
	${FREETYPE_INCLUDE_DIRS}
	${GLEW_INCLUDE_DIR}
	${GLUT_INCLUDE_DIR}
	include
)

include(${CMAKE_SOURCE_DIR}/cmake/platform.cmake)

# https://en.cppreference.com/w/cpp/filesystem
# GNU implementation prior to 9.1 requires linking with -lstdc++fs and LLVM implementation prior to 
# LLVM 9.0 requires linking with -lc++fs
# if(gcc < 9.1 || clang < 9.0)
#     CPP_LIBARAY += stdc++fs
if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
#	if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS 9.0)
		set(CPP_LIBARAY ${CPP_LIBARAY} stdc++fs)
#	endif()
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
	if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS 9.1)
		set(CPP_LIBARAY ${CPP_LIBARAY} stdc++fs)
	endif()
endif()

#link_directories(
#	${THIRD_PARTY_DIR}/glut/lib/
#	)
set(TARGET_NAME pea)

add_library(${TARGET_NAME} SHARED ${PEA_SOURCE})

target_link_libraries(${TARGET_NAME}
	${CPP_LIBARAY}
	
	${OPENGL_LIBRARY} # ${OPENGL_LIBRARY} == ${OPENGL_gl_LIBRARY} + ${OPENGL_glu_LIBRARY}
	${GLUT_LIBRARY}
	${GLEW_LIBRARY}
	${GLFW3_LIBRARY}
	
	${OPENAL_LIBRARY}
	${ALUT_LIBRARY}
	${VORBIS_LIBRARIES}

	${PNG_LIBRARY}
	${JPEG_LIBRARY}
	${FREETYPE_LIBRARY}
#	${SDL2_LIBRARY}
#	asan
)

set_property(TARGET pea PROPERTY CXX_STANDARD 17)
# set(CXX_STANDARD 17)

#[[ # fake a block comment
add_library(${TARGET_NAME}_static STATIC ${PEA_SOURCE})

# Make both the static and shared libraries in one build with CMake.
set_target_properties(${TARGET_NAME}_static PROPERTIES OUTPUT_NAME ${TARGET_NAME})

install(DIRECTORY include DESTINATION include)
install(TARGETS ${TARGET_NAME} ${TARGET_NAME}_static
	RUNTIME DESTINATION bin
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib
	)

install(TARGETS ${TARGET_NAME} ${TARGET_NAME}_static
	DESTINATION ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE})
# create a resource directory link for out of source build
# https://cmake.org/Wiki/CMake_FAQ#What_is_an_.22out-of-source.22_build.3F
# if(NOT "${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
# 	add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
# 		COMMAND ${CMAKE_COMMAND} -E create_symlink "${CMAKE_SOURCE_DIR}/res" "${CMAKE_BINARY_DIR}/res"
# 		COMMENT "Generating symbolic link for resource directory")
# endif()
#]]
