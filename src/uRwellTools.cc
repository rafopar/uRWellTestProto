#include <iostream>
#include <algorithm>
#include <uRwellTools.h>

#include <TLine.h>
#include <TCanvas.h>
#include <TEfficiency.h>

using namespace std;

int uRwellTools::getGEMSlot(int ch) {
    if (ch < 1 || ch > 1256) {
        cout << "This should not happen: GEM channel is " << ch << endl;
        cout << "Exiting" << endl;
        exit(1);
    }

    if (ch <= 128) {
        return 12;
    } else if (ch >= 129 && ch <= 256) {
        return 13;
    } else if (ch >= 1001 && ch <= 1128) {
        return 14;
    } else if (ch >= 1129 && ch <= 1256) {
        return 15;
    } else {
        cout << "This should not happen: GEM channel is " << ch << endl;
        cout << "Exiting" << endl;
        exit(1);
    }
}

int uRwellTools::getURwellSlot(int ch) {
    if (ch <= 64) {
        return 0;
    } else if (ch >= 65 && ch <= 192) {
        return 1;
    } else if (ch >= 193 && ch <= 320) {
        return 2;
    } else if (ch >= 321 && ch <= 448) {
        return 4;
    } else if (ch >= 449 && ch <= 576) {
        return 9;
    } else if (ch >= 577 && ch <= 704) {
        return 11;
    } else if (ch >= 1001 && ch <= 1064) {
        return 6;
    } else if (ch >= 1065 && ch <= 1192) {
        return 7;
    } else if (ch >= 1193 && ch <= 1320) {
        return 8;
    } else if (ch >= 1321 && ch <= 1448) {
        return 10;
    } else if (ch >= 1449 && ch <= 1576) {
        return 3;
    } else if (ch >= 1577 && ch <= 1704) {
        return 5;
    } else {
        return -1; // Should not happen, non existing slot
    }
}

double uRwellTools::getStripLength(int ch) {
    if (ch < 1 || ch > nMaxUStrip) {
        cout << "Channel " << ch << " is outside of the range from 1 to " << nMaxUStrip << endl;
        cout << "Exiting." << endl;
        exit(1);
    }

    /*
     *  see https://jeffersonlab-my.sharepoint.com/:x:/g/personal/rafopar_jlab_org/EXSTTdqkWClJl9J0J-0IayUBFrx2ZroRYswHxBfEYwc3fg?e=MHDlDT
     */

    if (ch <= 253) {
        return -0.144385 + ch * 5.41734;
    } else if (ch <= 529) {
        return 1609.32 - ch * 0.934112;
    } else {
        return 4476.99 - ch * 6.35164;
    }
}

double uRwellTools::getStripArea(int ch, int view) {
    double l = uRwellTools::getStripLength(ch);
    int slot = uRwellTools::getURwellSlot(1000 * view + ch);
    double width = uRwellTools::m_StripWidth.at(slot) / 1000.; // 1./1000 is for converting to mm

    return l * width;
}

int uRwellTools::slot_Offset[uRwellTools::nSlot] = {
    0, 64, 192, 1448, 320, 1576, 1000, 1064, 1192, 448, 1320, 576, 0, 128, 1000, 1128
};

uRwellTools::ADC_Distribution uRwellTools::CalcMPVandMean(TH1D *h_in) {
    TF1 *f_Landau = new TF1("f_Landau", "[0]*TMath::Landau(x, [1], [2])", 0., 1000.);
    f_Landau->SetNpx(4500);

    f_Landau->SetParameters(7. * h_in->GetMaximum(), h_in->GetBinCenter(h_in->GetMaximumBin()), 5);
    h_in->Fit(f_Landau, "MeV", "", 0., 500);

    ADC_Distribution distr;

    distr.MPV = f_Landau->GetParameter(1);
    distr.errMPV = f_Landau->GetParError(1);
    distr.Mean = h_in->GetMean();
    distr.errMean = h_in->GetMeanError();

    delete f_Landau;

    return distr;
}

