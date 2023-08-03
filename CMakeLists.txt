
# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.2)


#Cmake Version 3.2: https://cmake.org/cmake/help/v3.3/release/3.2.html
# Major feature added: continue in foreach() and while() loops
# UTF8 support on windows.
#Cmake version 3.1: https://cmake.org/cmake/help/v3.3/release/3.1.html
# C++ Standard Support
# Fix major generator bug.
# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("CompressedFileLibrary")

option(CFL_BUILD_TESTCFL "Build CFL Test program" ON)
option(CFL_BUILD_EXAMPLES "Build Compressed File Library Examples" ON)
option(CFL_BUILD_UNCFL "Build UnCompressed File Library Utility (uncfl)" ON)
option(CFL_BUILD_MAKECFL "Build Make Compressed File Library Utility (makecfl)" ON)
option(CFL_BUILD_ZLIB "Build Compressed File LIbrary with Zlib Support" OFF) 


if(CFL_BUILD_ZLIB)
	find_package(ZLIB REQUIRED)
endif()

set(COMPRESSED_FILE_LIBRARY_SOURCE
	"src/CFL.cpp"
	"src/CFLDirectoryEntry.cpp"
	"src/CFLLibrary.cpp"
	"src/CFLMaker.cpp"
	"src/CFLResource.cpp"
	"src/CFL_delta16.cpp"
	"src/CFL_delta32.cpp"
	"src/CFL_delta8.cpp"
	"src/CFL_none.cpp"
	"src/CFLResourceFilter.cpp"
)
set( COMPRESSED_FILE_LIBRARY_INCLUDES
	"include/CFL.h"
	"include/CFLDirectoryEntry.h"
	"include/CFLEnumerator.h"
	"include/CFLEnums.h"
	"include/CFLLibrary.h"
	"include/CFLMaker.h"
	"include/CFLResource.h"
	"include/CFLResourceFilter.h"
)

if(CFL_BUILD_ZLIB)
	list(APPEND COMPRESSED_FILE_LIBRARY_SOURCE "src/CFL_zlib.cpp")
endif()
add_library(CompressedFileLibrary STATIC ${COMPRESSED_FILE_LIBRARY_SOURCE})
set_property(TARGET CompressedFileLibrary PROPERTY CXX_STANDARD 98)

target_include_directories(CompressedFileLibrary PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/>
  $<INSTALL_INTERFACE:include/> 
)

if(CFL_BUILD_ZLIB)
	target_link_libraries(CompressedFileLibrary PUBLIC ZLIB::ZLIB)
endif()

if(MSVC)
	target_compile_definitions(CompressedFileLibrary PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()

if(CFL_BUILD_MAKECFL)
	add_executable(makecfl "makecfl/makecfl.cpp" "makecfl/tokenwad.cpp" "makecfl/tokenwad.h")
	set_property(TARGET makecfl PROPERTY CXX_STANDARD 98)
	target_link_libraries(makecfl CompressedFileLibrary)
	if(MSVC)
		#Visual Studio deprecated a lot of old C++ standard functions.
		target_compile_definitions(makecfl PRIVATE _CRT_SECURE_NO_WARNINGS _CRT_NONSTDC_NO_WARNINGS)
	endif()
endif()


if(CFL_BUILD_UNCFL)
	add_executable(uncfl "contrib/uncfl/uncfl.cpp")
	set_property(TARGET uncfl PROPERTY CXX_STANDARD 98)
	target_link_libraries(uncfl CompressedFileLibrary)
	if(MSVC)
		#Visual Studio deprecated a lot of old C++ standard functions.
		target_compile_definitions(uncfl PRIVATE _CRT_SECURE_NO_WARNINGS _CRT_NONSTDC_NO_WARNINGS)
	endif()
endif()


if(CFL_BUILD_TESTCFL)
	# Test Program for Compressed File Library.
	add_executable(TestCFL "testcfl/testcfl.cpp")
	target_link_libraries(TestCFL   CompressedFileLibrary)
	if(MSVC)
		target_compile_definitions(TestCFL   PRIVATE _CRT_SECURE_NO_WARNINGS _CRT_NONSTDC_NO_WARNINGS)
	endif()
endif()

if (false AND CFL_BUILD_EXAMPLES AND CFL_BUILD_MAKECFL)
	#TODO: This needs fixed to work correctly.
	# Example Project.
	add_executable(HelloFromCompressedFileLibrary "example/hello.cpp")
	target_link_libraries(HelloFromCompressedFileLibrary  CompressedFileLibrary)
	if(MSVC)
		target_compile_definitions(HelloFromCompressedFileLibrary  PRIVATE _CRT_SECURE_NO_WARNINGS _CRT_NONSTDC_NO_WARNINGS)
	endif()
	add_custom_target(GenerateHelloCFL_File
					  COMMENT "Generating Hello.cfl")
	add_custom_command(
		OUTPUT hello.cfl
		BYPRODUCTS "${CMAKE_CURRENT_SOURCE_DIR}/example/hello.cfl"

	  WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/example"
	  COMMAND makecfl cfl hello.ini
	  DEPENDS makecfl 
	  VERBATIM)
	add_custom_target(generate_hello_cfl DEPENDS hello.cfl)
	add_dependencies(HelloFromCompressedFileLibrary generate_hello_cfl)
	#TODO: Fix output path for custom command for in-program debug.
endif()