#include "HipoBankWriter.h"

void HipoBankWriter::open(const std::string &filename)
{
    // Schema definitions copied from etc/bankdefs/hipo4 (group/item/entries).
    fSchURW  = hipo::schema("URWELL::adc", 22300, 11);
    fSchURW.parse("sector/B,layer/B,component/S,order/B,ADC/I,time/F,ped/S");

    fSchHODO = hipo::schema("XYHODO::tdc", 21850, 13);
    fSchHODO.parse("sector/B,layer/B,component/S,order/B,TDC/I");

    fSchCFG  = hipo::schema("RUN::config", 10000, 11);
    fSchCFG.parse("run/I,event/I,unixtime/I,trigger/L,timestamp/L,type/B,mode/B,torus/F,solenoid/F");

    // Resolve column indices once (avoids per-cell string lookups while filling).
    iU_sector = fSchURW.getEntryOrder("sector");
    iU_layer  = fSchURW.getEntryOrder("layer");
    iU_comp   = fSchURW.getEntryOrder("component");
    iU_order  = fSchURW.getEntryOrder("order");
    iU_adc    = fSchURW.getEntryOrder("ADC");
    iU_time   = fSchURW.getEntryOrder("time");
    iU_ped    = fSchURW.getEntryOrder("ped");

    iH_sector = fSchHODO.getEntryOrder("sector");
    iH_layer  = fSchHODO.getEntryOrder("layer");
    iH_comp   = fSchHODO.getEntryOrder("component");
    iH_order  = fSchHODO.getEntryOrder("order");
    iH_tdc    = fSchHODO.getEntryOrder("TDC");

    iC_run       = fSchCFG.getEntryOrder("run");
    iC_event     = fSchCFG.getEntryOrder("event");
    iC_unixtime  = fSchCFG.getEntryOrder("unixtime");
    iC_trigger   = fSchCFG.getEntryOrder("trigger");
    iC_timestamp = fSchCFG.getEntryOrder("timestamp");
    iC_type      = fSchCFG.getEntryOrder("type");
    iC_mode      = fSchCFG.getEntryOrder("mode");
    iC_torus     = fSchCFG.getEntryOrder("torus");
    iC_solenoid  = fSchCFG.getEntryOrder("solenoid");

    fWriter.getDictionary().addSchema(fSchURW);
    fWriter.getDictionary().addSchema(fSchHODO);
    fWriter.getDictionary().addSchema(fSchCFG);
    fWriter.open(filename.c_str());

    // One reusable event buffer (a full URWELL::adc bank is ~0.35 MB; the
    // default 128 KB capacity is too small, and reallocating per event is slow).
    fEvent = hipo::event(4 * 1024 * 1024);
}

void HipoBankWriter::writeEvent(int run, int event, int unixtime,
                                const std::vector<AdcRow> &adc,
                                const std::vector<TdcRow> &tdc)
{
    fEvent.reset();

    // RUN::config (always, one row). Matches CLASDecoder4.createHeaderBank for
    // the fields downstream cares about; trigger/timestamp/torus/solenoid/type/
    // mode are left at 0 as for these standalone runs.
    hipo::bank cfg(fSchCFG, 1);
    cfg.putInt(iC_run, 0, run);
    cfg.putInt(iC_event, 0, event);
    cfg.putInt(iC_unixtime, 0, unixtime);
    cfg.putLong(iC_trigger, 0, 0);
    cfg.putLong(iC_timestamp, 0, 0);
    cfg.putByte(iC_type, 0, 0);
    cfg.putByte(iC_mode, 0, 0);
    cfg.putFloat(iC_torus, 0, 0.f);
    cfg.putFloat(iC_solenoid, 0, 0.f);
    fEvent.addStructure(cfg);

    if (!adc.empty()) {
        hipo::bank b(fSchURW, static_cast<int>(adc.size()));
        for (int i = 0; i < static_cast<int>(adc.size()); ++i) {
            const AdcRow &r = adc[i];
            b.putByte(iU_sector, i, static_cast<int8_t>(r.sector));
            b.putByte(iU_layer, i, static_cast<int8_t>(r.layer));
            b.putShort(iU_comp, i, static_cast<int16_t>(r.component));
            b.putByte(iU_order, i, static_cast<int8_t>(r.order));
            b.putInt(iU_adc, i, r.adc);
            // time = 1000*(layer-1) + component  (CLASDecoder4.java:314)
            b.putFloat(iU_time, i, static_cast<float>(1000 * (r.layer - 1) + r.component));
            b.putShort(iU_ped, i, static_cast<int16_t>(r.ped));
        }
        fEvent.addStructure(b);
    }

    if (!tdc.empty()) {
        hipo::bank b(fSchHODO, static_cast<int>(tdc.size()));
        for (int i = 0; i < static_cast<int>(tdc.size()); ++i) {
            const TdcRow &r = tdc[i];
            b.putByte(iH_sector, i, static_cast<int8_t>(r.sector));
            b.putByte(iH_layer, i, static_cast<int8_t>(r.layer));
            b.putShort(iH_comp, i, static_cast<int16_t>(r.component));
            b.putByte(iH_order, i, static_cast<int8_t>(r.order));
            b.putInt(iH_tdc, i, r.tdc);
        }
        fEvent.addStructure(b);
    }

    fWriter.addEvent(fEvent);
}