void uRwellTools::CalcEfficiencies(TH2 *h_in, uRwellTools::uRwellEff &eff) {
    int binx1 = h_in->GetXaxis()->FindBin(1);
    int binx2 = h_in->GetNbinsX() + 1;
    int biny1 = h_in->GetYaxis()->FindBin(1);
    int biny2 = h_in->GetNbinsY() + 1;
    double counts_integral = h_in->Integral(0, binx2, 0, biny2);
    double counts_has_U_Cluster = h_in->Integral(binx1, binx2, 1, biny2);
    double counts_has_V_Cluster = h_in->Integral(1, binx2, biny1, biny2);
    double counts_has_AnyCluster = counts_integral - h_in->GetBinContent(1, 1);
    double counts_has_U_AND_V_Cluster = h_in->Integral(binx1, binx2, biny1, biny2);

    eff.eff_U = 100. * counts_has_U_Cluster / counts_integral;
    eff.eff_V = 100. * counts_has_V_Cluster / counts_integral;
    eff.eff_OR = 100. * counts_has_AnyCluster / counts_integral;
    eff.eff_AND = 100. * counts_has_U_AND_V_Cluster / counts_integral;

    eff.errUp_eff_U = 100 * TEfficiency::ClopperPearson(counts_integral, counts_has_U_Cluster, OneSigma, true) - eff.
                      eff_U;
    eff.errLow_eff_U = eff.eff_U - 100 * TEfficiency::ClopperPearson(counts_integral, counts_has_U_Cluster, OneSigma,
                                                                     false);
    eff.errUp_eff_V = 100 * TEfficiency::ClopperPearson(counts_integral, counts_has_V_Cluster, OneSigma, true) - eff.
                      eff_V;
    eff.errLow_eff_V = eff.eff_V - 100 * TEfficiency::ClopperPearson(counts_integral, counts_has_V_Cluster, OneSigma,
                                                                     false);
    eff.errUp_eff_OR = 100 * TEfficiency::ClopperPearson(counts_integral, counts_has_AnyCluster, OneSigma, true) - eff.
                       eff_OR;
    eff.errLow_eff_OR = eff.eff_OR - 100 * TEfficiency::ClopperPearson(counts_integral, counts_has_AnyCluster, OneSigma,
                                                                       false);
    eff.errUp_eff_AND = 100 * TEfficiency::ClopperPearson(counts_integral, counts_has_U_AND_V_Cluster, OneSigma, true) -
                        eff.eff_AND;
    eff.errLow_eff_AND = eff.eff_AND - 100 * TEfficiency::ClopperPearson(
                             counts_integral, counts_has_U_AND_V_Cluster, OneSigma, false);

    cout << eff.errLow_eff_U << " Kuku   " << eff.errUp_eff_U << "  " << counts_integral << "   " <<
            counts_has_U_Cluster << endl;
}

double uRwellTools::getNofBgrSbtrCrosses(TH2 *h_in) {
    TF1 *f_Gaus = new TF1("f_Gaus", "[0]*TMath::Gaus(x, [1], [2])", -900., 900.);
    f_Gaus->SetNpx(4500);
    TF1 *f_GPol4 = new TF1("f_GPol4", "[0]*TMath::Gaus(x, [1], [2]) + [3] + x*( [4] + x*( [5] + x*( [6] + x*[7] ) )  )",
                           -900., 900.);
    f_GPol4->SetNpx(4500);
    TF1 *f_Pol4 = new TF1("f_Pol4", "[0] + x*( [1] + x*( [2] + x*( [3] + x*[4] ) )  )", -900., 900.);
    f_Pol4->SetNpx(4500);
    f_Pol4->SetLineColor(4);

    TH1D *h_Cross_X2 = (TH1D *) h_in->ProjectionX("h_Cross_X2", 1, h_in->GetNbinsY());
    h_Cross_X2->Draw();
    f_GPol4->SetParameters(h_Cross_X2->GetMaximum(), h_Cross_X2->GetBinCenter(h_Cross_X2->GetMaximumBin()), 75.);
    h_Cross_X2->Fit(f_GPol4, "MeV", "", -800, 800.);
    double pars[8];
    f_GPol4->GetParameters(pars);
    f_Gaus->SetParameters(pars);
    f_Pol4->SetParameters(&pars[3]);
    f_Pol4->Draw("Same");
    double N_Cross_BgrSubtr = f_Gaus->Integral(-800., 800.) / h_Cross_X2->GetBinWidth(15);

    return N_Cross_BgrSubtr;
}

