#ifndef URWELL_PULSE_FIT_WRITER_H
#define URWELL_PULSE_FIT_WRITER_H

#include "dictionary.h"
#include "writer.h"
#include "event.h"
#include "bank.h"
#include "HipoBankWriter.h"   // reuse AdcRow / TdcRow row structs

#include <string>
#include <vector>
#include <unordered_map>

class TF1;

// Optional output mode of the decoder: instead of the gigantic URWELL::adc bank,
// reproduce exactly what AnaCodes/Skim_PulseFit.cc does, i.e. build the per-strip
// waveforms, keep only strips whose pedestal-subtracted ADC passes the 3-sigma
// threshold, Landau-fit each surviving waveform, and write the small
// uRwell::Pulse bank (+ an empty RAW::adc, RUN::config, XYHODO::tdc pass-through).
//
// The decode -> Skim_PulseFit two-step and this fused single step must produce
// an identical uRwell::Pulse bank.  To guarantee that, this class is a 1:1 port
// of the Skim_PulseFit event body; the only difference is that the waveforms are
// fed from the decoder's in-memory hit list rather than read back from a
// URWELL::adc bank on disk.
class PulseFitWriter
{
public:
    using AdcRow = HipoBankWriter::AdcRow;
    using TdcRow = HipoBankWriter::TdcRow;

    // pedDir/Peds_<run> (uRwell, sector 6) and pedDir/GEM_Peds_<run> (GEM,
    // sector 8) are loaded here, exactly as Skim_PulseFit does.  Returns false
    // if either pedestal file is missing.
    bool open(const std::string &filename, int run, const std::string &pedDir);
    void close() { fWriter.close(); }

    // One physics event.  Mirrors the Skim_PulseFit per-event logic, including
    // its skip rules: nothing is written when there are no uRwell ADC rows or no
    // strip passes the threshold.
    void writeEvent(int run, int event, int unixtime,
                    const std::vector<AdcRow> &adc,
                    const std::vector<TdcRow> &tdc);

    long pulsesWritten() const { return fPulses; }
    long eventsWritten() const { return fEvents; }

private:
    hipo::writer fWriter;
    hipo::schema fSchPulse;   // uRwell::Pulse
    hipo::schema fSchRaw;     // RAW::adc   (passed through empty)
    hipo::schema fSchCfg;     // RUN::config
    hipo::schema fSchHodo;    // XYHODO::tdc

    TF1 *fFit = nullptr;      // [0] + [1]*Landau(x,[2],[3]), p0 fixed to 0

    // Pedestals keyed by unique channel, as in Skim_PulseFit.
    std::unordered_map<int, double> fPedMean,     fPedRms;
    std::unordered_map<int, double> fPedGEMMean,  fPedGEMRms;

    long fPulses = 0, fEvents = 0;

    static bool loadPed(const std::string &path,
                        std::unordered_map<int, double> &mean,
                        std::unordered_map<int, double> &rms);
};

#endif
