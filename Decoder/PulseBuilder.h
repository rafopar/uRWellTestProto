#ifndef URWELL_PULSE_BUILDER_H
#define URWELL_PULSE_BUILDER_H

#include "DecoderTypes.h"

#include <uRwellTools.h>

#include <string>
#include <unordered_map>
#include <vector>

class TF1;

// In-memory port of the pulse-building part of AnaCodes/Skim_PulseFit.cc /
// Decoder/PulseFitWriter.cc.  From the translated ADC rows of one event it
// builds the per-strip waveforms, keeps the strips whose pedestal-subtracted
// ADC passes the 3-sigma threshold, Landau-fits each surviving waveform, and
// returns the resulting uRwell (sector 6) and GEM (sector 8) pulses.
//
// The event viewer uses this to decode EVIO on the fly so it shows exactly the
// pulses the decoded HIPO file would contain.  The logic is kept a faithful
// copy of PulseFitWriter::writeEvent (which is left untouched so the production
// decoder stays byte-identical); see that file for the reference comments.
class PulseBuilder {
public:
    ~PulseBuilder();

    // Loads pedDir/Peds_<run> (uRwell) and pedDir/GEM_Peds_<run> (GEM).
    // Returns false if either pedestal file is missing.
    bool loadPedestals(int run, const std::string &pedDir);

    // Builds the pulses of one event from its translated ADC rows.  The two
    // output vectors are cleared first.
    void build(const std::vector<DecodedAdcRow> &adc,
               std::vector<uRwellTools::APV25Pulse> &uRwellPulses,
               std::vector<uRwellTools::APV25Pulse> &gemPulses);

private:
    TF1 *fFit = nullptr;
    std::unordered_map<int, double> fPedMean, fPedRms;
    std::unordered_map<int, double> fPedGEMMean, fPedGEMRms;

    void ensureFit();
    static bool loadPed(const std::string &path,
                        std::unordered_map<int, double> &mean,
                        std::unordered_map<int, double> &rms);
};

#endif