bool uRwellTools::IsInsideDetector(double x, double y) {
    return y > uRWell_Y_min && y < uRWell_Y_max &&
           y > (uRWell_Y_min + (x - uRwell_XBot) * (uRWell_Y_max - uRWell_Y_min) / (uRwell_XTop - uRwell_XBot)) &&
           y > (uRWell_Y_min + (x + uRwell_XBot) * (uRWell_Y_max - uRWell_Y_min) / (uRwell_XBot - uRwell_XTop));
}

void uRwellTools::DrawGroupStripBiundaries() {
    const int n_UBounderies = 3;
    const int n_VBounderies = 3;

    double U_bounderies[n_UBounderies] = {64.5, 320.5, 448.5};
    double V_bounderies[n_VBounderies] = {64.5, 320.5, 448.5};

    // Lets define the line for the left and right side, i.e. "a" and "b" of y = ax+b
    double a_left = (Y_top_edge - Y_bot_edge) / (X_bot_edge - X_top_edge);
    double b_left = Y_bot_edge + a_left * X_bot_edge;

    double a_right = (Y_top_edge - Y_bot_edge) / (X_top_edge - X_bot_edge);
    double b_right = Y_bot_edge - a_right * X_bot_edge;

    TLine *line1 = new TLine();

    // ============ Let's draw U lines =============
    line1->SetLineColor(2);
    for (int i = 0; i < n_UBounderies; i++) {
        // x1 and y1 are the intersection of the strip and the left side of the uRwell
        double x1 = (b_left - Y_0 + (U_bounderies[i] * pitch) / cos(strip_alpha)) / (tan(strip_alpha) - a_left);
        double y1 = a_left * x1 + b_left;

        // x2 and y2 are the intersection coordinates of the strip and either the Top base or the right side, whichever intersects first

        // First we check the crossing with the top base
        double x2 = (Y_top_edge - Y_0 + (U_bounderies[i] * pitch)) / tan(strip_alpha);
        double y2 = Y_top_edge;

        if (x2 > X_top_edge) {
            //  This means the strip doesn't reach the top boundary, but crosses the right side.
            x2 = (b_right - Y_0 + (U_bounderies[i] * pitch) / cos(strip_alpha)) / (tan(strip_alpha) - a_right);
            y2 = a_right * x2 + b_right;
        }
        line1->DrawLine(x1, y1, x2, y2);
    }


    // ============ Let's draw V lines =============
    line1->SetLineColor(4);
    for (int i = 0; i < n_VBounderies; i++) {
        // x1 and y1 are the intersection of the strip and the left side of the uRwell
        double x1 = (b_right - Y_0 + (V_bounderies[i] * pitch) / cos(-strip_alpha)) / (tan(-strip_alpha) - a_right);
        double y1 = a_right * x1 + b_right;

        // x2 and y2 are the intersection coordinates of the strip and either the Top base or the right side, whichever intersects first

        // First we check the crossing with the top base
        double x2 = (Y_top_edge - Y_0 + (V_bounderies[i] * pitch)) / tan(-strip_alpha);
        double y2 = Y_top_edge;

        if (x2 < -X_top_edge) {
            //  This means the strip doesn't reach the top boundary, but crosses the right side.
            x2 = (b_left - Y_0 + (V_bounderies[i] * pitch) / cos(-strip_alpha)) / (tan(-strip_alpha) - a_left);
            y2 = a_left * x2 + b_left;
        }
        line1->DrawLine(x1, y1, x2, y2);
    }
}

void uRwellTools::DrawActiveArea() {
    TLine *line1 = new TLine();
    line1->SetLineColor(2);
    line1->SetLineWidth(3);
    line1->DrawLine(-723, 250., 723., 250.);
    line1->DrawLine(-506.14, -250., 506.14, -250.);
    line1->DrawLine(-506.14, -250., -723, 250.);
    line1->DrawLine(506.14, -250., 723, 250.);
}

