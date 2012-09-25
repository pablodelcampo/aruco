# ----------------------------------------------------------------------------
#   Basic Configuration
# ----------------------------------------------------------------------------
CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

PROJECT(aruco)
SET(PROJECT_VERSION "1.2.4")
STRING(REGEX MATCHALL "[0-9]" PROJECT_VERSION_PARTS "${PROJECT_VERSION}")
LIST(GET PROJECT_VERSION_PARTS 0 PROJECT_VERSION_MAJOR)
LIST(GET PROJECT_VERSION_PARTS 1 PROJECT_VERSION_MINOR)
LIST(GET PROJECT_VERSION_PARTS 2 PROJECT_VERSION_PATCH)
SET(PROJECT_SOVERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}")


#------------------------------------------------------
# Build type
#------------------------------------------------------

IF(NOT CMAKE_BUILD_TYPE )
   SET( CMAKE_BUILD_TYPE "Debug" )
ENDIF()

#------------------------------------------------------
# Lib Names and Dirs
#------------------------------------------------------

IF(WIN32)
    # Postfix of DLLs:
    SET(PROJECT_DLLVERSION "${PROJECT_VERSION_MAJOR}${PROJECT_VERSION_MINOR}${PROJECT_VERSION_PATCH}")
    SET(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}")
    SET(RUNTIME_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin CACHE PATH "Directory for dlls and binaries")
    SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin CACHE PATH "Directory for binaries")
    SET(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin CACHE PATH "Directory for dlls")
ELSE()
    # Postfix of so's:
    SET(PROJECT_DLLVERSION)
    SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_INSTALL_PREFIX}/lib/cmake/ /usr/lib/cmake)
ENDIF()


OPTION(BUILD_SHARED_LIBS 	"Set to OFF to build static libraries" ON)

# ----------------------------------------------------------------------------
#   Find Dependencies
# ----------------------------------------------------------------------------

FIND_PACKAGE(OpenCV 	REQUIRED )
SET (REQUIRED_LIBRARIES ${OpenCV_LIBS})


IF(EXISTS ${GLUT_PATH})
    INCLUDE_DIRECTORIES(${GLUT_PATH}/include)
    SET(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} ${GLUT_PATH}/lib)
ENDIF()

##LOOK FOR OPENGL AND GLUT
#FIND OPENGL LIBRARY. In Android, there is no need since it is included
IF(NOT ANDROID_CREATION)
  FIND_PACKAGE(OpenGL)
  FIND_PACKAGE(GLUT)#standard package

  MESSAGE(STATUS "GLUT_FOUND=${GLUT_FOUND} OPENGL_gl_LIBRARY=${OPENGL_gl_LIBRARY} GLUT_HEADER=${GLUT_HEADER}")
  IF(NOT GLUT_FOUND) #else, freeglut
    FIND_LIBRARY(GLUT_glut_LIBRARY     NAMES freeglut)
    MESSAGE(STATUS "GLUT_glut_LIBRARY=${GLUT_glut_LIBRARY}")
  ENDIF()
  IF ( (NOT GLUT_glut_LIBRARY AND NOT GLUT_FOUND) OR NOT OPENGL_gl_LIBRARY)
    SET(GL_FOUND "NO")
  ELSE()
    SET(GL_FOUND "YES")
    SET (OPENGL_LIBS  ${OPENGL_gl_LIBRARY} ${OPENGL_glu_LIBRARY} ${GLUT_glut_LIBRARY})
  ENDIF()
ENDIF()

# ----------------------------------------------------------------------------
#   PROJECT CONFIGURATION
#   force some variables that could be defined in the command line to be written to cache
# ----------------------------------------------------------------------------
OPTION(WARNINGS_ARE_ERRORS        "Treat warnings as errors" 									                  ON)
OPTION(WARNINGS_ANSI_ISO          "Issue all the mandatory diagnostics Listed in C standard" 	  ON)
OPTION(ENABLE_PROFILING           "Enable profiling in Valgrind (Add flags: -g -fno_inline)"    OFF)
OPTION(BUILD_SHARED_LIBS 		      "Build shared libraries"                                      ON)

OPTION(INSTALL_DOC                "Install documentation in system"                             OFF)
OPTION(USE_MATHJAX  			        "Generate doc-formulas via mathjax instead of latex"          ON)
OPTION(USE_DOT 				            "Diagram generation with graphviz"                            ON)
OPTION(USE_LATEX			            "Build latex documentation"                                   OFF)
OPTION(USE_CHM						        "Build CHM Windows documentation"                             OFF)


# ----------------------------------------------------------------------------
#   Compiler options
# ----------------------------------------------------------------------------
INCLUDE(cmake/compileroptions.cmake REQUIRED)
IF(MSVC)
  INCLUDE(cmake/crtlinkage.cmake    REQUIRED)
ENDIF(MSVC)

# ----------------------------------------------------------------------------
#   Documentation
# ----------------------------------------------------------------------------
INCLUDE("${PROJECT_SOURCE_DIR}/cmake/generateDoc.cmake")
GENERATE_DOCUMENTATION(${PROJECT_SOURCE_DIR}/cmake/dox.in)

