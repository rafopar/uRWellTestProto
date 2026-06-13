//
// EVCutEngine.cc
//

#include "EVCutEngine.h"

#include <algorithm>
#include <regex>

namespace {

    // The order here defines the variable index used by GetPulseVar/GetEventVar.
    // Names must be lower case (the comparison is case-insensitive).
    const std::vector<std::string> kPulseVarNames = {
        "sec", "layer", "strip", "striplocal", "adc", "adcrel", "ts", "slot",
        "ped_rms", "pulse_p0", "pulse_a0", "pulse_mpv", "pulse_sigma",
        "pulse_chi2", "pulse_ndf", "pulse_integral"
    };

    const std::vector<std::string> kEventVarNames = {
        "n_u_pulses", "n_v_pulses", "n_pulses"
    };

    double GetPulseVar(const uRwellTools::APV25Pulse &p, int idx) {
        switch (idx) {
            case 0: return p.hit.sector;
            case 1: return p.hit.layer;
            case 2: return p.hit.strip;
            case 3: return p.hit.stripLocal;
            case 4: return p.hit.adc;
            case 5: return p.hit.adcRel;
            case 6: return p.hit.ts;
            case 7: return p.hit.slot;
            case 8: return p.ped_rms;
            case 9: return p.pulse_p0;
            case 10: return p.pulse_A0;
            case 11: return p.pulse_MPV;
            case 12: return p.pulse_Sigma;
            case 13: return p.pulse_Chi2;
            case 14: return p.pulse_NDF;
            case 15: return p.pulse_Integral;
            default: return 0.;
        }
    }

    double GetEventVar(const EVEvent &ev, int idx) {
        switch (idx) {
            case 0: return double(ev.v_U_Pulses.size());
            case 1: return double(ev.v_V_Pulses.size());
            case 2: return double(ev.nPulses());
            default: return 0.;
        }
    }

    std::string ToLower(const std::string &s) {
        std::string out = s;
        std::transform(out.begin(), out.end(), out.begin(), ::tolower);
        return out;
    }
}

bool EVCutEngine::BuildFormula(const std::string &expr, const std::vector<std::string> &varNames,
                               const char *formulaName, std::unique_ptr<TFormula> &formula,
                               std::vector<int> &slotVars, std::string &error) {
    formula.reset();
    slotVars.clear();
    error.clear();

    // Check that the expression is not only whitespace
    if (expr.find_first_not_of(" \t\n") == std::string::npos) {
        return true; // An empty expression disables the cut
    }

    /*
     * Scan the expression for identifiers. Every identifier that matches a known
     * variable name is replaced with x[i]. Unknown identifiers (e.g. function
     * names like abs, sqrt, TMath::Landau) are left untouched for TFormula.
     */
    std::string translated;
    std::vector<int> slots; // slot -> variable index
    std::regex identRegex("[A-Za-z_][A-Za-z0-9_]*");

    auto wordsBegin = std::sregex_iterator(expr.begin(), expr.end(), identRegex);
    auto wordsEnd = std::sregex_iterator();
    size_t lastPos = 0;

    for (auto it = wordsBegin; it != wordsEnd; ++it) {
        std::string token = ToLower(it->str());
        auto found = std::find(varNames.begin(), varNames.end(), token);
        translated += expr.substr(lastPos, it->position() - lastPos);
        if (found != varNames.end()) {
            int varIdx = int(found - varNames.begin());
            auto slotIt = std::find(slots.begin(), slots.end(), varIdx);
            int slot;
            if (slotIt == slots.end()) {
                slot = int(slots.size());
                slots.push_back(varIdx);
            } else {
                slot = int(slotIt - slots.begin());
            }
            translated += "x[" + std::to_string(slot) + "]";
        } else {
            translated += it->str();
        }
        lastPos = it->position() + it->length();
    }
    translated += expr.substr(lastPos);

    auto newFormula = std::make_unique<TFormula>(formulaName, translated.c_str(), false);
    if (!newFormula->IsValid()) {
        error = "Invalid expression: " + expr;
        return false;
    }

    formula = std::move(newFormula);
    slotVars = slots;
    return true;
}

bool EVCutEngine::SetPulseCut(const std::string &expr, std::string &error) {
    return BuildFormula(expr, kPulseVarNames, "ev_pulse_cut", fPulseFormula, fPulseSlotVars, error);
}

bool EVCutEngine::SetEventCut(const std::string &expr, std::string &error) {
    return BuildFormula(expr, kEventVarNames, "ev_event_cut", fEventFormula, fEventSlotVars, error);
}

bool EVCutEngine::PassPulse(const uRwellTools::APV25Pulse &pulse) const {
    if (fPulseFormula == nullptr) {
        return true;
    }
    double x[32] = {0};
    for (size_t i = 0; i < fPulseSlotVars.size(); i++) {
        x[i] = GetPulseVar(pulse, fPulseSlotVars[i]);
    }
    return fPulseFormula->EvalPar(x, nullptr) != 0.;
}

bool EVCutEngine::PassEvent(const EVEvent &ev) const {
    if (fEventFormula == nullptr) {
        return true;
    }
    double x[32] = {0};
    for (size_t i = 0; i < fEventSlotVars.size(); i++) {
        x[i] = GetEventVar(ev, fEventSlotVars[i]);
    }
    return fEventFormula->EvalPar(x, nullptr) != 0.;
}

EVEvent EVCutEngine::FilterEvent(const EVEvent &raw) const {
    if (fPulseFormula == nullptr) {
        return raw;
    }

    EVEvent filtered = raw;
    filtered.v_U_Pulses.clear();
    filtered.v_V_Pulses.clear();
    for (const auto &p : raw.v_U_Pulses) {
        if (PassPulse(p)) {
            filtered.v_U_Pulses.push_back(p);
        }
    }
    for (const auto &p : raw.v_V_Pulses) {
        if (PassPulse(p)) {
            filtered.v_V_Pulses.push_back(p);
        }
    }
    return filtered;
}
