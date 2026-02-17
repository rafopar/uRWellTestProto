/* 
 * File:   CheckDecoding.cc
 * Author: rafopar
 *
 * Created on December 20, 2022, 3:11 PM
 */

#include <cstdlib>

#include <TH2D.h>
#include <TH1D.h>
#include <TFile.h>

// ===== Hipo headers =====
#include <reader.h>
#include <writer.h>
#include <dictionary.h>

#include <map>
#include <vector>
#include <utility>

using namespace std;

/*
 * 
 */
int main(int argc, char** argv) {

    char outputFile[256];
    char inputFile[256];
    cout << "test if florian can modify code " << endl;
    int run = 0;
    int fnum = -1;
    if (argc > 2) {
        run = atoi(argv[1]);
        fnum = atoi(argv[2]);
        //sprintf(inputFile, "%s", argv[1]);
        sprintf(inputFile, "Data/decoded_%d_%d.hipo", run, fnum);
        sprintf(outputFile, "outSkim.hipo");
    } else {
        std::cout << " *** please provide a file name..." << std::endl;
        exit(0);
    }

    hipo::reader reader;
    reader.open(inputFile);

    hipo::dictionary factory;

    reader.readDictionary(factory);

    factory.show();
    hipo::event event;
    int evCounter = 0;

    hipo::bank buRWellADC(factory.getSchema("URWELL::adc"));
    hipo::bank bRAWADc(factory.getSchema("RAW::adc"));
    hipo::bank bRunConf(factory.getSchema("RUN::config"));

    const int crateID_test3 = 40; // the fADC is on ROC 40
    const int slot_fADC = 3; // and the slot is 3
    const int n_ts = 15;

    TFile *file_out = new TFile(Form("CheckDecoding_%d_%d.root", run, fnum), "Recreate");
    TH2D *h_ADC_chan = new TH2D("h_ADC_chan", "", 1711, -0.5, 1710.5, 400, -400., 400.);
    TH2D *h_ADC_chan_GEM = new TH2D("h_ADC_chan_GEM", "", 1258, -0.5, 1257.5, 400, -400., 400.);
    TH2D *h_ADC_AllChan = new TH2D("h_ADC_AllChan", "", 1711, -0.5, 1710.5, 400, -600., 600.);
    TH2D * h_ADC_Chan_ts_[n_ts];

    TH2D *h_ch_Coorelation = new TH2D("h_ch_Coorelation", "", 17, -0.5, 16.5, 17, -0.5, 16.5);

    TH2D *h_ADC_TopBot1 = new TH2D("h_ADC_TopBot1", "", 200, 0., 500, 200, 0., 500);

    for (int i = 0; i < n_ts; i++) {
        h_ADC_Chan_ts_[i] = new TH2D(Form("h_ADC_Chan_ts_%d", i), "", 1711, -0.5, 1710.5, 400, -1500., 500.);
    }


    /**
     * Checking some decoding staff
     */
    const int n_testEv = 2;
    const int n_testCh = 3;

    try {

        while (reader.next() == true) {
            reader.read(event);

            evCounter = evCounter + 1;

            if (evCounter > 2000) {
                break;
            }
            if (evCounter % 500 == 0) {
                cout.flush() << "Processed " << evCounter << " events \r";
            }



            event.getStructure(buRWellADC);
            event.getStructure(bRAWADc);
            event.getStructure(bRunConf);
            int __bank_Sec_INDEX_ = buRWellADC.getSchema().getEntryOrder("sector");
            int __bank_Layer_INDEX_ = buRWellADC.getSchema().getEntryOrder("layer");
            int __bank_Component_INDEX_ = buRWellADC.getSchema().getEntryOrder("component");
            int __bank_Order_INDEX_ = buRWellADC.getSchema().getEntryOrder("order");
            int __bank_ADC_INDEX_ = buRWellADC.getSchema().getEntryOrder("ADC");
            int __bank_Time_INDEX_ = buRWellADC.getSchema().getEntryOrder("time");
            int __bank_Ped_INDEX_ = buRWellADC.getSchema().getEntryOrder("ped");

            int n_ADC = buRWellADC.getRows();
            int n_RunConf = bRunConf.getRows();
            
            int event = bRunConf.getInt("event", 0);


            int n_fADC = bRAWADc.getRows();

            /**
             * Only 4 channels on fADC are read out 0,1 and 9,10 . 0 and 1 are from Bottom scintillator, while 9 and 10 are from Top
             */
            int ADC_SCTop = 0;
            int ADC_SCBot = 0;


            for (int ifADC = 0; ifADC < n_fADC; ifADC++) {

                int crate = bRAWADc.getInt("crate", ifADC);
                int slot = bRAWADc.getInt("slot", ifADC);

                if (!(crate == crateID_test3 && slot == slot_fADC)) {
                    cout << "Wrong crate or slot. Should not be happen" << endl;
                    cout << "              crate = " << crate << "     slot = " << slot << endl;

                    continue;
                }

                int ch = bRAWADc.getInt("channel", ifADC);
                int ADC = bRAWADc.getInt("ADC", ifADC);
                double time = bRAWADc.getFloat("time", ifADC);

                if (ch == 0 || ch == 1) {
                    ADC_SCBot = ADC_SCBot + ADC;
                } else if (ch == 9 || ch == 10) {
                    ADC_SCTop = ADC_SCTop + ADC;
                }

                for (int jfADC = ifADC + 1; jfADC < n_fADC; jfADC++) {

                    int jch = bRAWADc.getInt("channel", jfADC);
                    int jADC = bRAWADc.getInt("ADC", jfADC);

                    if (jADC > 0 && ADC > 0) {
                        h_ch_Coorelation->Fill(ch, jch);
                    }
                }

            }

            h_ADC_TopBot1->Fill(ADC_SCBot, ADC_SCTop);

            std::map< int, int > m_ADC_[n_ts]; // The key of the map is unique_channel, and the value is the ADC value
            std::map< int, int > m_ADC_GEM_[n_ts]; // The key of the map is unique_channel, and the value is the ADC value

            for (int i = 0; i < n_ADC; i++) {


                int sector = buRWellADC.getInt(__bank_Sec_INDEX_, i);
                int layer = buRWellADC.getInt(__bank_Layer_INDEX_, i);
                int channel = buRWellADC.getInt(__bank_Component_INDEX_, i);
                int ADC = buRWellADC.getInt(__bank_ADC_INDEX_, i);
                int uniqueChan = int(buRWellADC.getFloat(__bank_Time_INDEX_, i));
                int ts = buRWellADC.getInt(__bank_Ped_INDEX_, i);
                if (ts == n_ts) {
                    cout << "Kuku 3  "<<"  event "<<event<<"   sector" << sector << "   " << layer << "   " << channel << "   " << ADC << "   " << uniqueChan << "   " << ts << endl;
                    continue;
                }

                //                int sector = buRWellADC.getInt("sector", i);
                //                int layer = buRWellADC.getInt("layer", i);
                //                int channel = buRWellADC.getInt("component", i);
                //                int ADC = buRWellADC.getInt("ADC", i);
                //                int uniqueChan = int(buRWellADC.getFloat("time", i));
                //                int ts = buRWellADC.getInt("ped", i);

                int slot = layer;

                if (evCounter < 5 && layer < 3) {
                    cout << evCounter << "\t" << slot << "\t" << ts << "\t" << channel << "\t" << ADC << endl;
                }

                //if (sector == 85 && layer == 1) { // Non Sec 6 events are not physical
                if (sector == 6) { // Non Sec 6 events are not physical
                    h_ADC_Chan_ts_[ts]->Fill(uniqueChan, ADC);
                    h_ADC_AllChan->Fill(uniqueChan, ADC);
                    m_ADC_[ts][uniqueChan] = ADC;
                } else if (sector == 8) {
                    m_ADC_GEM_[ts][uniqueChan] = ADC; // For GEM we will use the uniquechannel as well
                }
            }

            for (auto map0 : m_ADC_[0]) {
                int ch = map0.first;
                //double avg_ADC = (m_ADC_[0][ch] + m_ADC_[1][ch] + m_ADC_[2][ch]) / 3.;
                double avg_ADC = 0.;
                for (int i_ts = 0; i_ts < n_ts; i_ts++) {
                    avg_ADC = avg_ADC + m_ADC_[i_ts][ch];
                }
                avg_ADC = avg_ADC / double(n_ts);
                h_ADC_chan->Fill(ch, avg_ADC);
            }

            for (auto map0 : m_ADC_GEM_[0]) {
                int ch = map0.first;
                double avg_ADC = 0.;
                for (int i_ts = 0; i_ts < n_ts; i_ts++) {
                    avg_ADC = avg_ADC + m_ADC_GEM_[i_ts][ch];
                }
                avg_ADC = avg_ADC / double(n_ts);
                h_ADC_chan_GEM->Fill(ch, avg_ADC);
            }

        }
    } catch (const char msg) {
        cerr << msg << endl;
    }

    gDirectory->Write();
    file_out->Close();
    return 0;
}
