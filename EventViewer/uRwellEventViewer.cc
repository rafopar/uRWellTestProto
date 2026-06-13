//
// uRwellEventViewer.cc
//
// An event viewer for the uRwell test prototype. It reads a skimmed hipo
// file (with uRwell::Pulse and XYHODO::tdc banks) and shows the pulses,
// hits and clusters of individual events.
//
// Usage:
//      ./uRwellEventViewer.exe <file.hipo>
// e.g.
//      ./uRwellEventViewer.exe Skims/Skim_PulseFit_3208_403.hipo
//

#include <iostream>
#include <string>

#include <TApplication.h>

#include "EVMainFrame.h"

using namespace std;

int main(int argc, char *argv[]) {

    if (argc < 2) {
        cerr << "Usage: ./uRwellEventViewer.exe <file.hipo>" << endl;
        cerr << "e.g.   ./uRwellEventViewer.exe Skims/Skim_PulseFit_3208_403.hipo" << endl;
        return 1;
    }

    // TApplication mangles argv, so the file name is copied beforehand
    string filename = argv[1];

    TApplication app("uRwellEventViewer", &argc, argv);

    try {
        new EVMainFrame(gClient->GetRoot(), filename.c_str());
    } catch (const exception &e) {
        cerr << e.what() << endl;
        return 1;
    }

    app.Run();
    return 0;
}
