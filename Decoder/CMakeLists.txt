# Standalone C++ uRwell/hodoscope EVIO -> HIPO decoder.
# Replaces the Java coatjava decoder for the test-prototype data.

# --- vendored EVIO-5.2 C library + EVIO event parser ------------------------
add_library(uRwellEvio STATIC
  evio-5.2/evio.c
  evio-5.2/eviofmt.c
  evio-5.2/eviofmtswap.c
  evio-5.2/evioswap.c
  EvioFileReader.cpp
  EventParser.cpp)
target_include_directories(uRwellEvio PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/evio-5.2)
set_target_properties(uRwellEvio PROPERTIES POSITION_INDEPENDENT_CODE ON)

# System SQLite (CCDB translation-table reader). Only libsqlite3.so.0 is
# installed (no dev symlink), so link it by soname.
set(SQLITE3_LIB "-l:libsqlite3.so.0")

# --- reusable decode core (shared by the decoder exe and the event viewer) ---
# Static (like uRwellEvio) so both executables stay self-contained. It holds the
# per-tag raw decoders, the CCDB translation table, and the in-memory pulse
# builder (PulseBuilder), so the viewer can decode EVIO on the fly. It needs ROOT
# (TF1 Landau fit) and uRWellLib (getURwellSlot/getGEMSlot/slot_Offset + structs).
add_library(uRwellDecodeCore STATIC
  uRwellSRSDecoder.cpp
  MarocHodoDecoder.cpp
  TranslationTable.cpp
  PulseBuilder.cpp)
target_include_directories(uRwellDecodeCore PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}/include
  ${ROOT_INCLUDE_DIRS})
target_link_libraries(uRwellDecodeCore PUBLIC uRWellLib ${ROOT_LIBRARIES})
set_target_properties(uRwellDecodeCore PROPERTIES POSITION_INDEPENDENT_CODE ON)

# --- the decoder ------------------------------------------------------------
# The --pulse path reproduces AnaCodes/Skim_PulseFit.cc, so it needs ROOT (TF1
# Landau fit) and uRWellLib (getURwellSlot/getGEMSlot/slot_Offset + structs).
add_executable(uRwellDecoder.exe
  uRwellDecoder.cc
  HipoBankWriter.cpp
  PulseFitWriter.cpp)
target_include_directories(uRwellDecoder.exe PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}/hipo/hipo4
  ${CMAKE_SOURCE_DIR}/include
  ${ROOT_INCLUDE_DIRS})
target_link_libraries(uRwellDecoder.exe
  uRwellEvio uRwellDecodeCore Hipo4 ${SQLITE3_LIB} uRWellLib ${ROOT_LIBRARIES} pthread)

# --- validation tools -------------------------------------------------------
# CompareDecoded: URWELL::adc / XYHODO::tdc multisets of two decoded files.
add_executable(CompareDecoded.exe CompareDecoded.cc)
target_include_directories(CompareDecoded.exe PRIVATE ${CMAKE_SOURCE_DIR}/hipo/hipo4)
target_link_libraries(CompareDecoded.exe Hipo4)

# ComparePulse: uRwell::Pulse multisets (keyed by RUN::config.event) of two
# files, e.g. the fused --pulse output vs decode -> Skim_PulseFit.
add_executable(ComparePulse.exe ComparePulse.cc)
target_include_directories(ComparePulse.exe PRIVATE ${CMAKE_SOURCE_DIR}/hipo/hipo4)
target_link_libraries(ComparePulse.exe Hipo4)

# --- installation (decoder gets its own subdirectory) -----------------------
install(TARGETS uRwellDecoder.exe CompareDecoded.exe ComparePulse.exe
        DESTINATION ${CMAKE_INSTALL_PREFIX}/decoder/bin)