# ----------------------------------------------------------------------------
#   Uninstall target, for "make uninstall"
# ----------------------------------------------------------------------------
CONFIGURE_FILE( "${CMAKE_CURRENT_SOURCE_DIR}/cmake/cmake_uninstall.cmake.in" "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake" IMMEDIATE @ONLY)
ADD_CUSTOM_TARGET(uninstall "${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake")

# ----------------------------------------------------------------------------
# create configuration file from .in file (If you use windows take care with paths)
# ----------------------------------------------------------------------------

CONFIGURE_FILE("${PROJECT_SOURCE_DIR}/cmake/config.cmake.in" "${PROJECT_BINARY_DIR}/Find${PROJECT_NAME}.cmake")
INSTALL(FILES "${PROJECT_BINARY_DIR}/Find${PROJECT_NAME}.cmake" DESTINATION lib/cmake/ )

# ----------------------------------------------------------------------------
#   PACKAGE GENERATOR
# ----------------------------------------------------------------------------

IF (UNIX)
SET(CPACK_GENERATOR "DEB")
SET(CPACK_PACKAGE_CONTACT "rmsalinas@uco.es")
SET(CPACK_DEBIAN_PACKAGE_MAINTAINER "rmsalinas@uco.es")
SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "ArUco Library")
SET(CPACK_PACKAGE_VENDOR "Rafael Munoz-Salinas. AVA Group. University of Cordoba. Spain")
SET(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
SET(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_VERSION_MINOR})
SET(CPACK_PACKAGE_VERSION_PATCH ${PROJECT_VERSION_PATCH})
SET(CPACK_PACKAGE_INSTALL_DIRECTORY "CMake ${CMake_VERSION_MAJOR}.${CMake_VERSION_MINOR}")
SET(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
SET(CPACK_STRIP_FILES "bin/lib${PROJECT_NAME}")
SET(CPACK_SOURCE_STRIP_FILES "")
SET(CPACK_PACKAGING_INSTALL_PREFIX "/usr/")
IF (CPACK_GENERATOR STREQUAL "DEB")
    SET(CPACK_DEBIAN_PACKAGE_DEPENDS "opencv (>= 2.1.0)")
ENDIF()
INCLUDE(CPack)
ENDIF()

#------------------------------------------------
# DIRS
#------------------------------------------------
ADD_SUBDIRECTORY(src)
IF(NOT ANDROID_CREATION)
  ADD_SUBDIRECTORY(utils)
ENDIF()

# ----------------------------------------------------------------------------
# display status message for important variables
# ----------------------------------------------------------------------------
MESSAGE( STATUS )
MESSAGE( STATUS "-------------------------------------------------------------------------------" )
MESSAGE( STATUS "General configuration for ${PROJECT_NAME} ${PROJECT_VERSION}")
MESSAGE( STATUS "-------------------------------------------------------------------------------" )
MESSAGE( STATUS )
MESSAGE( STATUS "Built as dynamic libs?:  ${BUILD_SHARED_LIBS}")
MESSAGE( STATUS "Compiler:                ${CMAKE_CXX_COMPILER}")
MESSAGE( STATUS "C++ flags (Release):     ${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_RELEASE}")
MESSAGE( STATUS "C++ flags (Debug):       ${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_DEBUG}")
MESSAGE( STATUS "CMAKE_CXX_FLAGS:         ${CMAKE_CXX_FLAGS}")
MESSAGE( STATUS "CMAKE_BINARY_DIR:        ${CMAKE_BINARY_DIR}")

MESSAGE( STATUS )
MESSAGE( STATUS "WARNINGS_ANSI_ISO =      ${WARNINGS_ANSI_ISO}" )
MESSAGE( STATUS "WARNINGS_ARE_ERRORS =    ${WARNINGS_ARE_ERRORS}" )
MESSAGE( STATUS "CMAKE_SYSTEM_PROCESSOR = ${CMAKE_SYSTEM_PROCESSOR}" )
MESSAGE( STATUS "BUILD_SHARED_LIBS =      ${BUILD_SHARED_LIBS}" )
MESSAGE( STATUS "CMAKE_INSTALL_PREFIX =   ${CMAKE_INSTALL_PREFIX}" )
MESSAGE( STATUS "CMAKE_BUILD_TYPE =       ${CMAKE_BUILD_TYPE}" )
MESSAGE( STATUS "CMAKE_MODULE_PATH =      ${CMAKE_MODULE_PATH}" )

MESSAGE( STATUS )
MESSAGE( STATUS "---------------------------     Documentation     -----------------------------" )
MESSAGE( STATUS )
MESSAGE( STATUS "INSTALL_DOC = ${INSTALL_DOC}" )
MESSAGE( STATUS "USE_LATEX = ${USE_LATEX}" )
MESSAGE( STATUS "USE_DOT = ${USE_DOT}" )
MESSAGE( STATUS "USE_CHM = ${USE_CHM}" )
MESSAGE( STATUS )
MESSAGE( STATUS "FOUND OPENGL=${GL_FOUND}    LIBS=${OPENGL_LIBS}")
MESSAGE( STATUS "OpenCV_LIB_DIR=${OpenCV_LIB_DIR}")
MESSAGE( STATUS "CMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}")
MESSAGE( STATUS )
MESSAGE( STATUS )
MESSAGE( STATUS "Change a value with: cmake -D<Variable>=<Value>" )
MESSAGE( STATUS )
