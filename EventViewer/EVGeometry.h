//
// EVGeometry.h
//
// Helpers for drawing U and V strips with real detector coordinates.
// The strip geometry (angle, pitch, offset and the trapezoidal active area)
// is taken from the constants in uRwellTools.h.
//

#ifndef EVGEOMETRY_H
#define EVGEOMETRY_H

namespace EVGeo {

    // Returns the parameters of the line y = a*x + b of a U/V strip.
    // "strip" is the strip number (can be fractional, e.g. a cluster centroid).
    void GetUStripLine(double strip, double &a, double &b);
    void GetVStripLine(double strip, double &a, double &b);

    // Clips the line y = a*x + b to the trapezoidal uRwell active area.
    // Returns false if the line does not cross the active area.
    bool ClipLineToActiveArea(double a, double b, double &x1, double &y1, double &x2, double &y2);
}

#endif /* EVGEOMETRY_H */
