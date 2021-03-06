## This is a CMake file, part of Unidata's netCDF package.
# Copyright 2012-2013, see the COPYRIGHT file for more information.


##################################
# Set Project Properties
##################################

#Minimum required CMake Version
cmake_minimum_required(VERSION 2.8.8)

#Project Name
project(netCDF C)
set(PACKAGE "netCDF" CACHE STRING "")
#Project Version
SET(netCDF_VERSION_MAJOR 4)
SET(netCDF_VERSION_MINOR 3)
SET(netCDF_VERSION_PATCH 1)
SET(netCDF_VERSION_NOTE "")
SET(netCDF_VERSION ${netCDF_VERSION_MAJOR}.${netCDF_VERSION_MINOR}.${netCDF_VERSION_PATCH}${netCDF_VERSION_NOTE})
SET(VERSION ${netCDF_VERSION})
SET(netCDF_SO_VERSION 7.2.0)

# Get system configuration,
# Use it to determine osname,
# os release, cpu. These will be used
# when committing to CDash.
find_program(UNAME NAMES uname)
IF(UNAME)
macro(getuname name flag)
        exec_program("${UNAME}" ARGS "${flag}" OUTPUT_VARIABLE "${name}")
endmacro(getuname)
getuname(osname -s)
getuname(osrel  -r)
getuname(cpu    -m)
set(BUILDNAME        "${osname}-${osrel}-${cpu}" CACHE STRING "Build name variable for CDash")
ENDIF()

# For CMAKE_INSTALL_LIBDIR
INCLUDE(GNUInstallDirs)

IF(MSVC)
  SET(GLOBAL PROPERTY USE_FOLDERS ON)
ENDIF()


#Add custom CMake Module
SET (CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules/"
    CACHE INTERNAL "Location of our custom CMake modules.")

# auto-configure style checks, other CMake modules.
INCLUDE (${CMAKE_ROOT}/Modules/CheckLibraryExists.cmake)
INCLUDE (${CMAKE_ROOT}/Modules/CheckIncludeFile.cmake)
INCLUDE (${CMAKE_ROOT}/Modules/CheckTypeSize.cmake)
INCLUDE (${CMAKE_ROOT}/Modules/CheckFunctionExists.cmake)
INCLUDE (${CMAKE_ROOT}/Modules/CheckCXXSourceCompiles.cmake)
INCLUDE (${CMAKE_ROOT}/Modules/TestBigEndian.cmake)
INCLUDE (${CMAKE_ROOT}/Modules/CheckSymbolExists.cmake)
INCLUDE (${CMAKE_ROOT}/Modules/GetPrerequisites.cmake)
FIND_PACKAGE(PkgConfig QUIET)

# Enable 'dist and distcheck'.
# File adapted from http://ensc.de/cmake/FindMakeDist.cmake
FIND_PACKAGE(MakeDist)
# End 'enable dist and distcheck'

# Set the build type.
IF (NOT CMAKE_BUILD_TYPE)
	SET (CMAKE_BUILD_TYPE DEBUG CACHE STRING "Choose the type of build, options are: None, Debug, Release."
		FORCE)
ENDIF()

################################
# End Project Properties
################################


################################
# Set CTest Properties
################################

ENABLE_TESTING()
INCLUDE(CTest)

# Copy the CTest customization file into binary directory, as required.
FILE(COPY ${CMAKE_SOURCE_DIR}/CTestCustom.cmake DESTINATION ${CMAKE_BINARY_DIR})

# Set Memory test program for non-MSVC based builds.
# Assume valgrind for now.
IF(NOT MSVC)
	SET (CTEST_MEMORYCHECK_COMMAND valgrind CACHE STRING "")
ENDIF()

# Set variable to define the build type.
INCLUDE(GenerateExportHeader)

################################
# End CTest Properties
################################


################################
# Compiler Configuration
################################


# Default to shared libs on.
OPTION(BUILD_SHARED_LIBS "Configure netCDF as a shared library." ON)
SET (LIB_TYPE STATIC)
IF (BUILD_SHARED_LIBS)
	SET(LIB_TYPE SHARED)
	IF(CMAKE_COMPILER_IS_GNUCC OR APPLE)
		SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")
	ENDIF()
ENDIF()

# Supress unused variable and parameter warnings, for the time being,
# on GCC.
#
# Also, set some other default compiler flags.
IF(CMAKE_COMPILER_IS_GNUCC OR APPLE)
  SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -Wall -Wno-unused-variable -Wno-unused-parameter")
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -Wno-unused-variable -Wno-unused-parameter")
ENDIF()

