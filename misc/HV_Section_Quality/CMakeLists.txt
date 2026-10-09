################################ Installing Python scripts ################################
install(FILES extract_section_geometry.py plot_HV_sections.py
        DESTINATION ${CMAKE_INSTALL_PREFIX}/misc/HV_Section_Quality
        PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                    GROUP_READ GROUP_EXECUTE
                    WORLD_READ WORLD_EXECUTE)

################################ Installing input data files ##############################
# The section geometry is generated once from the CAD file with
# extract_section_geometry.py and never edited by hand.
#
# The measurement files (TOP/BOT_HV_Sections_<gas>.dat) are installed as well, so every
# 'make install' overwrites the installed copies: edit them in the repository, not in
# the install directory.
install(FILES uRwell_HV_section_geometry.dat
              TOP_HV_Sections_90Ar_7CO2_3Iso.dat BOT_HV_Sections_90Ar_7CO2_3Iso.dat
              TOP_HV_Sections_UHPN2.dat          BOT_HV_Sections_UHPN2.dat
        DESTINATION ${CMAKE_INSTALL_PREFIX}/misc/HV_Section_Quality
        PERMISSIONS OWNER_READ OWNER_WRITE
                    GROUP_READ
                    WORLD_READ)

################################ Installing output directory ##############################
install(DIRECTORY
        DESTINATION ${CMAKE_INSTALL_PREFIX}/misc/HV_Section_Quality/Figs)