namespace uRwellTools {
    uRwellCluster getMaxAdcCluster(std::vector<uRwellCluster> &v_clusters, int MinHits) {
        int ind_Max = 0;
        double ADC_Max = 0;

        for (int ind = 0; ind < v_clusters.size(); ind++) {
            if (v_clusters.at(ind).getHits()->size() < MinHits) {
                continue;
            }

            if (v_clusters.at(ind).getPeakADC() > ADC_Max) {
                ADC_Max = v_clusters.at(ind).getPeakADC();
                ind_Max = ind;
            }
        }

        // Check if there is such a cluster, otherwise make a new cluster with 0
        // energy that easily can be checked
        if (ADC_Max > 0) {
            return v_clusters.at(ind_Max);
        } else {
            uRwellCluster tmp;
            tmp.setEnergy(0);
            return tmp;
        }
    }

    PulseCluster getMaxIntegralPulseCluster(std::vector<PulseCluster> &v_Pulseclusters, int MinHits) {
        int ind_Max = 0;
        double Integral_Max = 0;

        for (int ind = 0; ind < v_Pulseclusters.size(); ind++) {
            if (v_Pulseclusters.at(ind).getPulses()->size() < MinHits) {
                continue;
            }

            if (v_Pulseclusters.at(ind).getClusterPulseIntegral() > Integral_Max ) {
                Integral_Max = v_Pulseclusters.at(ind).getClusterPulseIntegral();
                ind_Max = ind;
            }
        }

        if (Integral_Max > 0) {
            return v_Pulseclusters.at(ind_Max);
        }else {
            PulseCluster tmp;
            return tmp;
        }
    }


    uRwellCluster::uRwellCluster() {
        fnStrips = 0;
        fEnergy = 0;
    }

    void uRwellCluster::setHits(std::vector<uRwellHit> aHits) {
        fv_Hits = aHits;
    }

    void uRwellCluster::findPeakEnergy() {
        double MaxADC = 0;

        for (auto curHit: fv_Hits) {
            MaxADC = MaxADC > curHit.adc ? MaxADC : curHit.adc;
        }

        fPeakADC = MaxADC;
    }

    void uRwellCluster::findAvgStrip() {
        double WeightedSum = 0;
        double ADCSum = 0;

        for (auto curHit: fv_Hits) {
            WeightedSum = WeightedSum + curHit.adc * curHit.strip;
            ADCSum = ADCSum + curHit.adc;
        }

        fAvgStrip = WeightedSum / ADCSum;
    }

    void uRwellCluster::findPeakTime() {
        int time = -1;

        double MaxADC = 0;

        for (auto curHit: fv_Hits) {
            if (curHit.adc > MaxADC) {
                MaxADC = curHit.adc;
                time = curHit.ts;
            }
        }
        fPeakTime = time;
    }

    void uRwellCluster::FinalizeCluster() {
        double MaxADC = 0;
        double WeightedSum = 0;
        double ADCSum = 0;
        int time = -1;

        for (auto curHit: fv_Hits) {
            if (curHit.adc > MaxADC) {
                MaxADC = curHit.adc;
                time = curHit.ts;
            }

            WeightedSum = WeightedSum + curHit.adc * curHit.strip;
            ADCSum = ADCSum + curHit.adc;
        }

        fPeakADC = MaxADC; // ADC of the strip w/ highest ADC
        fAvgStrip = WeightedSum / ADCSum; // The weighted average strip number of the cluster
        fPeakTime = time; // Time of the highest energy strip
    }

    PulseCluster::PulseCluster() : fnStrips(0), fSeedMPV(0), fClusterMPV(0), fSeedSigma(0), fClusterSigma(), fSeedPulseIntegral(), fClusterPulseIntegral{} {

    }

    void PulseCluster::setPulses( std::vector<APV25Pulse> aPulses ) {
        fv_Pulses = std::move(aPulses);

        // New pulses are set for this cluster, so we will perform the FinalizeCluster() on it
        FinalizeCluster();
    }

