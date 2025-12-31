# ==============================================================================
# EPANET Configuration
# ==============================================================================

# ------------------------------------------------------------------------------
# Determine EPANET version tag and directory
# ------------------------------------------------------------------------------
option(REPRODUCIBILITY_MODE "Use reproducibility mode for EPANET" OFF)

if(NOT DEFINED EPANET_DIR)
  if(REPRODUCIBILITY_MODE)
    math(EXPR EN_YY "${BEME_EN_VERSION_INT} / 10000")
    math(EXPR EN_MM "(${BEME_EN_VERSION_INT} % 10000) / 100")
    math(EXPR EN_DD "${BEME_EN_VERSION_INT} % 100")
    set(EPANET_VERSION_TAG "${EN_YY}.${EN_MM}.${EN_DD}")
    # Default: worktree in extern/EPANET.beme/
    # Override with: -DEPANET_DIR=/custom/path
    set(EPANET_DIR "${PROJECT_SOURCE_DIR}/extern/EPANET.beme/${EPANET_VERSION_TAG}")
  else()
    set(EPANET_DIR "${PROJECT_SOURCE_DIR}/extern/EPANET")
  endif()
endif()

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
if(BEME_EN_VERSION_INT GREATER 240712)
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
    "EPANET_VERSION=${BEME_EN_VERSION_INT}"
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
