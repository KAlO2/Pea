# use cmake version 3.8+ for C++17
set(CMAKE_CXX_STANDARD 17)
#set(CATCH_HOME "G:/Source/Catch2/")

set(PEA_TEST_DIR ${PEA_HOME}/test)
message(STATUS "PEA_TEST_DIR = ${PEA_TEST_DIR}")

include_directories(${PROJECT_SOURCE_DIR}
	${PEA_HOME}/pea/include
	${PEA_HOME}/pea/src
	${PEA_HOME}/pea/test
)

#file(GLOB PEA_TEST_SOURCE ${PEA_TEST_DIR}/*.cpp)
set(PEA_TEST_SOURCE
	test_Camera.cpp
	test_geometry.cpp
	test_Path.cpp
	test_image.cpp
	test_intersection.cpp
	test_math.cpp
	test_utility.cpp
	test_IndexBuffer.cpp
	main.cpp
)
add_executable(test_pea ${PEA_TEST_SOURCE})

target_link_libraries(test_pea
	pea
#	c++_shared
)

function(createTestCase NAME)
	set(SAMPLE_NAME test_${NAME})
	add_executable(${SAMPLE_NAME} ${SAMPLE_NAME}.cpp)

	add_test( 
		NAME ${SAMPLE_NAME}
		COMMAND $<TARGET_FILE:${SAMPLE_NAME}>)
endfunction()

if(WIN32)
	include(${CMAKE_SOURCE_DIR}/cmake/MSVCConfig.cmake)
else()
	include(${CMAKE_SOURCE_DIR}/cmake/GCCConfig.cmake)
endif()

set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS_DEBUG})
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

include_directories(
	${CMAKE_SOURCE_DIR}/include/
	${GLUT_INCLUDE_DIR}
)

link_libraries(pea
	${OPENGL_LIBRARY}
	${GLUT_LIBRARY}
	${GLEW_LIBRARY}
	${PNG_LIBRARY}
	${JPEG_LIBRARY}
)

createTestCase(ConvexHull)
createTestCase(noise)

#[[
createTestCase(math)
createTestCase(util)
createTestCase(image)
createTestCase(model)
createTestCase(geometry)

createTestCase(Camera)
#]]


