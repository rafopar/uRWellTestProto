//
// EVReader.h
//
// A small wrapper around the hipo::reader that provides random access to
// events of a skimmed file (Skim_PulseFit_<RUN>_<File>.hipo) and fills
// an EVEvent object with uRwell pulses and hodoscope TDC hits.
//

#ifndef EVREADER_H
#define EVREADER_H

#include <memory>
#include <string>

#include <reader.h>

#include "EVEvent.h"

class EVReader {
public:
    // Opens the file. Returns false if the file can not be opened or it
    // does not contain the uRwell::Pulse bank.
    bool Open(const char *filename);

    int GetEntries() const { return fEntries; }

    const std::string &GetFileName() const { return fFileName; }

    // Reads the event with the given 0-based index and fills "ev".
    // Returns false if the index is out of range.
    bool ReadEvent(int index, EVEvent &ev);

private:
    void Reopen();
    void FillEvent(EVEvent &ev, int index);

    std::string fFileName;
    std::unique_ptr<hipo::reader> fReader;
    hipo::dictionary fFactory;
    hipo::event fHipoEvent;
    std::unique_ptr<hipo::bank> fPulseBank;
    std::unique_ptr<hipo::bank> fHodoBank;
    std::unique_ptr<hipo::bank> fConfBank;
    int fEntries = 0;
};

#endif /* EVREADER_H */
