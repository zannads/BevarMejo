# ==============================================================================
# EPANET Configuration
# ==============================================================================
#
# Unlike externally managed libraries (vcpkg, JSON, Pagmo), EPANET lacks frequent
# releases and patch versioning.
# To ensure traceability, we maintain a fork of the OWA-EPANET repository.
# We track the library version using the commit date from its dev (or master) branch.
# The fork's branch (`fork_repo/beme/EN_vYY.mm.dd`) branches off at a given commit. 
# That commit is tagged like this: `beme-EN_vYY.mm.dd`. 
# The same branch also offers another variation with the suffix `-quiet`.
# This indicates the same commit with one additional commit turning off printing
# during simulation and is helpful to include in optimisations.
# ==============================================================================

set(EPANET_VERSION 250930) # DEFAULT: Commit at 2025-09-30 on Master (EPANET v2.3.3)

if(BEME_VERSION LESS 250200)
  # Before February 2025, we were using EPANET with the commit at 18th June 2024
  set(EPANET_VERSION 240618)
  message(WARNING "Using EPANET previous version (24-06-18). Results differ from latest.")
elseif(BEME_VERSION LESS 251200)
  # Before December 2025 we were using EPANET with the commit at 21st December 2024
  set(EPANET_VERSION 241221)
  message(WARNING "Using EPANET previous version (24-12-21). Results differ from latest.")
endif()

# Some code changes based on EPANET version so let's define the preprocessor definition
add_definitions(-DEPANET_VERSION=${EPANET_VERSION})

# ------------------------------------------------------------------------------
# Determine EPANET version tag and directory
# ------------------------------------------------------------------------------
math(EXPR EN_YY "${EPANET_VERSION} / 10000")
math(EXPR EN_MM "(${EPANET_VERSION} % 10000) / 100")
math(EXPR EN_DD "${EPANET_VERSION} % 100")
set(EPANET_VERSION_TAG "${EN_YY}.${EN_MM}.${EN_DD}")

# Default: worktree in extern/EPANET.beme/
# Override with: -DEPANET_DIR=/custom/path
set(EPANET_DIR "${PROJECT_SOURCE_DIR}/extern/EPANET.beme/${EPANET_VERSION_TAG}"
  CACHE PATH "Path to the exact EPANET version root directory (worktree source).")

# Verify worktree exists
if(NOT EXISTS "${EPANET_DIR}/src")
  message(FATAL_ERROR "EPANET source not found at: ${EPANET_DIR}")
endif()

# ------------------------------------------------------------------------------
# Collect source files
# ------------------------------------------------------------------------------
set(EPANET_SRC
    "${EPANET_DIR}/src/epanet.c"
    # "${EPANET_DIR}/src/epanet2.c"
    "${EPANET_DIR}/src/genmmd.c"
    "${EPANET_DIR}/src/hash.c"
    "${EPANET_DIR}/src/hydcoeffs.c"
    "${EPANET_DIR}/src/hydraul.c"
    "${EPANET_DIR}/src/hydsolver.c"
    "${EPANET_DIR}/src/hydstatus.c"
    "${EPANET_DIR}/src/inpfile.c"
    "${EPANET_DIR}/src/input1.c"
    "${EPANET_DIR}/src/input2.c"
    "${EPANET_DIR}/src/input3.c"
    "${EPANET_DIR}/src/mempool.c"
    "${EPANET_DIR}/src/output.c"
    "${EPANET_DIR}/src/project.c"
    "${EPANET_DIR}/src/quality.c"
    "${EPANET_DIR}/src/qualreact.c"
    "${EPANET_DIR}/src/qualroute.c"
    "${EPANET_DIR}/src/report.c"
    "${EPANET_DIR}/src/rules.c"
    "${EPANET_DIR}/src/smatrix.c"
    "${EPANET_DIR}/src/validate.c"
    "${EPANET_DIR}/src/util/cstr_helper.c"
    "${EPANET_DIR}/src/util/errormanager.c"
    "${EPANET_DIR}/src/util/filemanager.c"
  )

# Add leakage support for versions after 240712 (PR #808)
if(EPANET_VERSION GREATER 240712)
    list(APPEND EPANET_SRC
        "${EPANET_DIR}/src/leakage.c"
        "${EPANET_DIR}/src/flowbalance.c"
    )
endif()

# ------------------------------------------------------------------------------
# Create EPANET library target
# ------------------------------------------------------------------------------
add_library(epanet ${EPANET_SRC})

# All API functions use DLLEXPORT macro. Since we're building from source,
# we need to empty-define it to prevent export declarations on Windows.
target_compile_definitions(epanet PUBLIC 
    "DLLEXPORT=" 
    "EPANET_VERSION=${EPANET_VERSION}"
)

target_include_directories(epanet PUBLIC 
    "${EPANET_DIR}/include"
    "${EPANET_DIR}/src"
)

# Always optimize EPANET (even when bemelib is in Debug mode)
target_compile_options(epanet PRIVATE
    $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:-O2>
    $<$<CXX_COMPILER_ID:MSVC>:/O2>
)

message(STATUS "EPANET target configured (using: ${EPANET_DIR})")

# ELSE() # NOT EPANET FROM SOURCE

#   # Normal behaviour, we are using the library.
#   add_library(epanet SHARED IMPORTED)
  
#   # However, based on the operating system, we need to set the correct path to the library.
#   # We also need to adjust for the EPANET version. We require that EPANET is 
#   # built in builds/VERSION_YY.VERSION_MM.VERSION_DD folder for previous versions
#   # and in build for the latest version.
#   set(EPANET_BUILD_DIR "${EPANET_ROOT}/build")
#   IF(BEME_VERSION LESS CURRENT_VERSION)
#     math(EXPR MAJOR_VERSION "${EPANET_VERSION} / 10000")
#     math(EXPR MINOR_VERSION "(${EPANET_VERSION} % 10000) / 100")
#     math(EXPR PATCH_VERSION "${EPANET_VERSION} % 100")
#     set(EPANET_BUILD_DIR "${EPANET_ROOT}/beme-releases/${MAJOR_VERSION}.${MINOR_VERSION}.${PATCH_VERSION}")
#   ENDIF()
#   message(STATUS "EPANET build directory: ${EPANET_BUILD_DIR}")

#   # When using Visual Studio on Windows, cmake is a multi-configuration generator.
#   # We need to set the path for Release configuration. On Unix, makefiles are usually
#   # used, so it is a single configuration to the folder.

#   # To expose the library to the system we define the IMPORTED_LOCATION property to the target.
#   IF (WIN32)
#     set_target_properties(epanet PROPERTIES
#       IMPORTED_LOCATION "${EPANET_BUILD_DIR}/bin/Release/epanet2.dll"
#       IMPORTED_IMPLIB   "${EPANET_BUILD_DIR}/lib/Release/epanet2.lib"
#     )
#   ENDIF(WIN32)
#   IF(APPLE)
#     set_target_properties(epanet PROPERTIES
#       IMPORTED_LOCATION "${EPANET_BUILD_DIR}/lib/libepanet2.dylib"
#     )
#   ENDIF(APPLE)
#   IF(LINUX)
#     set_target_properties(epanet PROPERTIES
#       IMPORTED_LOCATION "${EPANET_BUILD_DIR}/lib/libepanet2.so"
#     )
#   ENDIF(LINUX)

#   # Finally, we need to include the directories where the headers are located so
#   # that my library can see them.
#   target_include_directories(epanet INTERFACE "${EPANET_ROOT}/include" "${EPANET_ROOT}/src")

# ENDIF()
