
# Calculate version as a single integer for easy comparison
math(EXPR BEME_VERSION_INT 
    "${PROJECT_VERSION_MAJOR} * 10000 + ${PROJECT_VERSION_MINOR} * 100 + ${PROJECT_VERSION_PATCH}")

# Check that the project version is a valid version string (
# in bounds/years, no BEME_PATCH version greater than 99, BEME_MINOR version is a valid month
set(BEME_FIRST_VERSION_INT 230600)
if(
  BEME_VERSION_INT LESS BEME_FIRST_VERSION_INT 
  OR BEME_VERSION_INT GREATER BEME_LATEST_VERSION_INT 
  OR PROJECT_VERSION_PATCH GREATER 99 
  OR PROJECT_VERSION_MINOR GREATER 12 
  OR PROJECT_VERSION_MINOR EQUAL 0)
  message(FATAL_ERROR "Invalid version string: ${PROJECT_VERSION}")
endif()

# Determine minimum compatible version based on current version
if(BEME_VERSION_INT LESS 240401)
    set(BEME_MIN_VERSION_INT ${BEME_FIRST_VERSION_INT})
elseif(BEME_VERSION_INT LESS 240601)
    set(BEME_MIN_VERSION_INT 240401)
elseif(BEME_VERSION_INT LESS 241100)
    set(BEME_MIN_VERSION_INT 240601)
elseif(BEME_VERSION_INT LESS 241200)
    set(BEME_MIN_VERSION_INT 241100)
elseif(BEME_VERSION_INT LESS 250200)
    set(BEME_MIN_VERSION_INT 241200)
elseif(BEME_VERSION_INT LESS 251200)
    set(BEME_MIN_VERSION_INT 250200)
else() # BEME_VERSION_INT >= 251200
    set(BEME_MIN_VERSION_INT 251200)
endif()

# ==============================================================================
# Unlike externally managed libraries (vcpkg, JSON, Pagmo), EPANET lacks frequent
# releases and patch versioning.
# To ensure traceability, we maintain a fork of the OWA-EPANET repository.
# We track the library version using the commit date from its dev (or master) branch.
# The fork's branch (`fork_repo/beme/en-vYY.mm.dd`) branches off at a given commit 
# with tag `beme-en-vYY.mm.dd`. 
# ==============================================================================

set(BEME_EN_VERSION_INT 250930) # DEFAULT: Commit at 2025-09-30 on Master (EPANET v2.3.3)

if(BEME_VERSION_INT LESS 250200)
  # Before February 2025, we were using EPANET with the commit at 18th June 2024
  set(BEME_EN_VERSION_INT 240618)
  message(WARNING "Using EPANET previous version (24-06-18). Results differ from latest.")
elseif(BEME_VERSION_INT LESS 251200)
  # Before December 2025 we were using EPANET with the commit at 21st December 2024
  set(BEME_EN_VERSION_INT 241221)
  message(WARNING "Using EPANET previous version (24-12-21). Results differ from latest.")
endif()

# Add version-specific preprocessor definitions
add_compile_definitions(
    BEME_VERSION=${BEME_VERSION_INT}
    BEME_MIN_VERSION=${BEME_MIN_VERSION_INT}
    # BEME_EN_VERSION=${BEME_EN_VERSION_INT} this is defined as public in target epanet only
)