IF(NOT ENABLE_COVERAGE_TESTS)
  SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O2")
  SET(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -O2")
ENDIF()


ADD_DEFINITIONS()

# Supress CRT Warnings.
# Only necessary for Windows
IF(MSVC)
	ADD_DEFINITIONS(-D_CRT_SECURE_NO_WARNINGS)
ENDIF()


################################
# End Compiler Configuration
################################


################################
# Define Utility Macros
################################

# Macro to append files to the EXTRA_DIST files.
SET(EXTRA_DIST "")
MACRO(ADD_EXTRA_DIST files)
  FOREACH(F ${files})
    SET(EXTRA_DIST ${EXTRA_DIST} ${CMAKE_CURRENT_SOURCE_DIR}/${F})
    SET(EXTRA_DIST ${EXTRA_DIST} PARENT_SCOPE)
  ENDFOREACH()
ENDMACRO()

# A basic script used to convert m4 files
FIND_PROGRAM(NC_M4 NAMES m4)
MACRO(GEN_m4 filename)
	IF(NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${filename}.c)
	ADD_CUSTOM_COMMAND(
		OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/${filename}.c
		COMMAND ${NC_M4} 
		ARGS ${CMAKE_CURRENT_SOURCE_DIR}/${filename}.m4 > ${CMAKE_CURRENT_SOURCE_DIR}/${filename}.c
		VERBATIM
	)
	ENDIF()
ENDMACRO(GEN_m4)

# Binary tests, but ones which depend on value of 'TEMP_LARGE' being defined.
MACRO(add_bin_env_temp_large_test prefix F)
	ADD_EXECUTABLE(${prefix}_${F} ${F}.c)
	TARGET_LINK_LIBRARIES(${prefix}_${F} netcdf)
	IF(MSVC)
		SET_TARGET_PROPERTIES(${prefix}_${F} 
			PROPERTIES LINK_FLAGS_DEBUG " /NODEFAULTLIB:MSVCRT"
			)
	ENDIF()


	ADD_TEST(${prefix}_${F} bash "-c" "TEMP_LARGE=${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}/${prefix}_${F}")
	IF(MSVC)
		SET_PROPERTY(TARGET ${prefix}_${F} PROPERTY FOLDER "tests")
	ENDIF()
ENDMACRO()


# Tests which are binary, but depend on a particular environmental variable.
MACRO(add_bin_env_test prefix F)
	ADD_EXECUTABLE(${prefix}_${F} ${F}.c)
	TARGET_LINK_LIBRARIES(${prefix}_${F} netcdf)
	IF(MSVC)
		SET_TARGET_PROPERTIES(${prefix}_${F} 
			PROPERTIES LINK_FLAGS_DEBUG " /NODEFAULTLIB:MSVCRT"
			)
	ENDIF()


	ADD_TEST(${prefix}_${F} bash "-c" "TOPSRCDIR=${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}/${prefix}_${F}")
	IF(MSVC)
		SET_PROPERTY(TARGET ${prefix}_${F} PROPERTY FOLDER "tests")
	ENDIF()
ENDMACRO()

# Build a binary used by a script, but don't make a test out of it.
MACRO(build_bin_test F) 
	ADD_EXECUTABLE(${F} ${F}.c)
	TARGET_LINK_LIBRARIES(${F} netcdf)
	IF(MSVC)
		SET_TARGET_PROPERTIES(${F} 
			PROPERTIES LINK_FLAGS_DEBUG " /NODEFAULTLIB:MSVCRT"
		)
	ENDIF()
ENDMACRO()

# Binary tests which are used by a script looking for a specific name.
MACRO(add_bin_test_no_prefix F)
	build_bin_test(${F})
	ADD_TEST(${F} ${EXECUTABLE_OUTPUT_PATH}/${F})
	IF(MSVC)
		SET_PROPERTY(TEST ${F} PROPERTY FOLDER "tests/")
	ENDIF()	
ENDMACRO()

# Binary tests which are used by a script looking for a specific name.
MACRO(build_bin_test_no_prefix F)
	build_bin_test(${F})
	ADD_TEST(${F} ${EXECUTABLE_OUTPUT_PATH}/${F})
	IF(MSVC)
		SET_PROPERTY(TEST ${F} PROPERTY FOLDER "tests/")
	ENDIF()	
ENDMACRO()


MACRO(add_bin_test prefix F)
	ADD_EXECUTABLE(${prefix}_${F} ${F}.c)
	TARGET_LINK_LIBRARIES(${prefix}_${F} netcdf)
	IF(MSVC)
		SET_TARGET_PROPERTIES(${prefix}_${F} 
			PROPERTIES LINK_FLAGS_DEBUG " /NODEFAULTLIB:MSVCRT"
			)
	ENDIF()
	ADD_TEST(${prefix}_${F} ${EXECUTABLE_OUTPUT_PATH}/${prefix}_${F})
	IF(MSVC)
		SET_PROPERTY(TEST ${prefix}_${F} PROPERTY FOLDER "tests/")
	ENDIF()	
ENDMACRO()

# A script to print out information at the end of the configuration step.
MACRO(print_conf_summary)
	MESSAGE("")
	MESSAGE("")
	MESSAGE(STATUS "CMake Summary:")
	MESSAGE("")
	MESSAGE(STATUS "\tBuild Type:\t\t\t" ${CMAKE_BUILD_TYPE})
	MESSAGE(STATUS "\tBuilding Shared Libraries:\t" ${BUILD_SHARED_LIBS})
	MESSAGE(STATUS "\tBuilding netCDF-4:\t\t" ${ENABLE_NETCDF_4})
	MESSAGE(STATUS "\tBuilding DAP Support:\t\t" ${ENABLE_DAP})
	MESSAGE(STATUS "\tBuilding Utilities:\t\t" ${BUILD_UTILITIES})
	IF(CMAKE_PREFIX_PATH)
		MESSAGE(STATUS "\tCMake Prefix Path:\t\t" "${CMAKE_PREFIX_PATH}")
	ENDIF()
	MESSAGE(STATUS "\tUsing pnetcdf:\t\t\t" ${STATUS_PNETCDF})
	MESSAGE(STATUS "\tUsing Parallel IO:\t\t" ${STATUS_PARALLEL})
	MESSAGE(STATUS "\tLinking against:\t\t" "${ALL_TLL_LIBS}") 

	MESSAGE("")

ENDMACRO()
# Shell script Macro
MACRO(add_sh_test prefix F)
	IF(NOT MSVC)
		ADD_TEST(${prefix}_${F} bash "-c" "export srcdir=${CMAKE_CURRENT_SOURCE_DIR};export TOPSRCDIR=${CMAKE_SOURCE_DIR};${CMAKE_CURRENT_BINARY_DIR}/${F}.sh")
	ENDIF()

ENDMACRO()


# Macro for replacing '/MD' with '/MT'.
# Used only on Windows, /MD tells VS to use the shared
# CRT libs, MT tells VS to use the static CRT libs.
#
# Taken From:
# 	http://www.cmake.org/Wiki/CMake_FAQ#How_can_I_build_my_MSVC_application_with_a_static_runtime.3F
#
MACRO(specify_static_crt_flag) 

  SET(vars 
    CMAKE_C_FLAGS
    CMAKE_C_FLAGS_DEBUG
    CMAKE_C_FLAGS_MINSIZEREL
    CMAKE_C_FLAGS_RELWITHDEBINFO
    CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG
    CMAKE_CXX_FLAGS_RELEASE CMAKE_CXX_FLAGS_MINSIZEREL
    CMAKE_CXX_FLAGS_RELWITHDEBINFO)
  
  FOREACH(flag_var ${vars})
    IF(${flag_var} MATCHES "/MD")
      STRING(REGEX REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}")
    ENDIF()
  ENDFOREACH()			    
  
  FOREACH(flag_var ${vars})
    MESSAGE(STATUS " '${flag_var}': ${${flag_var}}")
  ENDFOREACH()
  MESSAGE(STATUS "")

ENDMACRO()

################################
# End Macro Definitions
################################


##
# Configuration for post-install RPath
# Adapted from http://www.cmake.org/Wiki/CMake_RPATH_handling
##
IF(NOT MSVC)
	# use, i.e. don't skip the full RPATH for the build tree
	SET(CMAKE_SKIP_BUILD_RPATH  FALSE)

	# when building, don't use the install RPATH already
	# (but later on when installing)
	SET(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE) 

	# Stanza for Apple RPATH.
	# See: http://www.cmake.org/pipermail/cmake/2011-April/043826.html
	# 
	IF(APPLE)
	  SET(CMAKE_INSTALL_NAME_DIR ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR})
	ENDIF(APPLE)

	SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}")

	# add the automatically determined parts of the RPATH
	# which point to directories outside the build tree to the install RPATH
	SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

	# the RPATH to be used when installing, but only if it's not a system directory
	LIST(FIND CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}" isSystemDir)
	IF("${isSystemDir}" STREQUAL "-1")
		SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}")
	ENDIF("${isSystemDir}" STREQUAL "-1")
