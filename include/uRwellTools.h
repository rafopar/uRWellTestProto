/*
 * File:   uRwellTools.h
 * Author: rafopar
 *
 * Created on May 7, 2023, 9:20 PM
 */

#ifndef URWELLTOOLS_H
#define URWELLTOOLS_H
#include <vector>
#include <cmath>
#include <TF1.h>
#include <TH2D.h>
#include <TCanvas.h>

namespace uRwellTools {

    // There are 11 distinct geometric regions with different U over V width ratios.
    const int nGroups = 11;

    const std::map<int, double> m_UStripWidth{
        {0, 350},
        {1, 262},
        {2, 262},
        {3, 262},
        {4, 262},
        {5, 350},
        {6, 350},
        {7, 350},
        {8, 175},
        {9, 175},
        {10, 175}};
    const std::map<int, double> m_VStripWidth{
        {0, 500},
        {1, 650},
        {2, 500},
        {3, 650},
        {4, 355},
        {5, 500},
        {6, 650},
        {7, 355},
        {8, 500},
        {9, 650},
        {10, 355}};


    /*
     *  The Key of the map is the uRwell slot
     */
    const std::map<int, double> m_StripWidth{
        {0, 350},  // U strip
        {1, 262},  // U strip
        {2, 262},  // U strip
        {3, 355},  // V strip
        {4, 350},  // U strip
        {5, 355},  // V strip
        {6, 650},  // V strip
        {7, 500},  // V strip
        {8, 500},  // V strip
        {9, 175},  // U strip
        {10, 650}, // V strip
        {11, 175}, // U strip
    };

    /*
     *  The Key of the map is the slot, and the value is the view 0=U, 1=V
     */
    const std::map<int, int> m_uRwellView{
        {0, 0},   // U
        {1, 0},   // U
        {2, 0},   // U
        {3, 1},   // V
        {4, 0},   // U
        {5, 1},   // V
        {6, 1},   // V
        {7, 1},   // V
        {8, 1},   // V
        {9, 0},   // U
        {10, 1},  // V
        {11, 0}   // U
    };
    
    // ======= Boundaries of groups of strips with the given strip width =======
    const std::vector<double> gr_UBounderies = {0.5, 64.5, 320.5, 448.5, 704.5};
    const std::vector<double> gr_VBounderies = {0.5, 64.5, 320.5, 448.5, 704.5};

    /*
     * This function returns the strip length in [mm] of the given channel.
     * The argument ranges from 1 to 704.
     * The function is the same for both U and V strips, so there is no need to
     * specify the View.
     */
    double getStripLength(int ch);
    
    /*
     *  This method returns the area = strip_Length * strip_width
     *  * ch = channel number
     *  * view is the strip view, 0 = U, 1 = V
     */
    double getStripArea(int ch, int view);

    class uRwellException : public std::exception {
    private:
        char * message;

    public:

        uRwellException(char * msg) : message(msg) {
        }

        char * what() {
            return message;
        }
    };

    struct uRwellHit {
        int sector;
        int layer;
        int strip;
        int stripLocal;
        double adc;
        double adcRel;
        int ts;
        int slot;
    };

    /*
     * This object contains different uRwell related efficiencies, and associated uncertainties
     */
    struct uRwellEff {
        double eff_U; // Efficiency of at least one U cluster
        double eff_V; // Efficiency of at least one V cluster
        double eff_OR; // Efficiency of any cluster
        double eff_AND; // Efficiency of U AND V cluster to be present together
        double eff_crs_BgrSubtr; // Efficiency of background sutracted UxV cross
        double errUp_eff_U; // Upper Error on eff_U
        double errUp_eff_V; // Upper Error on eff_V
        double errUp_eff_OR; // Upper Error on eff_OR
        double errUp_eff_AND; // Upper Error on eff_AND
        double errUp_eff_crs_BgrSubtr; // Upper Error on eff_crs_BgrSubtr
        double errLow_eff_U; // Lower Error on eff_U
        double errLow_eff_V; // Lower Error on eff_V
        double errLow_eff_OR; // Lower Error on eff_OR
        double errLow_eff_AND; // Lower Error on eff_AND
        double errLow_eff_crs_BgrSubtr; // Lower Error on eff_crs_BgrSubtr
    };

    /*
     * This object contains some summary information about the ADC distribution that is fitted with a Landau function.
     */
    struct ADC_Distribution {
        double MPV;
        double errMPV;
        double Mean;
        double errMean;
    };

    const double OneSigma = 0.683;

    const int clStripGap = 2; // The Max length of the gap in between strips with a given cluster 
    int getURwellSlot(int ch);
    int getGEMSlot(int ch);
    const int nSlot = 16; // The test Prototype has only 12 slots
    extern int slot_Offset[nSlot]; // Gives the 1st strip channel (unique channel) for the given slot

    const double Y_top_edge = 250;
    const double Y_bot_edge = -250;
    const double X_top_edge = 723;
    const double X_bot_edge = 506.14;

    const double strip_alpha = 10. * 0.017453293; // The strip angle in radians
    const double Y_0 = 250 + X_top_edge * tan(strip_alpha); // This is the Y coordinate of the 1st strip (U or V should be the same) when the X 0. In other words this is the offset of the 1st strip
    const double pitch = 1.; // mm


