#ifndef URWELL_DECODER_TYPES_H
#define URWELL_DECODER_TYPES_H

#include <cstdint>

// One decoded uRwell SRS-APV sample: a (crate,slot,channel) at a given time
// sample, with the common-mode-subtracted ADC value. Mirrors the per-(channel,
// ts) ADCData produced by the Java getDataEntries_57631.
struct URwellRawHit {
    int crate;
    int slot;       // = APV hybrid id
    int channel;    // 0..127
    int ts;         // time sample index (-> "ped" column)
    int adc;        // (int)(sample - commonMode)  (-> "ADC" column)
};

// One decoded MAROC hodoscope TDC hit, before translation.
struct HodoRawHit {
    int crate;
    int slot;       // = slotByte + HodoSlotOffset(20)
    int channel;    // = TT_Channel = 2*(ch - (ch/64)*64) + edge
    int tdc;        // rawtdc & 0x7FFF
};

// A translated ADC row (post-CCDB): the input to pulse building / hipo writing.
// Structurally identical to HipoBankWriter::AdcRow but declared here so the
// decode-core library (PulseBuilder) does not need the hipo writer headers.
struct DecodedAdcRow {
    int sector, layer, component, order, adc, ped;
};

// A translated hodoscope TDC row (post-CCDB).
struct DecodedTdcRow {
    int sector, layer, component, order, tdc;
};

#endif