ENDIF()

##
# End configuration for post-install RPath
##


################################
# Option checks
################################

SET(CMAKE_INCLUDE_CURRENT_DIR ON)

# HDF5 cache variables.
SET (DEFAULT_CHUNK_SIZE 4194304 CACHE STRING "Default Chunk Cache Size.")
SET (DEFAULT_CHUNKS_IN_CACHE 10 CACHE STRING "Default number of chunks in cache.")
SET (CHUNK_CACHE_SIZE 4194304 CACHE STRING "Default Chunk Cache Size.")
SET (CHUNK_CACHE_NELEMS 1009 CACHE STRING "Default maximum number of elements in cache.")
SET (CHUNK_CACHE_PREEMPTION 0.75 CACHE STRING "Default file chunk cache preemption policy for HDf5 files (a number between 0 and 1, inclusive.")
SET (MAX_DEFAULT_CACHE_SIZE 67108864 CACHE STRING "Default maximum cache size.")
SET (NETCDF_LIB_NAME "" CACHE STRING "Default name of the netcdf library.") 
SET (TEMP_LARGE "." CACHE STRING "Where to put large temp files if large file tests are run.")

IF(NOT NETCDF_LIB_NAME STREQUAL "")
	SET(MOD_NETCDF_NAME ON)
ENDIF()

# Option to use a local or remote CDash instance.
# instances Defined in CTestConfig.cmake
OPTION (USE_REMOTE_CDASH "Use CDash instance at CDash.org." OFF)

# Set the appropriate compiler/architecture for universal OSX binaries.
IF(${CMAKE_SYSTEM_NAME} EQUAL "Darwin")
	SET(CMAKE_OSX_ARCHITECTURES i386;x86_64)
ENDIF(${CMAKE_SYSTEM_NAME} EQUAL "Darwin")


# Option to use Static Runtimes in MSVC
IF(MSVC)
  OPTION(NC_USE_STATIC_CRT "Use static CRT Libraries ('\\MT')." OFF)
  IF(NC_USE_STATIC_CRT)
    SET(USE_STATIC_CRT ON)
    specify_static_crt_flag()
  ENDIF()
ENDIF()

# Option to build netCDF Version 2
OPTION (BUILD_V2 "Build netCDF Version 2." ON)

# Option to build utilities
OPTION (BUILD_UTILITIES "Build ncgen, ncgen3, ncdump." ON)
	
# Option to use MMAP
OPTION (ENABLE_MMAP "Use MMAP." OFF)
IF(ENABLE_MMAP)
	IF(NOT HAVE_MREMAP)
		MESSAGE(STATUS "mremap not found: disabling MMAP support.")
		SET(ENABLE_MMAP OFF)
	ELSE()
		SET(HAVE_MMAP ON)
		SET(BUILD_MMAP ON)
		SET(USE_MMAP ON)
	ENDIF()
ENDIF()


# Option to use examples.
OPTION (ENABLE_EXAMPLES "Build Examples" ON)


# Option to use Diskless
OPTION (ENABLE_DISKLESS "Build Diskless." ON)
IF(ENABLE_DISKLESS)
	SET(BUILD_DISKLESS ON)
	SET(USE_DISKLESS ON)
ENDIF()

OPTION(ENABLE_CONVERSION_WARNINGS "Enable warnings for implicit conversion from 64 to 32-bit datatypes." ON)
IF(ENABLE_CONVERSION_WARNINGS)
        IF(NOT MSVC)
		IF(APPLE)
			SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wshorten-64-to-32")
    			SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wshorten-64-to-32")
  		ELSE()
			SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wconversion")
    			SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wconversion")
  		ENDIF(APPLE)
        ENDIF(NOT MSVC)
ENDIF(ENABLE_CONVERSION_WARNINGS)
# Option Logging, only valid for netcdf4. 
OPTION (ENABLE_LOGGING "Enable Logging." OFF)
IF(ENABLE_LOGGING)
	SET(LOGGING ON)
ENDIF()
# Option to use HDF4
OPTION (ENABLE_HDF4 "Build netCDF-4 with HDF5 read capability (HDF4, HDF5 and Zlib required)." OFF)
IF (ENABLE_HDF4) 
   SET(USE_HDF4 ON) 
   # Check for include files, libraries.

   FIND_PATH(MFHDF_H_INCLUDE_DIR mfhdf.h)
   IF(NOT MFHDF_H_INCLUDE_DIR)
	MESSAGE(FATAL_ERROR "HDF4 Support specified, cannot find file mfhdf.h")
   ELSE()
     SET(HAVE_MFHDF_H ON CACHE BOOL "")
     INCLUDE_DIRECTORIES(${MFHDF_H_INCLUDE_DIR})
   ENDIF() 
 
   FIND_LIBRARY(HDF4_DF_LIB NAMES df libdf)
   IF(NOT HDF4_DF_LIB)
	MESSAGE(FATAL_ERROR "Can't find or link to the hdf4 df library.")
   ENDIF()

   FIND_LIBRARY(HDF4_MFHDF_LIB NAMES mfhdf libmfhdf)
   IF(NOT HDF4_MFHDF_LIB)
	MESSAGE(FATAL_ERROR "Can't find or link to the hdf4 mfhdf library.")
   ENDIF()

   SET(HDF4_LIBRARIES ${HDF4_MFHDF_LIB} ${HDF4_DF_LIB})
   # End include files, libraries.
   MESSAGE(STATUS "HDF4 libraries: ${HDF4_DF_LIB}, ${HDF4_MFHDF_LIB}")
   
   MESSAGE(STATUS "Seeking HDF4 jpeg dependency.")

   # Look for the jpeglib.h header file.
   FIND_PATH(JPEGLIB_H_INCLUDE_DIR jpeglib.h)
   IF(NOT JPEGLIB_H_INCLUDE_DIR)
     MESSAGE(FATAL_ERROR "HDF4 Support enabled but cannot find jpeglib.h")
   ELSE()
     SET(HAVE_JPEGLIB_H ON CACHE BOOL "")
     INCLUDE_DIRECTORIES(${JPEGLIB_H_INCLUDE_DIR})
   ENDIF()

   FIND_LIBRARY(JPEG_LIB NAMES jpeg libjpeg)
   IF(NOT JPEG_LIB)
     MESSAGE(FATAL_ERROR "HDF4 Support enabled but cannot find libjpeg")
   ENDIF()
   SET(HDF4_LIBRARIES ${JPEG_LIB} ${HDF4_LIBRARIES})
   MESSAGE(STATUS "Found JPEG libraries: ${JPEG_LIB}")
   
   # Option to enable HDF4 file tests.
   OPTION(ENABLE_HDF4_FILE_TESTS "Run HDF4 file tests.  This fetches sample HDF4 files from the Unidata ftp site to test with (requires wget)." ON)
   IF(ENABLE_HDF4_FILE_TESTS)
     FIND_PROGRAM(PROG_WGET NAMES wget)
     IF(PROG_WGET)
       SET(USE_HDF4_FILE_TESTS ON)
     ELSE()
       MESSAGE(STATUS "Unable to locate 'wget'.  Disabling hdf4 file tests.")
       SET(USE_HDF4_FILE_TESTS OFF)
     ENDIF()
     
   ENDIF()
   
 ENDIF ()

