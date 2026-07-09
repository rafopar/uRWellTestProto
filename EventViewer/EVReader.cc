//
// EVReader.cc
//

#include "EVReader.h"

#include <fstream>
#include <iostream>

bool EVReader::Open(const char *filename) {
    std::ifstream test(filename);
    if (!test.good()) {
        std::cerr << "EVReader::Open: can not open the file " << filename << std::endl;
        return false;
    }
    test.close();

    fFileName = filename;
    Reopen();

    if (fPulseBank == nullptr) {
        std::cerr << "EVReader::Open: the file " << filename
                  << " does not contain the uRwell::Pulse bank" << std::endl;
        return false;
    }

    // Cache every event's raw buffer for random access. hipo's gotoEvent()
    // binary search is unreliable on some files (it can crash), so the file is
    // read once sequentially and each event's bytes are kept in memory; a later
    // ReadEvent(index) re-initialises the working event from the cached bytes.
    fEventBuffers.clear();
    while (fReader->next()) {
        fReader->read(fHipoEvent);
        fEventBuffers.push_back(fHipoEvent.getEventBuffer());
    }
    fEntries = static_cast<int>(fEventBuffers.size());
    return true;
}

void EVReader::Reopen() {
    fReader = std::make_unique<hipo::reader>();
    fReader->open(fFileName.c_str());
    fFactory = hipo::dictionary();
    fReader->readDictionary(fFactory);

    fPulseBank.reset();
    fHodoBank.reset();
    fConfBank.reset();

    if (fFactory.hasSchema("uRwell::Pulse")) {
        fPulseBank = std::make_unique<hipo::bank>(fFactory.getSchema("uRwell::Pulse"));
    }
    if (fFactory.hasSchema("XYHODO::tdc")) {
        fHodoBank = std::make_unique<hipo::bank>(fFactory.getSchema("XYHODO::tdc"));
    }
    if (fFactory.hasSchema("RUN::config")) {
        fConfBank = std::make_unique<hipo::bank>(fFactory.getSchema("RUN::config"));
    }
}

bool EVReader::ReadEvent(int index, EVEvent &ev) {
    if (index < 0 || index >= fEntries) {
        return false;
    }

    // Re-load the working event from its cached bytes (see Open()).
    fHipoEvent.init(fEventBuffers[index]);

    FillEvent(ev, index);
    return true;
}

void EVReader::FillEvent(EVEvent &ev, int index) {
    ev = EVEvent();
    ev.eventIndex = index;

    if (fConfBank != nullptr) {
        fHipoEvent.getStructure(*fConfBank);
        if (fConfBank->getRows() > 0) {
            ev.runNumber = fConfBank->getInt("run", 0);
            ev.trueEventNumber = fConfBank->getInt("event", 0);
        }
    }

    fHipoEvent.getStructure(*fPulseBank);
    int nPulses = fPulseBank->getRows();

    for (int iPulse = 0; iPulse < nPulses; iPulse++) {
        uRwellTools::uRwellHit curHit;
        curHit.sector = fPulseBank->getInt("sec", iPulse);
        curHit.layer = fPulseBank->getInt("layer", iPulse);
        curHit.strip = fPulseBank->getInt("strip", iPulse);
        curHit.stripLocal = fPulseBank->getInt("stripLocal", iPulse);
        curHit.adc = double(fPulseBank->getFloat("adc", iPulse));
        curHit.adcRel = double(fPulseBank->getFloat("adcRel", iPulse));
        curHit.slot = fPulseBank->getInt("slot", iPulse);
        curHit.ts = fPulseBank->getInt("ts", iPulse);

        uRwellTools::APV25Pulse curPulse;
        curPulse.hit = curHit;
        curPulse.ped_rms = double(fPulseBank->getFloat("ped_rms", iPulse));
        curPulse.pulse_p0 = double(fPulseBank->getFloat("pulse_p0", iPulse));
        curPulse.pulse_A0 = double(fPulseBank->getFloat("pulse_A0", iPulse));
        curPulse.pulse_MPV = double(fPulseBank->getFloat("pulse_MPV", iPulse));
        curPulse.pulse_Sigma = double(fPulseBank->getFloat("pulse_Sigma", iPulse));
        curPulse.pulse_Chi2 = double(fPulseBank->getFloat("pulse_Chi2", iPulse));
        curPulse.pulse_NDF = fPulseBank->getInt("pulse_NDF", iPulse);
        curPulse.pulse_Integral = double(fPulseBank->getFloat("pulse_Integral", iPulse));
        for (int ts = 0; ts < 15; ts++) {
            curPulse.pulse_ADC[ts] = double(fPulseBank->getFloat(Form("pulse_ADC%d", ts), iPulse));
        }

        if (curPulse.hit.sector != uRwellTools::sec_TestProto) {
            continue;
        }

        if (curPulse.hit.layer == uRwellTools::layer_U_TestProto) {
            ev.v_U_Pulses.push_back(curPulse);
        } else if (curPulse.hit.layer == uRwellTools::layer_V_TestProto) {
            ev.v_V_Pulses.push_back(curPulse);
        }
    }

    ev.hodoBank = fHodoBank.get();
    if (fHodoBank != nullptr) {
        fHipoEvent.getStructure(*fHodoBank);
        int nHodoHits = fHodoBank->getRows();
        for (int i = 0; i < nHodoHits; i++) {
            EVEvent::HodoTDCHit hit;
            hit.sector = fHodoBank->getInt("sector", i);
            hit.layer = fHodoBank->getInt("layer", i);
            hit.component = fHodoBank->getInt("component", i);
            hit.order = fHodoBank->getInt("order", i);
            hit.tdc = fHodoBank->getInt("TDC", i);
            ev.v_HodoHits.push_back(hit);
        }
    }
}
