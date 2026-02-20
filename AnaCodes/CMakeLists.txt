set(INCLUDES
  ${ROOT_INCLUDE_DIR}
  ${CMAKE_SOURCE_DIR}/build/include
  ${CMAKE_SOURCE_DIR}/hipo/hipo4
  ${XYHodoTools_INCLUDE_DIRS})
include_directories(${INCLUDES})
link_directories(${CMAKE_SOURCE_DIR}/build/lib)
set(CMAKE_VERBOSE_MAKEFILE ON)


################################  SkimZeroSuppression  ##########################################
add_executable(SkimZeroSuppression.exe SkimZeroSuppression.cc)
target_link_libraries(SkimZeroSuppression.exe Hipo4 uRWellLib ${ROOT_LIBRARIES} )
#################################################################################################

################################  CheckDecoding.cc  #############################################
add_executable(CheckDecoding.exe CheckDecoding.cc)
target_link_libraries(CheckDecoding.exe Hipo4 uRWellLib ${ROOT_LIBRARIES} )
#################################################################################################

################################  AnaClustering.cc  #############################################
add_executable(AnaClustering.exe AnaClustering.cc)
target_link_libraries(AnaClustering.exe Hipo4 uRWellLib ${ROOT_LIBRARIES} )
#################################################################################################

############################  DrawPlotsWithClustering.cc  #######################################
add_executable(DrawPlotsWithClustering.exe DrawPlotsWithClustering.cc )
target_link_libraries(DrawPlotsWithClustering.exe uRWellLib ${ROOT_LIBRARIES} )
#################################################################################################

############################  DrawHVDependencePlots.cc  #########################################
add_executable(DrawHVDependencePlots.exe DrawHVDependencePlots.cc)
target_link_libraries(DrawHVDependencePlots.exe uRWellLib ${ROOT_LIBRARIES} )
#################################################################################################

############################  DrawHVDependencePlots.cc  #########################################
add_executable(DrawNoise_Vs_StripCoorelations.exe DrawNoise_Vs_StripCoorelations.cc)
target_link_libraries(DrawNoise_Vs_StripCoorelations.exe uRWellLib ${ROOT_LIBRARIES} )
#################################################################################################

add_executable(Skim_PulseFit Skim_PulseFit.cc)
set_target_properties(Skim_PulseFit PROPERTIES OUTPUT_NAME "Skim_PilseFit.exe")
target_link_libraries(Skim_PulseFit Hipo4 uRWellLib ${ROOT_LIBRARIES})

add_executable(AnaPulseFits AnaPulseFits.cc)
set_target_properties(AnaPulseFits PROPERTIES OUTPUT_NAME "AnaPulseFits.exe")
target_link_libraries(AnaPulseFits Hipo4 uRWellLib ${ROOT_LIBRARIES} ${XYHodoTools_LIBRARY})

add_executable(DrawPulseFitPlots DrawPulseFitPlots.cc)
set_target_properties(DrawPulseFitPlots PROPERTIES OUTPUT_NAME "DrawPulseFitPlots.exe" )
target_link_libraries(DrawPulseFitPlots uRWellLib ${ROOT_LIBRARIES})

add_executable(UpdateStripSigmas UpdateStripSigmas.cc)
set_target_properties(UpdateStripSigmas PROPERTIES OUTPUT_NAME "UpdateStripSigmas.exe")
target_link_libraries(UpdateStripSigmas ${ROOT_LIBRARIES})

add_executable(DrawEffWithHodo DrawEffWithHodo.cc)
set_target_properties(DrawEffWithHodo PROPERTIES OUTPUT_NAME "DrawEffWithHodo.exe")
target_link_libraries(DrawEffWithHodo uRWellLib ${ROOT_LIBRARIES} ${XYHodoTools_LIBRARY} )

################################ Installing Executable targets #############################
install( TARGETS SkimZeroSuppression.exe CheckDecoding.exe AnaClustering.exe DrawHVDependencePlots.exe DrawPlotsWithClustering.exe Skim_PulseFit
DrawNoise_Vs_StripCoorelations.exe AnaPulseFits DrawPulseFitPlots UpdateStripSigmas DrawEffWithHodo
  DESTINATION ${CMAKE_INSTALL_PREFIX}/bin )

################################ Installing Non target files ###############################
install( FILES DrawPedestals.cc Decode_Run.py RunAnaChain.py
  DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)

################################ Installing needed directores ##############################
install( DIRECTORY
  DESTINATION ${CMAKE_INSTALL_PREFIX}/bin/PedFiles)
install( DIRECTORY
  DESTINATION ${CMAKE_INSTALL_PREFIX}/bin/Figs )
install( DIRECTORY
  DESTINATION ${CMAKE_INSTALL_PREFIX}/bin/Pars )



