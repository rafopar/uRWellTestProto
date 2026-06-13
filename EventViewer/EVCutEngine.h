//
// EVCutEngine.h
//
// Parses and evaluates user defined cut expressions.
//
// Two kinds of cuts are supported:
//  * Pulse cuts   - evaluated for every pulse; pulses failing the cut are
//                   discarded from the event. Available variables:
//                   sec, layer, strip, stripLocal, adc, adcRel, ts, slot,
//                   ped_rms, pulse_p0, pulse_A0, pulse_MPV, pulse_Sigma,
//                   pulse_Chi2, pulse_NDF, pulse_Integral
//  * Event cuts   - evaluated once per event (after the pulse cut was applied);
//                   used by the Next/Prev navigation to skip events.
//                   Available variables: n_U_pulses, n_V_pulses, n_pulses
//
// Variable names are case-insensitive. Arbitrary arithmetic is allowed, the
// expression is handed to TFormula, e.g.:
//      pulse_Sigma > 3.5 && pulse_Chi2/pulse_NDF < 40
//      n_U_pulses + n_V_pulses > 4
//

#ifndef EVCUTENGINE_H
#define EVCUTENGINE_H

#include <memory>
#include <string>
#include <vector>

#include <TFormula.h>

#include "EVEvent.h"

class EVCutEngine {
public:
    // Both setters accept an empty string, which disables the corresponding cut.
    // On a parse error they return false and fill "error".
    bool SetPulseCut(const std::string &expr, std::string &error);
    bool SetEventCut(const std::string &expr, std::string &error);

    bool HasPulseCut() const { return fPulseFormula != nullptr; }
    bool HasEventCut() const { return fEventFormula != nullptr; }

    bool PassPulse(const uRwellTools::APV25Pulse &pulse) const;
    bool PassEvent(const EVEvent &ev) const;

    // Returns the event with the pulse cut applied to the U and V pulse lists.
    EVEvent FilterEvent(const EVEvent &raw) const;

private:
    // Translates the user expression into a TFormula, replacing every
    // recognized variable name with x[i]. "slotVars" maps the formula
    // slot i back to the index of the variable in "varNames".
    static bool BuildFormula(const std::string &expr, const std::vector<std::string> &varNames,
                             const char *formulaName, std::unique_ptr<TFormula> &formula,
                             std::vector<int> &slotVars, std::string &error);

    std::unique_ptr<TFormula> fPulseFormula;
    std::unique_ptr<TFormula> fEventFormula;
    std::vector<int> fPulseSlotVars;
    std::vector<int> fEventSlotVars;
};

#endif /* EVCUTENGINE_H */
