find_package(ROOT REQUIRED COMPONENTS Gui Gpad Graf Hist RIO)

# System SQLite (used transitively by the decode core's CCDB translation table).
# Only libsqlite3.so.0 is installed (no dev symlink), so link it by soname.
set(SQLITE3_LIB "-l:libsqlite3.so.0")

set(INCLUDES
  ${ROOT_INCLUDE_DIRS}
  ${CMAKE_SOURCE_DIR}/hipo/hipo4
  ${CMAKE_SOURCE_DIR}/Decoder
  ${XYHodoTools_INCLUDE_DIRS})
include_directories(${INCLUDES})

################################  uRwellEventViewer  ###########################################
add_executable(uRwellEventViewer
        uRwellEventViewer.cc
        EVMainFrame.cc
        EVReader.cc
        EVEvioReader.cc
        EVCutEngine.cc
        EVGeometry.cc
        EVPulsesTab.cc
        EVHitsTab.cc
        EVClustersTab.cc
        EVRawDataTab.cc
        EVHodoHitsTab.cc
        EVHodoCrossesTab.cc)
set_target_properties(uRwellEventViewer PROPERTIES OUTPUT_NAME "uRwellEventViewer.exe")
target_link_libraries(uRwellEventViewer
        Hipo4 uRWellLib uRwellEvio uRwellDecodeCore ${SQLITE3_LIB}
        ${ROOT_LIBRARIES} ROOT::Gui ${XYHodoTools_LIBRARY})
#################################################################################################

install(TARGETS uRwellEventViewer DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
