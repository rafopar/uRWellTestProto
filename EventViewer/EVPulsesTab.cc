//
// EVPulsesTab.cc
//

#include "EVPulsesTab.h"

#include <algorithm>
#include <cmath>

#include <TCanvas.h>
#include <TF1.h>
#include <TGButton.h>
#include <TGLabel.h>
#include <TGraphErrors.h>
#include <TLatex.h>
#include <TLine.h>
#include <TRootEmbeddedCanvas.h>
#include <TString.h>
#include <WidgetMessageTypes.h>

#include <uRwellTools.h>

EVPulsesTab::EVPulsesTab(const TGWindow *p) : EVTab(p) {

    auto *controlFrame = new TGHorizontalFrame(this);

    auto *btnPrev = new TGTextButton(controlFrame, "< Prev page", kBtnPrevPage);
    btnPrev->Associate(this);
    controlFrame->AddFrame(btnPrev, new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 5, 5, 2, 2));

    fPageLabel = new TGLabel(controlFrame, "Page 1 / 1");
    controlFrame->AddFrame(fPageLabel, new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 10, 10, 2, 2));

    auto *btnNext = new TGTextButton(controlFrame, "Next page >", kBtnNextPage);
    btnNext->Associate(this);
    controlFrame->AddFrame(btnNext, new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 5, 5, 2, 2));

    AddFrame(controlFrame, new TGLayoutHints(kLHintsExpandX, 2, 2, 2, 2));

    fCanvas = new TRootEmbeddedCanvas("PulsesCanvas", this, 800, 600);
    AddFrame(fCanvas, new TGLayoutHints(kLHintsExpandX | kLHintsExpandY, 2, 2, 2, 2));
}

void EVPulsesTab::SetEvent(const EVEvent *ev) {
    fCurrentPage = 0; // Every new event starts from the first page
    EVTab::SetEvent(ev);
}

Bool_t EVPulsesTab::ProcessMessage(Longptr_t msg, Longptr_t parm1, Longptr_t) {
    if (GET_MSG(msg) == kC_COMMAND && GET_SUBMSG(msg) == kCM_BUTTON) {
        if (parm1 == kBtnPrevPage) {
            fCurrentPage--;
            Redraw();
            return kTRUE;
        }
        if (parm1 == kBtnNextPage) {
            fCurrentPage++;
            Redraw();
            return kTRUE;
        }
    }
    return TGCompositeFrame::ProcessMessage(msg, parm1, 0);
}

void EVPulsesTab::Redraw() {

    TCanvas *c = fCanvas->GetCanvas();
    c->Clear();
    fDrawObjects.clear();

    // Collect the pulses: U pulses first, then V pulses, each sorted by strip
    std::vector<const uRwellTools::APV25Pulse *> pulses;
    if (fEvent != nullptr) {
        for (const auto &p : fEvent->v_U_Pulses) {
            pulses.push_back(&p);
        }
        for (const auto &p : fEvent->v_V_Pulses) {
            pulses.push_back(&p);
        }
    }

    const int nTotal = int(pulses.size());
    const int nPages = std::max(1, (nTotal + kPulsesPerPage - 1) / kPulsesPerPage);
    fCurrentPage = std::max(0, std::min(fCurrentPage, nPages - 1));
    fPageLabel->SetText(Form("Page %d / %d", fCurrentPage + 1, nPages));
    Layout();

    if (nTotal == 0) {
        auto lat = std::make_unique<TLatex>(0.5, 0.5, "No pulses in this event");
        lat->SetNDC();
        lat->SetTextAlign(22);
        lat->Draw();
        fDrawObjects.push_back(std::move(lat));
        c->Update();
        return;
    }

    const int firstPulse = fCurrentPage * kPulsesPerPage;
    const int nPlot = std::min(kPulsesPerPage, nTotal - firstPulse);

    int nCols = nPlot;
    int nRows = 1;
    if (nPlot > 5) {
        nCols = (nPlot + 1) / 2;
        nRows = 2;
    }
    c->Divide(nCols, nRows);

    for (int k = 0; k < nPlot; k++) {
        c->cd(k + 1);
        gPad->SetLeftMargin(0.14);
        gPad->SetRightMargin(0.01);
        const uRwellTools::APV25Pulse *p = pulses[firstPulse + k];
        const bool isU = (p->hit.layer == uRwellTools::layer_U_TestProto);
        const Color_t color = isU ? kRed : kBlue;

        auto gr = std::make_unique<TGraphErrors>();
        double yMin = 0.;
        double yMax = 0.;
        for (int ts = 0; ts < 15; ts++) {
            gr->SetPoint(ts, ts, p->pulse_ADC[ts]);
            gr->SetPointError(ts, 0., p->ped_rms);
            yMin = std::min(yMin, p->pulse_ADC[ts] - p->ped_rms);
            yMax = std::max(yMax, p->pulse_ADC[ts] + p->ped_rms);
        }
        yMax = std::max(yMax, p->ped_rms);

        double chi2ndf = p->pulse_NDF > 0 ? p->pulse_Chi2 / p->pulse_NDF : 0.;
        gr->SetTitle(Form("Strip %d, %s, #sigma = %1.2f, #chi^{2}/ndf = %1.2f; time sample; ADC",
                          p->hit.strip, isU ? "U" : "V", p->pulse_Sigma, chi2ndf));
        gr->SetMarkerStyle(20);
        gr->SetMarkerSize(0.7);
        gr->SetMarkerColor(color);
        gr->SetLineColor(color);
        gr->SetMinimum(yMin - 0.05 * (yMax - yMin));
        gr->SetMaximum(yMax + 0.1 * (yMax - yMin));
        gr->Draw("AP");
        gr->GetXaxis()->SetLimits(-1., 15.);

        auto func = std::make_unique<TF1>(Form("f_pulse_%d_%d", fCurrentPage, k),
                                          "[0] + [1]*TMath::Landau(x, [2], [3])", -1., 15.);
        func->SetParameters(p->pulse_p0, p->pulse_A0, p->pulse_MPV, p->pulse_Sigma);
        func->SetNpx(500);
        func->SetLineColor(color);
        func->Draw("same");

        auto pedLine = std::make_unique<TLine>(-1., p->ped_rms, 15., p->ped_rms);
        pedLine->SetLineStyle(2);
        pedLine->SetLineColor(kGray + 2);
        pedLine->Draw();

        fDrawObjects.push_back(std::move(gr));
        fDrawObjects.push_back(std::move(func));
        fDrawObjects.push_back(std::move(pedLine));
    }

    c->cd();
    c->Update();
}
