/* 
 * File:   DrawNose_Vs_StripCoorelations.cc
 * Author: rafopar
 *
 * Created on July 19, 2024, 5:18 PM
 */

#include <cstdlib>

#include <TH1D.h>
#include <TH2D.h>
#include <TFile.h>
#include <TGraph.h>
#include <TLegend.h>
#include <TCanvas.h>
#include <TMultiGraph.h>

#include "uRwellTools.h"

using namespace std;

/*
 * 
 */
int main() {

    int run = 2153;
    const int nSlot = 12;

    TCanvas *c1 = new TCanvas("c1", "", 1800, 950);
    c1->SetTopMargin(0.02);
    c1->SetRightMargin(0.06);

    TFile *file_in = new TFile(Form("CheckDecoding_%d_0.root", run), "Read");

    std::vector<TGraph*> v_gr_U_Noise_vs_strLength;
    std::vector<TGraph*> v_gr_U_Noise_vs_strArea;
    std::vector<TGraph*> v_gr_V_Noise_vs_strLength;
    std::vector<TGraph*> v_gr_V_Noise_vs_strArea;
    
    std::map<int, int> m_cols{
        {0, 1},
        {1, 2},
        {2, 3},
        {4, 4},
        {9, 95},
        {11, 6},
        {6, 1},
        {7, 2},
        {8, 3},
        {10, 4},
        {3, 95},
        {5, 6},};

    TGraph * gr_Noise_vs_StrLength[nSlot];
    TGraph * gr_Noise_vs_StrArea[nSlot];
    TLegend *leg_U_Noise_vs_StrLength = new TLegend(0.12, 0.7, 0.4, 0.96);
    leg_U_Noise_vs_StrLength->SetBorderSize(0);
    TLegend *leg_U_Noise_vs_StrArea = new TLegend(0.12, 0.7, 0.4, 0.96);
    leg_U_Noise_vs_StrArea->SetBorderSize(0);

    TLegend *leg_V_Noise_vs_StrLength = new TLegend(0.12, 0.7, 0.4, 0.96);
    leg_V_Noise_vs_StrLength->SetBorderSize(0);
    TLegend *leg_V_Noise_vs_StrArea = new TLegend(0.12, 0.7, 0.4, 0.96);
    leg_V_Noise_vs_StrArea->SetBorderSize(0);
            
    for (int i = 0; i < nSlot; i++) {
        int view = uRwellTools::m_uRwellView.at(i);

        gr_Noise_vs_StrLength[i] = new TGraph();
        gr_Noise_vs_StrLength[i]->SetMarkerColor(m_cols.at(i));
        gr_Noise_vs_StrLength[i]->SetMarkerStyle(20 + view);
        
        gr_Noise_vs_StrArea[i] = new TGraph();
        gr_Noise_vs_StrArea[i]->SetMarkerColor(m_cols.at(i));
        gr_Noise_vs_StrArea[i]->SetMarkerStyle(20 + view);

        if (view == 0) {
            v_gr_U_Noise_vs_strLength.push_back(gr_Noise_vs_StrLength[i]);
            v_gr_U_Noise_vs_strArea.push_back(gr_Noise_vs_StrArea[i]);
            leg_U_Noise_vs_StrLength->AddEntry( gr_Noise_vs_StrLength[i], Form("Slot %d", i) );
            leg_U_Noise_vs_StrArea->AddEntry( gr_Noise_vs_StrArea[i], Form("Slot %d", i) );
        } else if (view == 1) {
            v_gr_V_Noise_vs_strLength.push_back(gr_Noise_vs_StrLength[i]);
            v_gr_V_Noise_vs_strArea.push_back(gr_Noise_vs_StrArea[i]);
            leg_V_Noise_vs_StrLength->AddEntry( gr_Noise_vs_StrLength[i], Form("Slot %d", i) );
            leg_V_Noise_vs_StrArea->AddEntry( gr_Noise_vs_StrArea[i], Form("Slot %d", i) );
        }
    }

    TH2D *h_ADC_chan = (TH2D*) file_in->Get("h_ADC_chan");

    h_ADC_chan->SetTitle("; channel #; ADC");

    h_ADC_chan->Draw();

    for (int i = 0; i < h_ADC_chan->GetNbinsX(); i++) {
        TH1D *h_tmp = (TH1D*) h_ADC_chan->ProjectionY(Form("tmp_%d", i), i + 1, i + 1);

        if (h_tmp->GetEntries() < 10) {
            continue;
        }

        double rms = h_tmp->GetRMS();

        int ch = int(h_ADC_chan->GetXaxis()->GetBinCenter(i + 1));

        int view = ch / 1000;
        int ch_view = ch % 1000;
        int slot = uRwellTools::getURwellSlot(ch);

        double l = uRwellTools::getStripLength(ch_view);
        double area = uRwellTools::getStripArea(ch_view, view);
        
        //cout<<"ch = "<<ch<<"   l = "<<l<<"    width = "<<w<<"     area = "<<area<<endl;
        
        gr_Noise_vs_StrLength[slot]->AddPoint(l, rms);
        gr_Noise_vs_StrArea[slot]->AddPoint(area, rms);
    }


    TMultiGraph *mtgr_U_Noise_vs_StrLength = new TMultiGraph();
    mtgr_U_Noise_vs_StrLength->SetTitle("; U strip length [mm]; RMS of ADC");
    TMultiGraph *mtgr_U_Noise_vs_StrArea = new TMultiGraph();
    mtgr_U_Noise_vs_StrArea->SetTitle("; U strip area [mm^{2}]; RMS of ADC");
    for (auto gr : v_gr_U_Noise_vs_strLength) {
        mtgr_U_Noise_vs_StrLength->Add(gr);
    }

    mtgr_U_Noise_vs_StrLength->Draw("AP");
    leg_U_Noise_vs_StrLength->Draw();
    c1->Print(Form("Figs/U_Noise_vs_StrLength_%d.pdf", run));
    c1->Print(Form("Figs/U_Noise_vs_StrLength_%d.png", run));
    c1->Print(Form("Figs/U_Noise_vs_StrLength_%d.root", run));

    for (auto gr : v_gr_U_Noise_vs_strArea) {
        mtgr_U_Noise_vs_StrArea->Add(gr);
    }
    mtgr_U_Noise_vs_StrArea->Draw("AP");
    leg_U_Noise_vs_StrArea->Draw();
    c1->Print(Form("Figs/U_Noise_vs_StrArea_%d.pdf", run));
    c1->Print(Form("Figs/U_Noise_vs_StrArea_%d.png", run));
    c1->Print(Form("Figs/U_Noise_vs_StrArea_%d.root", run));
    
    
    TMultiGraph *mtgr_V_Noise_vs_StrLength = new TMultiGraph();
    mtgr_V_Noise_vs_StrLength->SetTitle("; V strip length [mm]; RMS of ADC");
    TMultiGraph *mtgr_V_Noise_vs_StrArea = new TMultiGraph();
    mtgr_V_Noise_vs_StrArea->SetTitle("; V strip area [mm^{2}]; RMS of ADC");
    for (auto gr : v_gr_V_Noise_vs_strLength) {
        mtgr_V_Noise_vs_StrLength->Add(gr);
    }

    mtgr_V_Noise_vs_StrLength->Draw("AP");
    leg_V_Noise_vs_StrLength->Draw();
    c1->Print(Form("Figs/V_Noise_vs_StrLength_%d.pdf", run));
    c1->Print(Form("Figs/V_Noise_vs_StrLength_%d.png", run));
    c1->Print(Form("Figs/V_Noise_vs_StrLength_%d.root", run));

    for (auto gr : v_gr_V_Noise_vs_strArea) {
        mtgr_V_Noise_vs_StrArea->Add(gr);
    }
    mtgr_V_Noise_vs_StrArea->Draw("AP");
    leg_V_Noise_vs_StrArea->Draw();
    c1->Print(Form("Figs/V_Noise_vs_StrArea_%d.pdf", run));
    c1->Print(Form("Figs/V_Noise_vs_StrArea_%d.png", run));
    c1->Print(Form("Figs/V_Noise_vs_StrArea_%d.root", run));
    
    
    
    file_in->Close();
    return 0;
}