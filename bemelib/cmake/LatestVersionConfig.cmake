# Allow manually specifying an older version during configuration, if not, default to current.
set(BEME_LATEST_VERSION_INT 260100)

if(NOT PROJECT_VERSION OR PROJECT_VERSION STREQUAL "latest")
  math(EXPR BEME_LATEST_MAJOR "${BEME_LATEST_VERSION_INT} / 10000")
  math(EXPR BEME_LATEST_MINOR "(${BEME_LATEST_VERSION_INT} % 10000) / 100")
  math(EXPR BEME_LATEST_PATCH "${BEME_LATEST_VERSION_INT} % 100")
  set(PROJECT_VERSION "${BEME_LATEST_MAJOR}.${BEME_LATEST_MINOR}.${BEME_LATEST_PATCH}")
endif()

# This file will be included in the main CMakeLists file so the current source dir
# should be monorepo/bemelib
file(WRITE "${CMAKE_CURRENT_SOURCE_DIR}/../pybeme/src/pybeme/VERSION"
  "${PROJECT_VERSION}")