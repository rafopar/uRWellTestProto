################################ Installing Python scripts ################################
install(FILES extract_section_geometry.py plot_HV_sections.py
        DESTINATION ${CMAKE_INSTALL_PREFIX}/misc/HV_Section_Quality
        PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                    GROUP_READ GROUP_EXECUTE
                    WORLD_READ WORLD_EXECUTE)

################################ Installing input data files ##############################
# Only the section geometry is installed. It is generated once from the CAD file with
# extract_section_geometry.py and never edited by hand.
#
# The measurement files (TOP/BOT_HV_Sections_*.dat) are deliberately NOT installed:
# they are edited in place in the install directory while measuring, and installing
# them would overwrite those edits on every 'make install'.
install(FILES uRwell_HV_section_geometry.dat
        DESTINATION ${CMAKE_INSTALL_PREFIX}/misc/HV_Section_Quality
        PERMISSIONS OWNER_READ OWNER_WRITE
                    GROUP_READ
                    WORLD_READ)

################################ Installing output directory ##############################
install(DIRECTORY
        DESTINATION ${CMAKE_INSTALL_PREFIX}/misc/HV_Section_Quality/Figs)