# Option to Build DLL
IF(WIN32)
	OPTION (ENABLE_DLL "Build a Windows DLL." ${BUILD_SHARED_LIBS})
	IF (ENABLE_DLL)
  		SET(BUILD_DLL ON CACHE BOOL "")
		ADD_DEFINITIONS(-DDLL_NETCDF)
  		ADD_DEFINITIONS(-DDLL_EXPORT)
	ENDIF ()
ENDIF()
# Did the user specify a default minimum blocksize for posixio?
SET (NCIO_MINBLOCKSIZE 256 CACHE STRING "Minimum I/O Blocksize for netCDF classic and 64-bit offset format files.")

# Build netCDF4
OPTION (ENABLE_NETCDF_4 "Enable netCDF-4" ON)
IF(ENABLE_NETCDF_4)
	SET(USE_NETCDF4 ON CACHE BOOL "")
	SET(ENABLE_NETCDF_4 ON CACHE BOOL "")
	SET(ENABLE_NETCDF4 ON CACHE BOOL "")
ENDIF()

# Option for building RPC
OPTION (ENABLE_RPC OFF "Enable RPC Client and Server.")
IF(ENABLE_RPC)
	SET(BUILD_RPC ON CACHE BOOL "")
ENDIF()

# Option to Enable HDF5
OPTION (USE_SZIP "Use SZip" OFF)
OPTION (USE_HDF5 "Use HDF5." ${ENABLE_NETCDF_4})
OPTION (ENABLE_DYNAMIC_LOADING "Enable Dynamic Loading" OFF)
IF(ENABLE_DYNAMIC_LOADING)
  SET(USE_LIBDL ON CACHE BOOL "")
ENDIF()

IF (USE_HDF5 OR ENABLE_NETCDF_4)
  SET(USE_HDF5 ON)
  SET(USE_NETCDF4 ON)

  # Accomodate developers who have hdf5 libraries and
  # headers on their system, but do not have a the hdf
  # .cmake files.  If this is the case, they should
  # specify HDF5_HL_LIB, HDF5_LIB, HDF5_INCLUDE_DIR manually.
  IF(HDF5_LIB AND HDF5_HL_LIB AND HDF5_INCLUDE_DIR)
	SET(HDF5_LIBRARIES ${HDF5_LIB} ${HDF5_HL_LIB})
  	SET(HDF5_INCLUDE_DIRS ${HDF5_INCLUDE_DIR})
  ELSE()
    IF(MSVC)
      FIND_PACKAGE(HDF5 COMPONENTS C HL NO_MODULE REQUIRED)
      MESSAGE(STATUS "Found HDF5: ${HDF5_LIBRARIES}")
    ELSE()
      FIND_PACKAGE(HDF5 COMPONENTS C HL REQUIRED)
    ENDIF()

  ENDIF()
  
  INCLUDE_DIRECTORIES(${HDF5_INCLUDE_DIRS})
  
  # Starting with hdf5 1.8.11, dynamic loading is an option.
  # In case hdf5 has a dependency on libdl, the user must specify
  # -DENABLE_DYNAMIC_LOADING=ON when configuring netcdf.
  IF(USE_LIBDL)
    FIND_LIBRARY(LIBDL NAMES dl dld)
    FIND_PATH(LIBDL_INCLUDE_DIR dlfcn.h)
    IF(NOT LIBDL)
      MESSAGE(ERROR "Cannot find libdl, but dynamic loading was specified.")
    ENDIF()
    IF(NOT LIBDL_INCLUDE_DIR)
      MESSAGE(ERROR "Cannot find dlfcn.h, but dynamic loading was specified.")
    ENDIF()
    MESSAGE(STATUS "Found libdl: ${LIBDL}")
    SET(HAVE_LIBDL ON)
    INCLUDE_DIRECTORIES(${LIBDL_INCLUDE_DIR})

  ENDIF()

  #Check to see if H5Z_SZIP exists in HDF5_Libraries. If so, we must use szip.
  IF(USE_SZIP)
	FIND_PACKAGE(SZIP NO_MODULE)
	INCLUDE_DIRECTORIES(${SZIP_INCLUDE_DIR})
  ENDIF()
	
  SET(H5_USE_16_API 1)
  OPTION(NC_ENABLE_HDF_16_API "Enable HDF5 1.6.x Compatibility (Required)" ON)  
  IF(NOT NC_ENABLE_HDF_16_API)
    SET(H5_USE_16_API 0)
  ENDIF()
  
  # Check for ZLib, but only if using HDF5.
  FIND_PACKAGE(ZLIB)
  IF(NOT ZLIB_LIBRARY)
	MESSAGE(FATAL_ERROR "HDF5 Support specified, cannot find ZLib.")
  ENDIF()
  SET(USE_ZLIB ON)
  INCLUDE_DIRECTORIES(${ZLIB_INCLUDE_DIRS})
ENDIF ()

# Option to Build DAP Client
OPTION (ENABLE_DAP "Enable DAP Client." ON)
IF (ENABLE_DAP)
	SET(USE_DAP ON)
	FIND_PACKAGE(CURL)
	IF(NOT CURL_LIBRARY)
		MESSAGE(FATAL_ERROR "DAP Support specified, CURL libraries are not found.")
	ENDIF()
	ADD_DEFINITIONS(-DCURL_STATICLIB=1)
	INCLUDE_DIRECTORIES(${CURL_INCLUDE_DIRS})