    const int nMaxVStrip = 704; // U Strip number can not be larger than this number
    const int nMaxUStrip = 704; // V Strip number can not be larger than this number
    const int nMaxUniqueChan = 1704; // When using unique channel, this variable can go max to this value

    const int n_grU = 4;
    const int n_grV = 4;

    const int gr_min[n_grU] = {0, 1, 5, 8};
    const int gr_Vmin[n_grU] = {1, 0, 1, 1};

    const double uRWell_Y_max = 250.; // mm
    const double uRWell_Y_min = -250.; // mm
    const double uRwell_XTop = 728.;
    const double uRwell_XBot = 510.;
    
    const int sec_TestProto = 6;
    const int layer_U_TestProto = 1;
    const int layer_V_TestProto = 2;

    /*
     * This function takes the h_in histogram, which is intended to be the ADC distributions that supposed to look
     * like a Landau distribution.
     * It will fit this distribution with a Landau function, then return "ADC_Distribution" object which will contain
     * MPV and mean values of the distributions.
     */
    ADC_Distribution CalcMPVandMean(TH1D*);

    /*
     * This function takes the h_in histogram as a first argument, and the 2nd argument is and addresses of uRwellEff 
     * object. It will calculates these efficiencies and write to corresponding addresses.
     * the "h_in" histogram is just the "number of V vs number of U" clusters.
     */
    void CalcEfficiencies(TH2* h_in, uRwellEff &eff);

    /**
     * This function takes the h_in histogram as an input. It assumes the h_in histogram
     * is the cross "Y vs X" histogram. It projects it on "X" axis then fits with a Gaus + Pol4 function,
     * and calculates the number corresponding to the Gaussian.
     */
    double getNofBgrSbtrCrosses(TH2* h_in);

    /*
     * This function take cross x and y coordiantes, and checks whether the cross is inside the detector.
     * This assumes idealized geometry, and checks are based on variables uRWell_Y_max, uRWell_Y_min, uRwell_XTop, uRwell_XBot
     */
    bool IsInsideDetector(double x, double y);


    /**
     *       _______________________________
     *       \                             /
     *        \                           /
     *         \                         /
     *          \                       /
     *           \                     /
     *            ---------------------
     */

    void DrawGroupStripBiundaries();

    void DrawActiveArea();
    
    class uRwellCluster {
    public:
        uRwellCluster();

        void setEnergy(double aEnergy) {
            fEnergy = aEnergy;
        }

        void setHits(std::vector<uRwellHit>);

        std::vector<uRwellHit>* getHits() {
            return &fv_Hits;
        }

        void setNStrips(int aNStrips) {
            fnStrips = fv_Hits.size();
        }

        const double getEnergy() {
            return fEnergy;
        }

        const double getPeakADC() {
            return fPeakADC;
        }

        const double getAvgStrip() {
            return fAvgStrip;
        }

        const double getPeakTime() {
            return fPeakTime;
        }

        // This method will be called when all hits of the cluster are identifed, and set into the cluster
        // I till find the Peak energy, the weighted avg strip etc.
        void FinalizeCluster();

    private:
        int fnStrips;
        double fEnergy;
        double fPeakADC;
        double fAvgStrip;
        double fPeakTime; // This is the peak time of the highest ADC strip
        std::vector<uRwellHit> fv_Hits;

        void findPeakTime();
        void findPeakEnergy();
        void findAvgStrip();

    };

    class uRwellCross {
    public:
        uRwellCross();
        uRwellCross(double stripU, double stripV); // stripU and stripV are cluster centers in units of strip numbers, they don't have to be integer

        const double getStripU() {
            return fStripU;
        }

        const double getStripV() {
            return fStripV;
        }

        const double getX() {
            return fCrossX;
        }

        const double getY() {
            return fCrossY;
        }

        const int getGroupU() {
            return fgrU;
        }

        const int getGroupV() {
            return fgrV;
        }

        const int getGroupID() {
            return fGroupID;
        }

        const int getSlotU() {
            return fSlotU;
        }

        const int getSlotV() {
            return fSlotV;
        }

        const void PrintCross();

    private:
        double fStripU; // Cross U coordinated, in units of strip
        double fStripV; // Cross V coordinated, in units of strip

        double fCrossX; // Cross X coordinate
        double fCrossY; // Cross Y coordinate

        int fgrU; // ID of the U group of strips: {(0: 1, 64 ), (1: 65, 320 ), (2: 321, 448 ), (3: 449, 704 ) }
        int fgrV; // ID of the V group of strips: {(0: 1, 64 ), (1: 65, 320 ), (2: 321, 448 ), (3: 449, 704 ) }
        int fGroupID; // ID od the UxV strip groups where the cross falls
        int fSlotU; // The slot of the U cluster Center of the cross
        int fSlotV; // The slot of the V cluster Center of the cross
    };

    std::vector<uRwellCluster> getGlusters(std::vector<uRwellHit>);
    double getCrossX(double strip_U, double strip_V); // returns Cross UxV cross X coordinate
    double getCrossY(double strip_U, double strip_V); // returns Cross UxV cross Y coordinate
    uRwellCluster getMaxAdcCluster(std::vector<uRwellCluster> &, int);

    // Not specific for uRwell, later maybe I will have another more global name
    // for such type of functions
    bool fileExists(const char* filename);


}

#endif /* URWELLTOOLS_H */