find_package(ROOT REQUIRED COMPONENTS Gui Gpad Graf Hist RIO)

set(INCLUDES
  ${ROOT_INCLUDE_DIRS}
  ${CMAKE_SOURCE_DIR}/hipo/hipo4
  ${XYHodoTools_INCLUDE_DIRS})
include_directories(${INCLUDES})

################################  uRwellEventViewer  ###########################################
add_executable(uRwellEventViewer
        uRwellEventViewer.cc
        EVMainFrame.cc
        EVReader.cc
        EVCutEngine.cc
        EVGeometry.cc
        EVPulsesTab.cc
        EVHitsTab.cc
        EVClustersTab.cc)
set_target_properties(uRwellEventViewer PROPERTIES OUTPUT_NAME "uRwellEventViewer.exe")
target_link_libraries(uRwellEventViewer Hipo4 uRWellLib ${ROOT_LIBRARIES} ROOT::Gui)
#################################################################################################

install(TARGETS uRwellEventViewer DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