ENDIF()

# Option to Enable DAP long tests, remote tests.
OPTION(ENABLE_DAP_LONG_TESTS "Enable DAP long tests." OFF)
OPTION(ENABLE_DAP_REMOTE_TESTS "Enable DAP remote tests." ON)

# If netCDF4 and DAP, Option for DAP groups.
IF(ENABLE_NETCDF_4 AND USE_DAP)		
		  OPTION(ENABLE_DAP_GROUPS "Whether netcdf4 group names should be enabled." ON)
ELSE()
	SET(ENABLE_DAP_GROUPS OFF CACHE BOOL "Whether netcdf4 group names should be enabled.")
ENDIF()


# Enable some developer-only tests
OPTION(ENABLE_EXTRA_TESTS "Enable Extra tests. Some may not work because of known issues. Developers only." OFF)
IF(ENABLE_EXTRA_TESTS)
	SET(EXTRA_TESTS ON)
ENDIF()

# Option to use bundled XGetopt in place of getopt(). This is mostly useful
# for MSVC builds. If not building utilities, getopt() isn't required at all.
IF(MSVC)
	OPTION(ENABLE_XGETOPT "Enable bundled XGetOpt instead of external getopt()." ON)
	IF(ENABLE_XGETOPT)
		SET(USE_X_GETOPT ON CACHE BOOL "")
	ENDIF()
ENDIF()

SET(MATH "")
IF(NOT WIN32)
	# FFIO insteaad of PosixIO
	OPTION(ENABLE_FFIO "If true, use ffio instead of posixio" OFF)
	IF(ENABLE_FFIO)
		SET(USE_FFIO ON CACHE BOOL "")
	ENDIF()
ENDIF()

# Enable Tests
#IF(NOT MSVC)
OPTION (ENABLE_TESTS "Enable basic tests, run with 'make test'." ON)
IF(ENABLE_TESTS) 
 SET(BUILD_TESTSETS ON CACHE BOOL "")
ENDIF()

# Enable Large file tests
OPTION (ENABLE_LARGE_FILE_TESTS "Enable large file tests." OFF)
IF(ENABLE_LARGE_FILE_TESTS)
  SET(LARGE_FILE_TESTS ON)
ENDIF()

# Location for large file tests.
SET(TEMP_LARGE "." CACHE STRING "Location to store large file tests.")

OPTION (ENABLE_FSYNC "Enable experimental fsync code." OFF)
IF(ENABLE_FSYNC)
	SET(USE_FSYNC ON)
ENDIF()

# Linux specific large file support flags.
# Modelled after check in CMakeLists.txt for hdf5.
OPTION(ENABLE_LARGE_FILE_SUPPORT "Enable large file support." ON)
IF(ENABLE_LARGE_FILE_SUPPORT)
  IF(MSVC)
    SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /LARGEADDRESSAWARE")
    SET(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /LARGEADDRESSAWARE")
    SET(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} /LARGEADDRESSAWARE")
  ELSE()
    SET(_FILE_OFFSET_BITS 64)
    SET(_LARGEFILE64_SOURCE TRUE)
    SET(_LARGEFILE_SOURCE TRUE)
  ENDIF()
ENDIF()


# Provide the option to perform coverage tests.
IF(CMAKE_COMPILER_IS_GNUCC OR APPLE)
  OPTION (ENABLE_COVERAGE_TESTS "Enable compiler flags needed to perform coverage tests." OFF)
  IF(ENABLE_COVERAGE_TESTS)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0 -profile-arcs -ftest-coverage")
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -O0 -fprofile-arcs -ftest-coverage")
    SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fprofile-arcs -ftest-coverage")
    MESSAGE(STATUS "Coverage Tests: On.")
  ENDIF()
ENDIF()


OPTION (ENABLE_EXAMPLE_TESTS "Run extra example tests.  Requires GNU Sed. Ignored if netCDF-4 is not Enabled" OFF)
IF(NOT ENABLE_NETCDF_4 AND ENABLE_EXAMPLE_TESTS)
	SET (ENABLE_EXAMPLE_TESTS OFF)
ENDIF()

# Enable Parallel (different than pnetcdf).
SET(STATUS_PARALLEL "OFF")
OPTION(ENABLE_PARALLEL "Build netCDF-4 with parallel IO" OFF)
IF(ENABLE_PARALLEL AND ENABLE_NETCDF_4)
	SET(USE_PARALLEL ON CACHE BOOL "")
	# Check for H5Pget_fapl_mpiposx, define USE_PARALLEL_POSIX
	CHECK_LIBRARY_EXISTS(${HDF5_LIB} H5Pget_fapl_mpioposix "" USE_PARALLEL_POSIX)	
		
	# Else Check for H5Pget_fapl_mpio, define USE_PARALLEL_MPIO
	IF(NOT USE_PARALLEL_POSIX)
	       CHECK_LIBRARY_EXISTS(${HDF5_LIB} H5Pget_fapl_mpio "" USE_PARALLEL_MPIO)
	ENDIF()
	
	IF(NOT USE_PARALLEL_POSIX AND NOT USE_PARALLEL_MPIO)
		SET(USE_PARALLEL OFF CACHE BOOL "")
		MESSAGE(STATUS "Cannot find HDF5 library built with parallel support. Disabling parallel build.")
	ENDIF()
	SET(STATUS_PARALLEL "ON")
ENDIF()

# Options to enable parallel IO, tests.
SET(STATUS_PNETCDF "OFF")
OPTION(ENABLE_PNETCDF "Build with parallel I/O for classic and 64-bit offset files using parallel-netcdf." OFF)
IF(ENABLE_PNETCDF)
	# Check for ncmpi_create in libpnetcdf, define USE_PNETCDF
	# Does the user want to turn on PNETCDF read ability?
	SET(USE_PNETCDF ON CACHE BOOL "")	
	FIND_LIBRARY(PNETCDF NAMES pnetcdf)
	FIND_PATH(PNETCDF_INCLUDE_DIR pnetcdf.h)
	IF(NOT PNETCDF)
		MESSAGE(STATUS "Cannot find pnetcdf library. Disabling pnetcdf support.")
		SET(USE_PNETCDF OFF CACHE BOOL "")
	ENDIF()
	SET(STATUS_PNETCDF "ON")	
	INCLUDE_DIRECTORIES(${PNETCDF_INCLUDE_DIR})
	SET(HAVE_LIBPNETCDF ON)
        # pnetcdf => parallel
        SET(STATUS_PARALLEL "ON")
ENDIF()

# Enable Parallel Tests.
OPTION (ENABLE_PARALLEL_TESTS "Enable Parallel IO Tests. Ignored if netCDF4 is not enabled, or if there is no parallel I/O Support." ${USE_PARALLEL})
IF(ENABLE_PARALLEL_TESTS AND USE_PARALLEL)
	SET(TEST_PARALLEL ON CACHE BOOL "")
ENDIF()

# Determine whether or not to generate documentation.
OPTION (ENABLE_DOXYGEN "Enable generation of doxygen." OFF)
IF(ENABLE_DOXYGEN)
	SET(BUILD_DOCS ON CACHE BOOL "")
	OPTION(ENABLE_INTERNAL_DOCS "Build internal documentation. This is of interest to developers only." OFF)
	IF(ENABLE_INTERNAL_DOCS)
		SET(BUILD_INTERNAL_DOCS YES CACHE STRING "")
	ELSE()
		SET(BUILD_INTERNAL_DOCS NO CACHE STRING "")
	ENDIF()
ENDIF()

# By default, MSVC has a stack size of 1000000.
# Allow a user to override this.
IF(MSVC)
  SET(NC_MSVC_STACK_SIZE 40000000 CACHE STRING "Default stack size for MSVC-based projects.")
  
  # By default, CMake sets the stack to 1000000.
  # Remove this limitation.
  # See here for more details:
  # http://www.cmake.org/pipermail/cmake/2009-April/028710.html
  SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /STACK:${NC_MSVC_STACK_SIZE}")
  SET(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /STACK:${NC_MSVC_STACK_SIZE}")
  SET(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} /STACK:${NC_MSVC_STACK_SIZE}")
  
ENDIF()


# Internal Documentation
OPTION (ENABLE_INTERNAL_DOCS "Enable documentation of library internals. This is of interest only to those developing the netCDF library." OFF)
IF(ENABLE_INTERNAL_DOCS)
	SET(BUILD_INTERNAL_DOCS ON)
ENDIF()


# Set some of the options as advanced.
MARK_AS_ADVANCED(ENABLE_INTERNAL_DOCS VALGRIND_TESTS ENABLE_PNETCDF ENABLE_COVERAGE_TESTS)
MARK_AS_ADVANCED(ENABLE_DAP_REMOTE_TESTS ENABLE_DAP_LONG_TESTS USE_REMOTE_CDASH)

################################
# Option checks
################################


#####
# System inspection checks
#####
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/include)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/oc2)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/libsrc)
SET(CMAKE_REQUIRED_INCLUDES ${CMAKE_SOURCE_DIR}/libsrc)

