//
// EVReaderBase.h
//
// Abstract interface for an event source of the viewer. Two implementations
// exist: EVReader reads a skimmed HIPO file (uRwell::Pulse + XYHODO::tdc), and
// EVEvioReader reads a raw EVIO file and decodes it on the fly (Decoder code).
// EVMainFrame holds the current source through this interface and picks the
// concrete reader by file extension.
//

#ifndef EVREADERBASE_H
#define EVREADERBASE_H

#include <string>

#include "EVEvent.h"

class EVReaderBase {
public:
    virtual ~EVReaderBase() = default;

    // Opens the file. Returns false on failure.
    virtual bool Open(const char *filename) = 0;

    virtual int GetEntries() const = 0;

    virtual const std::string &GetFileName() const = 0;

    // Reads the event with the given 0-based index and fills "ev".
    // Returns false if the index is out of range.
    virtual bool ReadEvent(int index, EVEvent &ev) = 0;

    // True for the EVIO source (enables the raw-ADC tab).
    virtual bool IsEvio() const = 0;
};

#endif /* EVREADERBASE_H */
