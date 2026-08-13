################################ Installing Python scripts ################################
install(FILES extract_section_geometry.py plot_HV_sections.py
        DESTINATION ${CMAKE_INSTALL_PREFIX}/misc/HV_Section_Quality
        PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                    GROUP_READ GROUP_EXECUTE
                    WORLD_READ WORLD_EXECUTE)

################################ Installing input data files ##############################
# The section geometry is extracted from the CAD file with extract_section_geometry.py,
# plot_HV_sections.py needs it together with the measured max stable HV values.
install(FILES uRwell_HV_section_geometry.dat
              TOP_HV_Sections_90Ar_7Iso_3CO2.dat
              BOT_HV_Sections_90Ar_7Iso_3CO2.dat
        DESTINATION ${CMAKE_INSTALL_PREFIX}/misc/HV_Section_Quality
        PERMISSIONS OWNER_READ OWNER_WRITE
                    GROUP_READ
                    WORLD_READ)