#
# Library include checks
CHECK_INCLUDE_FILE("math.h"      HAVE_MATH_H)
CHECK_INCLUDE_FILE("unistd.h"	 HAVE_UNISTD_H)
# Solve a compatibility issue in ncgen/, which checks
# for NO_UNISTD_H
IF(NOT HAVE_UNISTD_H)
	SET(YY_NO_UNISTD_H TRUE)
ENDIF()

CHECK_INCLUDE_FILE("alloca.h"	 HAVE_ALLOCA_H)
CHECK_INCLUDE_FILE("malloc.h"    HAVE_MALLOC_H)
CHECK_INCLUDE_FILE("ctype.h"	 HAVE_CTYPE_H)
CHECK_INCLUDE_FILE("dirent.h"	 HAVE_DIRENT_H)
CHECK_INCLUDE_FILE("dlfcn.h"	 HAVE_DLFCN_H)
CHECK_INCLUDE_FILE("errno.h"	 HAVE_ERRNO_H)
CHECK_INCLUDE_FILE("fcntl.h"	 HAVE_FCNTL_H)
CHECK_INCLUDE_FILE("getopt.h"	 HAVE_GETOPT_H)
CHECK_INCLUDE_FILE("stdbool.h"	 HAVE_STDBOOL_H)
CHECK_INCLUDE_FILE("locale.h"	 HAVE_LOCAL_H)
CHECK_INCLUDE_FILE("stdint.h"	 HAVE_STDINT_H)
CHECK_INCLUDE_FILE("stdio.h"	 HAVE_STDIO_H)
CHECK_INCLUDE_FILE("stdlib.h"	 HAVE_STDLIB_H)
CHECK_INCLUDE_FILE("stdarg.h"    HAVE_STDARG_H)
CHECK_INCLUDE_FILE("strings.h"	 HAVE_STRINGS_H)
CHECK_INCLUDE_FILE("signal.h" 	 HAVE_SIGNAL_H)
CHECK_INCLUDE_FILE("sys/dir.h"	 HAVE_SYS_DIR_H)
CHECK_INCLUDE_FILE("sys/ndir.h"	 HAVE_SYS_NDIR_H)
CHECK_INCLUDE_FILE("sys/param.h" HAVE_SYS_PARAM_H)
CHECK_INCLUDE_FILE("sys/stat.h"	 HAVE_SYS_STAT_H)
CHECK_INCLUDE_FILE("sys/time.h"	 HAVE_SYS_TIME_H)
CHECK_INCLUDE_FILE("sys/types.h" HAVE_SYS_TYPES_H)
CHECK_INCLUDE_FILE("sys/wait.h"	 HAVE_SYS_WAIT_H)
CHECK_INCLUDE_FILE("sys/resource.h" HAVE_SYS_RESOURCE_H)
CHECK_INCLUDE_FILE("fcntl.h"	HAVE_FCNTL_H)
CHECK_INCLUDE_FILE("inttypes.h"  HAVE_INTTYPES_H)
CHECK_INCLUDE_FILE("pstdint.h"	HAVE_PSTDINT_H)
CHECK_INCLUDE_FILE("endian.h"	HAVE_ENDIAN_H)
CHECK_INCLUDE_FILE("BaseTsd.h"  HAVE_BASETSD_H)

