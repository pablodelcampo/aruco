if (WIN32 AND CMAKE_GENERATOR MATCHES "(MinGW)|(MSYS)")
  set(CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG" CACHE STRING "")
endif()

if(MSVC)
  if(CMAKE_CXX_FLAGS STREQUAL CMAKE_CXX_FLAGS_INIT)
    # override cmake default exception handling option
    string(REPLACE "/EHsc" "/EHa" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}"  CACHE STRING "Flags used by the compiler during all build types." FORCE)
  endif()
endif()

INCLUDE(cmake/utils.cmake           REQUIRED)

SET(EXTRA_C_FLAGS "")
SET(EXTRA_C_FLAGS_RELEASE "")
SET(EXTRA_C_FLAGS_DEBUG "")
SET(EXTRA_EXE_LINKER_FLAGS "")
SET(EXTRA_EXE_LINKER_FLAGS_RELEASE "")
SET(EXTRA_EXE_LINKER_FLAGS_DEBUG "")

IF(CMAKE_COMPILER_IS_GNUCXX)
  add_extra_compiler_option(-W)
  add_extra_compiler_option(-Wall)
  add_extra_compiler_option(-Werror=return-type)
  #add_extra_compiler_option(-Wno-long-long) #Always necessary for Qt
  
  IF(WARNINGS_ANSI_ISO)
    add_extra_compiler_option(-pedantic)
    add_extra_compiler_option(-ansi)
    add_extra_compiler_option(-Wcast-align)
    add_extra_compiler_option(-Wstrict-aliasing=2)
  ELSE()
    add_extra_compiler_option(-Wno-narrowing)
    add_extra_compiler_option(-Wno-delete-non-virtual-dtor)
    add_extra_compiler_option(-Wno-unnamed-type-template-args)
  ENDIF()

  IF(WARNINGS_ARE_ERRORS)
    add_extra_compiler_option(-Werror)
  ENDIF()

  IF(X86 AND NOT MINGW64 AND NOT X86_64 AND NOT APPLE)
    add_extra_compiler_option(-march=i686)
  ELSEIF(${CMAKE_SYSTEM_PROCESSOR} MATCHES arm*) # Specific flags for beagleboard
    add_extra_compiler_option(-mtune=cortex-a8)
    add_extra_compiler_option(-march=armv7-a)
    add_extra_compiler_option(-mfloat-abi=softfp)
    add_extra_compiler_option(-mfpu=neon)
    add_extra_compiler_option(-ftree-vectorizen)
  ENDIF()

  # Profiling?
  IF(ENABLE_PROFILING)
    add_extra_compiler_option("-pg -g")
    # turn off incompatible options
    FOREACH(flags CMAKE_CXX_FLAGS CMAKE_C_FLAGS CMAKE_CXX_FLAGS_RELEASE CMAKE_C_FLAGS_RELEASE CMAKE_CXX_FLAGS_DEBUG CMAKE_C_FLAGS_DEBUG EXTRA_C_FLAGS_RELEASE)
      string(REPLACE "-fomit-frame-pointer" "" ${flags} "${${flags}}")
      string(REPLACE "-ffunction-sections" "" ${flags} "${${flags}}")
    ENDFOREACH()
  ELSEIF(NOT APPLE AND NOT ANDROID)
    # Remove unreferenced functions: function level linking
    add_extra_compiler_option(-ffunction-sections)
  ENDIF()

  SET(EXTRA_C_FLAGS_RELEASE "${EXTRA_C_FLAGS_RELEASE} -DNDEBUG")
  SET(EXTRA_C_FLAGS_DEBUG "${EXTRA_C_FLAGS_DEBUG} -O0 -DDEBUG -D_DEBUG")
  IF(BUILD_WITH_DEBUG_INFO)
    SET(EXTRA_C_FLAGS_DEBUG "${EXTRA_C_FLAGS_DEBUG} -ggdb3")
  ENDIF()
ENDIF()

IF(MSVC)
  SET(EXTRA_C_FLAGS "${EXTRA_C_FLAGS} /D _CRT_SECURE_NO_DEPRECATE /D _CRT_NONSTDC_NO_DEPRECATE /D _SCL_SECURE_NO_WARNINGS")
  # 64-bit portability warnings, in MSVC80
  IF(MSVC80)
    SET(EXTRA_C_FLAGS "${EXTRA_C_FLAGS} /Wp64")
  ENDIF()

  # Remove unreferenced functions: function level linking
  SET(EXTRA_C_FLAGS "${EXTRA_C_FLAGS} /Gy")
  IF(NOT MSVC_VERSION LESS 1400)
    SET(EXTRA_C_FLAGS "${EXTRA_C_FLAGS} /bigobj")
  ENDIF()
  IF(BUILD_WITH_DEBUG_INFO)
    SET(EXTRA_C_FLAGS_RELEASE "${EXTRA_C_FLAGS_RELEASE} /Zi")
  ENDIF()
ENDIF()

# Extra link libs if the user selects building static libs:
IF(NOT BUILD_SHARED_LIBS AND CMAKE_COMPILER_IS_GNUCXX)
  SET(LINKER_LIBS ${LINKER_LIBS} stdc++)
  SET(EXTRA_C_FLAGS "-fPIC ${EXTRA_C_FLAGS}")
ENDIF()

# Add user supplied extra options (optimization, etc...)
# ==========================================================
set(EXTRA_C_FLAGS "${EXTRA_C_FLAGS}" CACHE INTERNAL "Extra compiler options")
set(EXTRA_C_FLAGS_RELEASE "${EXTRA_C_FLAGS_RELEASE}" CACHE INTERNAL "Extra compiler options for Release build")
set(EXTRA_C_FLAGS_DEBUG "${EXTRA_C_FLAGS_DEBUG}" CACHE INTERNAL "Extra compiler options for Debug build")
set(EXTRA_EXE_LINKER_FLAGS "${EXTRA_EXE_LINKER_FLAGS}" CACHE INTERNAL "Extra linker flags")
set(EXTRA_EXE_LINKER_FLAGS_RELEASE "${EXTRA_EXE_LINKER_FLAGS_RELEASE}" CACHE INTERNAL "Extra linker flags for Release build")
set(EXTRA_EXE_LINKER_FLAGS_DEBUG "${EXTRA_EXE_LINKER_FLAGS_DEBUG}" CACHE INTERNAL "Extra linker flags for Debug build")

#combine all "extra" options
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${EXTRA_C_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${EXTRA_C_FLAGS}")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}  ${EXTRA_C_FLAGS_RELEASE}")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} ${EXTRA_C_FLAGS_RELEASE}")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${EXTRA_C_FLAGS_DEBUG}")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} ${EXTRA_C_FLAGS_DEBUG}")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${EXTRA_EXE_LINKER_FLAGS}")
set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} ${EXTRA_EXE_LINKER_FLAGS_RELEASE}")
set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} ${EXTRA_EXE_LINKER_FLAGS_DEBUG}")

IF(MSVC)
  # avoid warnings from MSVC about overriding the /W* option
  # we replace /W3 with /W4 only for C++ files,
  # since all the 3rd-party libraries OpenCV uses are in C,
  # and we do not care about their warnings.
  STRING(REPLACE "/W3" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
  STRING(REPLACE "/W3" "/W4" CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")
  STRING(REPLACE "/W3" "/W4" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")

  # allow extern "C" functions throw exceptions
  FOREACH(flags CMAKE_C_FLAGS CMAKE_C_FLAGS_RELEASE CMAKE_C_FLAGS_RELEASE CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_RELEASE CMAKE_CXX_FLAGS_DEBUG)
    STRING(REPLACE "/EHsc-" "/EHs" ${flags} "${${flags}}")
    STRING(REPLACE "/EHsc" "/EHs" ${flags} "${${flags}}")
    STRING(REPLACE "/Zm1000" "" ${flags} "${${flags}}")
  ENDFOREACH()

  IF(NOT ENABLE_NOISY_WARNINGS)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4251") #class 'std::XXX' needs to have dll-interface to be used by clients of YYY
  ENDIF()
ENDIF()