    void PulseCluster::FinalizeCluster() {
        fnStrips = 0;
        fSeedMPV = -10000;
        fClusterMPV = -10000;
        fSeedSigma = -10000;
        fSeedStrip = -10000;
        fClusterSigma = -10000;
        fSeedPulseIntegral = -10000;
        fClusterPulseIntegral = -10000;
        fAvgStrip = -10000;
        fSeedPulse = {};

        if ( fv_Pulses.empty() ) {
            return;
        }
        fnStrips = fv_Pulses.size();
        fClusterPulseIntegral = 0;
        fClusterMPV = 0;
        fClusterSigma = 0;
        fAvgStrip = 0;

        for (auto curPulse : fv_Pulses) {
            fClusterPulseIntegral = fClusterPulseIntegral + curPulse.pulse_Integral;

            fClusterMPV = fClusterMPV + curPulse.pulse_Integral*curPulse.pulse_MPV;
            fClusterSigma = fClusterSigma + curPulse.pulse_Integral*curPulse.pulse_Sigma;
            fAvgStrip = fAvgStrip + curPulse.pulse_Integral*curPulse.hit.strip;

            if ( curPulse.pulse_Integral > fSeedPulseIntegral ) {
                fSeedPulseIntegral = curPulse.pulse_Integral;
                fSeedMPV = curPulse.pulse_MPV;
                fSeedSigma = curPulse.pulse_Sigma;
                fSeedStrip = curPulse.hit.strip;
                fSeedPulse = curPulse;
            }
        }

        fClusterMPV = fClusterMPV/fClusterPulseIntegral;
        fClusterSigma = fClusterSigma/fClusterPulseIntegral;
        fAvgStrip = fAvgStrip/fClusterPulseIntegral;
    }

    double PulseCluster::getSeedMPV() const {
        return fSeedMPV;
    }

    double PulseCluster::getClusterMPV() const {
        return fClusterMPV;
    }
    double PulseCluster::getSeedSigma() const {
        return fSeedSigma;
    }
    double PulseCluster::getClusterSigma() const {
        return fClusterSigma;
    }
    double PulseCluster::getSeedPulseIntegral() const {
        return fSeedPulseIntegral;
    }
    double PulseCluster::getClusterPulseIntegral() const {
        return fClusterPulseIntegral;
    }
    double PulseCluster::getClusterCenter() const {
        return fAvgStrip;
    }

    double PulseCluster::getSeedStrip() const {
        return fSeedStrip;
    }
    APV25Pulse PulseCluster::getSeedPulse() const {
        return fSeedPulse;
    }


    uRwellCross::uRwellCross() {
    }

    uRwellCross::uRwellCross(double stripU, double stripV) {
        try {
            // ========= Make sure strips are not out of range =========
            if (stripU < 1 || stripU > nMaxUStrip || stripV < 1 || stripV > nMaxVStrip) {
                cout << "Strip U is " << stripU << endl;
                cout << "Strip V is " << stripV << endl;

                throw ("One of strip is out of range");
            }

            fStripU = stripU;
            fStripV = stripV;

            fCrossX = getCrossX(fStripU, fStripV);
            fCrossY = getCrossY(fStripU, fStripV);

            fSlotU = getURwellSlot(int(fStripU));
            fSlotV = getURwellSlot(int(1000 + fStripV));
            // We add 1000, because the the getURwellSlot( ) function as an argument takes global strip number i, 1 to 1704

            fgrU = (std::lower_bound(gr_UBounderies.begin(), gr_UBounderies.end(), fStripU) - gr_UBounderies.begin()) -
                   1;
            fgrV = (std::lower_bound(gr_VBounderies.begin(), gr_VBounderies.end(), fStripV) - gr_VBounderies.begin()) -
                   1;

            fGroupID = IsInsideDetector(fCrossX, fCrossY) ? gr_min[fgrU] + fgrV - gr_Vmin[fgrU] : -1;
        } catch (uRwellException ex) {
            cout << "!! uRwell Exception !!" << endl;
            cout << ex.what();
        }
    }

