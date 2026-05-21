################################ Installing Python script #################################
install(FILES HVScanPlotter.py
        DESTINATION ${CMAKE_INSTALL_PREFIX}/misc/HVScans
        PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                    GROUP_READ GROUP_EXECUTE
                    WORLD_READ WORLD_EXECUTE)

################################ Installing output directory ##############################
install(DIRECTORY
        DESTINATION ${CMAKE_INSTALL_PREFIX}/misc/HVScans/Figs)