# Type checks
CHECK_TYPE_SIZE("double"	 SIZEOF_DOUBLE)
CHECK_TYPE_SIZE("float"		 SIZEOF_FLOAT)
CHECK_TYPE_SIZE("int"		 SIZEOF_INT)
CHECK_TYPE_SIZE("long"		 SIZEOF_LONG)
CHECK_TYPE_SIZE("long long"	 SIZEOF_LONG_LONG)
CHECK_TYPE_SIZE("off_t"		 SIZEOF_OFF_T)
CHECK_TYPE_SIZE("off64_t"        SIZEOF_OFF64_T)
CHECK_TYPE_SIZE("short"		 SIZEOF_SHORT)
CHECK_TYPE_SIZE("size_t"	 SIZEOF_SIZE_T)
CHECK_TYPE_SIZE("ssize_t"	 SIZEOF_SSIZE_T)
# __int64 is used on Windows for large file support.
CHECK_TYPE_SIZE("__int64"        SIZEOF___INT_64)
CHECK_TYPE_SIZE("uchar"		 SIZEOF_UCHAR)
CHECK_TYPE_SIZE("int64_t"	 SIZEOF_INT64_T)
CHECK_TYPE_SIZE("uint64_t"	 SIZEOF_UINT64_T)

# On windows systems, we redefine off_t as __int64
# to enable LFS. This is true on 32 and 64 bit system.s
# We must redefine SIZEOF_OFF_T to match.
IF(MSVC AND SIZEOF___INT_64)
  SET(SIZEOF_OFF_T  ${SIZEOF___INT_64})
ENDIF()

IF(SIZEOF_SSIZE_T)
	SET(HAVE_SSIZE_T 1)
ELSE()
	CHECK_TYPE_SIZE("SSIZE_T"	SIZEOF_SSIZE_T)
	IF(SIZEOF_SSIZE_T)
		SET(HAVE_SSIZE_T 1)
	ENDIF()
ENDIF()

# Check for various functions. 
CHECK_FUNCTION_EXISTS(fsync	HAVE_FSYNC)
CHECK_FUNCTION_EXISTS(strlcat 	HAVE_STRLCAT)
CHECK_FUNCTION_EXISTS(strerror	HAVE_STRERROR)
CHECK_FUNCTION_EXISTS(snprintf	HAVE_SNPRINTF)
CHECK_FUNCTION_EXISTS(strchr	HAVE_STRCHR)
CHECK_FUNCTION_EXISTS(strrchr	HAVE_STRRCHR)
CHECK_FUNCTION_EXISTS(strcat	HAVE_STRCAT)
CHECK_FUNCTION_EXISTS(strcpy	HAVE_STRCPY)
CHECK_FUNCTION_EXISTS(strdup	HAVE_STRDUP)
CHECK_FUNCTION_EXISTS(strcasecmp	HAVE_STRCASECMP)
CHECK_FUNCTION_EXISTS(strtod	HAVE_STRTOD)
CHECK_FUNCTION_EXISTS(strtoll	HAVE_STRTOLL)
CHECK_FUNCTION_EXISTS(strtoull	HAVE_STROULL)
CHECK_FUNCTION_EXISTS(strstr	HAVE_STRSTR)
CHECK_FUNCTION_EXISTS(mkstemp	HAVE_MKSTEMP)
CHECK_FUNCTION_EXISTS(rand	HAVE_RAND)
CHECK_FUNCTION_EXISTS(gettimeofday	HAVE_GETTIMEOFDAY)
CHECK_FUNCTION_EXISTS(fsync	HAVE_FSYNC)
CHECK_FUNCTION_EXISTS(MPI_Comm_f2C	HAVE_MPI_COMM_F2C)
CHECK_FUNCTION_EXISTS(memmove	HAVE_MEMMOVE)
CHECK_FUNCTION_EXISTS(getpagesize	HAVE_GETPAGESIZE)
CHECK_FUNCTION_EXISTS(sysconf	HAVE_SYSCONF)
CHECK_FUNCTION_EXISTS(mremap	HAVE_MREMAP)
CHECK_FUNCTION_EXISTS(getrlimit	HAVE_GETRLIMIT)
CHECK_FUNCTION_EXISTS(_filelengthi64 HAVE_FILE_LENGTH_I64)

#####
# End system inspection checks.
#####


# Check for the math library so it can be explicitely linked.
IF(NOT WIN32)
       FIND_LIBRARY(HAVE_LIBM NAMES math m libm)
       MESSAGE(STATUS "Found Math library: ${HAVE_LIBM}")
       IF(NOT HAVE_LIBM)
       	      MESSAGE(FATAL_ERROR "Unable to find the math library.")
       ENDIF()
ENDIF()

# Create config.h file, nc-config
configure_file("${netCDF_SOURCE_DIR}/config.h.in.cmake"
	"${netCDF_BINARY_DIR}/config.h")
ADD_DEFINITIONS(-DHAVE_CONFIG_H)
INSTALL(PROGRAMS ${netCDF_BINARY_DIR}/nc-config DESTINATION bin COMPONENT utilities)
INCLUDE_DIRECTORIES(${netCDF_BINARY_DIR})
# End autotools-style checs for config.h



#####
# Set core names of the libraries.
#####
SET (netCDF_LIB_CORENAME	"netcdf")

#####
# Set the true names of all the libraries, if customized by external project
#####
# Recurse into other subdirectories.
add_subdirectory("include")
add_subdirectory(libdispatch)
add_subdirectory(libsrc)

IF (USE_PNETCDF)
add_subdirectory(libsrc5)
ENDIF (USE_PNETCDF)

IF (USE_HDF5)
add_subdirectory(libsrc4)
ENDIF (USE_HDF5)

IF (USE_DAP)
	ADD_SUBDIRECTORY(oc2)
	ADD_SUBDIRECTORY(libdap2)
ENDIF()

add_subdirectory(liblib)

# For tests and utilities, we are no longer
# exporting symbols but rather importing them.
IF(BUILD_DLL)
	REMOVE_DEFINITIONS(-DDLL_EXPORT)
ENDIF()

# Enable Utilities.
IF (BUILD_UTILITIES)
	INCLUDE_DIRECTORIES(ncdump)
	ADD_SUBDIRECTORY(ncgen)
	ADD_SUBDIRECTORY(ncgen3)
	ADD_SUBDIRECTORY(ncdump)
ENDIF()

# Enable tests
IF(ENABLE_TESTS)
	ADD_SUBDIRECTORY(nctest)
	ADD_SUBDIRECTORY(nc_test)
	IF(USE_NETCDF4)
  		ADD_SUBDIRECTORY(nc_test4)
		ADD_SUBDIRECTORY(h5_test)
	ENDIF()
	IF(USE_DAP AND ENABLE_DAP_REMOTE_TESTS)
		ADD_SUBDIRECTORY(ncdap_test)
	ENDIF()
	
	IF(ENABLE_EXAMPLES)
		ADD_SUBDIRECTORY(examples)
	ENDIF()