    const void uRwellCross::PrintCross() {
        cout << " ============== CRoss ============= " << endl;
        cout << "*  Strip U: " << fStripU << "      Strip V: " << fStripV << endl;
        cout << "*  Cross (X, Y) = (" << fCrossX << "," << fCrossY << ")" << endl;
        cout << "*  Slot U    = " << fSlotU << endl;
        cout << "*  Slot V    = " << fSlotV << endl;
        cout << "*  group U   = " << fgrU << endl;
        cout << "*  group V   = " << fgrV << endl;
        cout << "*  group ID  = " << fGroupID << endl;
        cout << endl << endl;
    }

    std::vector<uRwellCluster> getGlusters(std::vector<uRwellHit> v_Hits) {
        //
        // Let's 1st sort the vector of strips
        //
        vector<uRwellCluster> v_Clusters;
        sort(v_Hits.begin(), v_Hits.end(), [ ](const auto &lhs, const auto &rhs) {
            return lhs.strip < rhs.strip;
        });


        double clEnergy = 0;
        bool newCluster = true;
        int prev_Strip = -10000; // Some number that clearly is not a real strip number
        //        vector<int> v_strips;
        vector<uRwellHit> v_ClHits;

        for (int i = 0; i < v_Hits.size(); i++) {
            int curStrip = v_Hits.at(i).strip;
            double curHitEnergy = v_Hits.at(i).adc;

            //cout << "v_Hits.at(i).strip = " << v_Hits.at(i).strip << "    curStrip =  " << curStrip << endl;

            /*
             * Since the v_Hits is sorted by Strip numbers, in the following if statement we
             * don't need to chack for the absolute value of the curStrip - prev_Strip
             */
            if ((curStrip - prev_Strip <= (clStripGap + 1)) || i == 0) {
                clEnergy = clEnergy + curHitEnergy;
                v_ClHits.push_back(v_Hits.at(i));

                if (i == v_Hits.size() - 1) {
                    uRwellTools::uRwellCluster curCluster;
                    curCluster.setEnergy(clEnergy);
                    curCluster.setHits(v_ClHits);
                    curCluster.FinalizeCluster();
                    v_Clusters.push_back(curCluster);
                }
            } else {
                uRwellTools::uRwellCluster curCluster;
                curCluster.setEnergy(clEnergy);
                curCluster.setHits(v_ClHits);
                curCluster.FinalizeCluster();
                v_Clusters.push_back(curCluster);

                v_ClHits.clear();
                v_ClHits.shrink_to_fit();

                clEnergy = curHitEnergy;
                v_ClHits.push_back(v_Hits.at(i));
            }
            prev_Strip = curStrip;
        }

        //cout<<"The size of the cluster is "<<v_Clusters.size()<<endl;
        return v_Clusters;
    }


    std::vector<PulseCluster> getPulseClusters(std::vector<APV25Pulse> v_Pulses) {
        vector<PulseCluster> v_PulseClusters;

        //
        // Sorting Pulses by the strip number
        //
        sort(v_Pulses.begin(), v_Pulses.end(), [ ](const auto &lhs, const auto &rhs) {
            return lhs.hit.strip < rhs.hit.strip;
        });

        int prev_Strip = -10000; // Some number that clearly is not a real strip number
        //        vector<int> v_strips;
        vector<APV25Pulse> v_ClPulses; // Vector of pulses of a cluster

        for (int i = 0; i < v_Pulses.size(); i++) {
            int curStrip = v_Pulses.at(i).hit.strip;

            /*
            * Since the v_Pulses is sorted by Strip numbers, in the following if statement we
            * don't need to chack for the absolute value of the curStrip - prev_Strip
            */

            if ((curStrip - prev_Strip <= (clStripGap + 1)) || i == 0) {
                v_ClPulses.push_back(v_Pulses.at(i));

                if (i == v_Pulses.size() - 1) {
                    PulseCluster curCluster;

                    curCluster.setPulses(v_ClPulses);
                    curCluster.FinalizeCluster();
                    v_PulseClusters.push_back(curCluster);

                }
            } else {
                PulseCluster curCluster;
                curCluster.setPulses(v_ClPulses);
                curCluster.FinalizeCluster();
                v_PulseClusters.push_back(curCluster);

                v_ClPulses.clear();
                v_ClPulses.shrink_to_fit();
                v_ClPulses.push_back(v_Pulses.at(i));
            }
            prev_Strip = curStrip;
        }

        return v_PulseClusters;
    }

