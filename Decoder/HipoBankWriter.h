#ifndef URWELL_HIPO_BANK_WRITER_H
#define URWELL_HIPO_BANK_WRITER_H

#include "dictionary.h"
#include "writer.h"
#include "event.h"
#include "bank.h"

#include <string>
#include <vector>

// Writes the decoded banks (URWELL::adc, XYHODO::tdc, RUN::config) into a HIPO
// file with the same schemas the Java decoder uses (from etc/bankdefs/hipo4).
//
// Performance notes: column indices are resolved once (open) and the integer
// index put-overloads are used, avoiding a per-cell string lookup; a single
// event buffer is reused across events instead of being reallocated.
class HipoBankWriter
{
public:
    // Already-translated output rows.
    struct AdcRow { int sector, layer, component, order, adc, ped; };
    struct TdcRow { int sector, layer, component, order, tdc; };

    void open(const std::string &filename);
    void close() { fWriter.close(); }

    // Writes one HIPO event. RUN::config is always written; the data banks are
    // written whenever they have rows.
    void writeEvent(int run, int event, int unixtime,
                    const std::vector<AdcRow> &adc,
                    const std::vector<TdcRow> &tdc);

private:
    hipo::writer fWriter;
    hipo::schema fSchURW;
    hipo::schema fSchHODO;
    hipo::schema fSchCFG;
    hipo::event  fEvent;          // reused across events

    // Cached column indices (entry order within each schema).
    int iU_sector, iU_layer, iU_comp, iU_order, iU_adc, iU_time, iU_ped;
    int iH_sector, iH_layer, iH_comp, iH_order, iH_tdc;
    int iC_run, iC_event, iC_unixtime, iC_trigger, iC_timestamp,
        iC_type, iC_mode, iC_torus, iC_solenoid;
};

#endif