ENDIF()

# Code to generate an export header
#GENERATE_EXPORT_HEADER(netcdf
#	BASE_NAME netcdf
#	EXPORT_MACRO_NAME netcdf_EXPORT
#	EXPORT_FILE_NAME netcdf_Export.h
#	STATIC_DEFINE netcdf_BUILT_AS_STATIC
#)

#####
# Build doxygen documentation, if need be.
#####
ADD_SUBDIRECTORY(man4)
#####
# Moving on to CPack, install packages.
#####
INSTALL(FILES ${netCDF_SOURCE_DIR}/include/netcdf.h
  	DESTINATION include COMPONENT headers)
IF(ENABLE_PNETCDF OR ENABLE_PARALLEL)
	INSTALL(FILES ${netCDF_SOURCE_DIR}/include/netcdf_par.h
	DESTINATION include COMPONENT headers)
ENDIF()
# Install the dependencies.
IF(MSVC)
	INSTALL(DIRECTORY ${CMAKE_PREFIX_PATH} DESTINATION "deps" COMPONENT dependencies)
ENDIF()


IF(ENABLE_DOXYGEN)	
	INSTALL(DIRECTORY man4 DESTINATION "." COMPONENT documentation)
ENDIF()

# Subdirectory CMakeLists.txt files should specify their own
# 'install' files.
# Including 'CPack' kicks everything off.
INCLUDE(InstallRequiredSystemLibraries)
CONFIGURE_FILE(
  ${CMAKE_CURRENT_SOURCE_DIR}/FixBundle.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/FixBundle.cmake
  @ONLY
)
#INSTALL(SCRIPT ${CMAKE_CURRENT_BINARY_DIR}/FixBundle.cmake)

# Create CMake package configuration files. With these, other packages using
# cmake should be able to find netcdf using find_package and find_library.
# This call is paired with one in liblib.
install (EXPORT netcdf-targets
	DESTINATION share/cmake
	COMPONENT documentation)

configure_file (
	${netCDF_SOURCE_DIR}/netcdf-config.cmake.in
	${netCDF_BINARY_DIR}/netcdf-config.cmake @ONLY)

configure_file (
	${netCDF_SOURCE_DIR}/netcdf-config-version.cmake.in
	${netCDF_BINARY_DIR}/netcdf-config-version.cmake @ONLY)

install (FILES ${netCDF_BINARY_DIR}/netcdf-config.cmake
		${netCDF_BINARY_DIR}/netcdf-config-version.cmake
		DESTINATION share/cmake)

# End CMake package configuration files.
#####
# Various options for CPACK
#####

##
# Declare exclusions list used when building a source file.
# NOTE!! This list uses regular expressions, NOT wildcards!! 
## 
SET (CPACK_SOURCE_IGNORE_FILES "${CPACK_SOURCE_IGNORE_FILES}"
	"/expecttds3/"
	"/nocacheremote3/"
	"/nocacheremote4/"
	"/special3/"
	"${CMAKE_BINARY_DIR}/*"
	"/myhtml/*"
	"/.svn/"
	"my.*\\\\.sh"
	"/.deps/"
	"/.libs"
	"/html/"
	".*\\\\.jar"
	".*\\\\.jdl"
	".*\\\\.sed"
	".*\\\\.proto"
	".*\\\\.texi"
	".*\\\\.example"
	"Make0"
	"/obsolete/"
	"/unknown/"
	".*~" 
)
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/COPYRIGHT
	${CMAKE_CURRENT_BINARY_DIR}/COPYRIGHT.txt
	@ONLY
)

SET(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_BINARY_DIR}/COPYRIGHT.txt")
IF(NOT CPACK_PACK_VERSION)
	SET(CPACK_PACKAGE_VERSION ${VERSION})
ENDIF()


IF(APPLE)
  SET(CPACK_SOURCE_GENERATOR "TGZ")
  SET(CPACK_GENERATOR "PackageMaker" "STGZ" "TBZ2" "TGZ" "ZIP")
ENDIF()

# Create an 'uninstall' target.
CONFIGURE_FILE(
	"${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
	"${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
	IMMEDIATE @ONLY)


ADD_CUSTOM_TARGET(uninstall
	COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)


## Customize some of the package component descriptions
set (CPACK_COMPONENT_UTILITIES_DISPLAY_NAME "netCDF Utilities")
set (CPACK_COMPONENT_LIBRARIES_DISPLAY_NAME "netCDF Libraries")
set (CPACK_COMPONENT_HEADERS_DISPLAY_NAME "netCDF Headers")
set (CPACK_COMPONENT_DEPENDENCIES_DISPLAY_NAME "netCDF Dependencies")
set (CPACK_COMPONENT_DOCUMENTATION_DISPLAY_NAME "netCDF Documentation")


set (CPACK_COMPONENT_UTILITIES_DESCRIPTION
	"The netCDF Utilities")
set (CPACK_COMPONENT_LIBRARIES_DESCRIPTION
	"The netCDF Libraries")
set (CPACK_COMPONENT_HEADERS_DESCRIPTION
	"C header files for use with netCDF")
set (CPACK_COMPONENT_DEPENDENCIES_DESCRIPTION
	"Dependencies for this build of netCDF")
set (CPACK_COMPONENT_DOCUMENTATION_DESCRIPTION
	"The netCDF user documentation.")
print_conf_summary()


# Create nc-config
FILE(MAKE_DIRECTORY ${netCDF_BINARY_DIR}/tmp)
configure_file("${netCDF_SOURCE_DIR}/nc-config.in.cmake"
  "${netCDF_BINARY_DIR}/tmp/nc-config" @ONLY)
FILE(COPY "${netCDF_BINARY_DIR}/tmp/nc-config"
  DESTINATION ${netCDF_BINARY_DIR}/
  FILE_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
#EXECUTE_PROCESS(COMMAND "chmod 755 ${netCDF_BINARY_DIR}/nc-config")




# Enable Makedist files.
ADD_MAKEDIST()
ENABLE_MAKEDIST(README COPYRIGHT RELEASE_NOTES INSTALL INSTALL.cmake test_prog.c lib_flags.am cmake CMakeLists.txt COMPILE.cmake.txt config.h.in.cmake cmake_uninstall.cmake.in netcdf-config-version.cmake.in netcdf-config.cmake.in FixBundle.cmake.in nc-config.in.cmake RELEASE_NOTES.md configure configure.ac install-sh config.h.in config.sub)

# CPack inclusion must come last.
INCLUDE(CPack)