    double getCrossX(double strip_U, double strip_V) {
        return pitch * (strip_U - strip_V) / (2 * sin(strip_alpha));
    }

    double getCrossY(double strip_U, double strip_V) {
        double crs_x = getCrossX(strip_U, strip_V);
        return tan(strip_alpha) * crs_x + Y_0 - (strip_U * pitch) / cos(strip_alpha);
    }


    double getROLength_U(double xx, double yy) {

        if ( !IsInsideDetector(xx, yy) ) {
            return -1;
        }

        // starting from this strip readout side for U(V) strips changes from left(right) to right(left) side.
        double stripFlip = 448.5;

        // Lets define the line for the left and right side, i.e. "a" and "b" of y = ax+b
        double a_left = (Y_top_edge - Y_bot_edge) / (X_bot_edge - X_top_edge);
        double b_left = Y_bot_edge + a_left * X_bot_edge;

        double a_right = (Y_top_edge - Y_bot_edge) / (X_top_edge - X_bot_edge);
        double b_right = Y_bot_edge - a_right * X_bot_edge;

        // x1 and y1 are the intersection of the left side of the uRwell and the splitting strip (i.e. strips above
        // are readout from left and strips below readout from right side)
        double x1 = (b_left - Y_0 + (stripFlip * pitch) / cos(strip_alpha)) / (tan(strip_alpha) - a_left);
        double y1 = a_left * x1 + b_left;

        double y_strEnd = 100000;
        double x_strEnd = 100000;

        bool leftSide = yy > y1 + (xx - x1)*tan(strip_alpha);
        if (leftSide) {
            x_strEnd = (yy - b_left - xx*tan(strip_alpha))/(a_left - tan(strip_alpha));
            y_strEnd = a_left*x_strEnd + b_left;
        }else {
            x_strEnd = (yy - b_right - xx*tan(strip_alpha))/(a_right - tan(strip_alpha));
            y_strEnd = a_right*x_strEnd + b_right;
        }
        return sqrt( (xx - x_strEnd)*(xx - x_strEnd) + (yy - y_strEnd)*(yy - y_strEnd) );
    }

     double getROLength_V(double xx, double yy) {

        if ( !IsInsideDetector(xx, yy) ) {
            return -1;
        }

        // starting from this strip readout side for U(V) strips changes from left(right) to right(left) side.
        double stripFlip = 448.5;

        // Lets define the line for the left and right side, i.e. "a" and "b" of y = ax+b
        double a_left = (Y_top_edge - Y_bot_edge) / (X_bot_edge - X_top_edge);
        double b_left = Y_bot_edge + a_left * X_bot_edge;

        double a_right = (Y_top_edge - Y_bot_edge) / (X_top_edge - X_bot_edge);
        double b_right = Y_bot_edge - a_right * X_bot_edge;

        // x1 and y1 are the intersection of the left side of the uRwell and the splitting strip (i.e. strips above
        // are readout from left and strips below readout from right side)
        // x1 and y1 are the intersection of the strip and the left side of the uRwell
        double x1 = (b_right - Y_0 + (stripFlip * pitch) / cos(-strip_alpha)) / (tan(-strip_alpha) - a_right);
        double y1 = a_right * x1 + b_right;

        double y_strEnd = 100000;
        double x_strEnd = 100000;

        bool rightSide = yy > y1 - (xx - x1)*tan(strip_alpha);
        if (rightSide) {
            x_strEnd = (yy - b_right + xx*tan(strip_alpha))/(a_right + tan(strip_alpha));
            y_strEnd = a_right*x_strEnd + b_right;
        }else {
            x_strEnd = (yy - b_left + xx*tan(strip_alpha))/(a_left + tan(strip_alpha));
            y_strEnd = a_left*x_strEnd + b_left;
        }
        return sqrt( (xx - x_strEnd)*(xx - x_strEnd) + (yy - y_strEnd)*(yy - y_strEnd) );
    }



    bool fileExists(const char *filename) {
        FILE *file = fopen(filename, "r");
        if (file) {
            fclose(file);
            return true;
        }
        return false;
    }
} // namespace uRwellTools
