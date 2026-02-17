# - Try to find XYHodoTools
# Once done, this will define
#
#  XYHodoTools_FOUND - system has XYHodoTools
#  XYHodoTools_INCLUDE_DIRS - the XYHodoTools include directories
#  XYHodoTools_LIBRARIES - link these to use XYHodoTools

# Look for the header file
find_path(XYHodoTools_INCLUDE_DIR
  NAMES XYHodoTools.h
  PATHS /home/rafopar/work/git/XYHodo/include/
)

# Look for the library file
find_library(XYHodoTools_LIBRARY
  NAMES XYHodoLib
  PATHS /home/rafopar/work/builds/XYHodo/lib/
)

# Check if we found everything
if(XYHodoTools_INCLUDE_DIR AND XYHodoTools_LIBRARY)
  set(XYHodoTools_FOUND TRUE)
  message(STATUS "FOUND XYHodo Tools")
else()
  message(STATUS "XYHodoTools is not FOUND!!")
  set(XYHodoTools_FOUND FALSE)
endif()

# Provide the include directories and libraries
if(XYHodoTools_FOUND)
  message(STATUS "Setting XYHodoTools_INCLUDE_DIRS and XYHodoTools_LIBRARIES variables...")
  set(XYHodoTools_INCLUDE_DIRS ${XYHodoTools_INCLUDE_DIR})
  set(XYHodoTools_LIBRARIES ${XYHodoTools_LIBRARY})

  message(STATUS "... XYHodoTools_INCLUDE_DIRS = ${XYHodoTools_INCLUDE_DIRS}")
  message(STATUS "... XYHodoTools_LIBRARIES = ${XYHodoTools_LIBRARIES}")
endif()

# Mark the cache entries as advanced
mark_as_advanced(XYHodoTools_INCLUDE_DIR XYHodoTools_LIBRARY)
