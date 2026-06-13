//
// EVGeometry.cc
//

#include "EVGeometry.h"

#include <cmath>
#include <vector>

#include <uRwellTools.h>

namespace EVGeo {

    /*
     * The strip line equations follow uRwellTools::getCrossY:
     *   U strip: y =  tan(alpha)*x + Y_0 - strip*pitch/cos(alpha)
     *   V strip: y = -tan(alpha)*x + Y_0 - strip*pitch/cos(alpha)
     */
    void GetUStripLine(double strip, double &a, double &b) {
        a = tan(uRwellTools::strip_alpha);
        b = uRwellTools::Y_0 - (strip * uRwellTools::pitch) / cos(uRwellTools::strip_alpha);
    }

    void GetVStripLine(double strip, double &a, double &b) {
        a = -tan(uRwellTools::strip_alpha);
        b = uRwellTools::Y_0 - (strip * uRwellTools::pitch) / cos(uRwellTools::strip_alpha);
    }

    bool ClipLineToActiveArea(double a, double b, double &x1, double &y1, double &x2, double &y2) {
        // The trapezoid corners, in order (counter-clockwise)
        const double X_top = uRwellTools::X_top_edge; // 723
        const double X_bot = uRwellTools::X_bot_edge; // 506.14
        const double Y_top = uRwellTools::Y_top_edge; // 250
        const double Y_bot = uRwellTools::Y_bot_edge; // -250

        const double cx[4] = {-X_top, X_top, X_bot, -X_bot};
        const double cy[4] = {Y_top, Y_top, Y_bot, Y_bot};

        std::vector<std::pair<double, double>> points;

        for (int i = 0; i < 4; i++) {
            int j = (i + 1) % 4;
            // Edge parametrization: P = P_i + t*(P_j - P_i), t in [0, 1]
            double dx = cx[j] - cx[i];
            double dy = cy[j] - cy[i];

            double denom = dy - a * dx;
            if (fabs(denom) < 1.e-12) {
                continue; // The line is parallel to this edge
            }
            double t = (a * cx[i] + b - cy[i]) / denom;
            if (t < 0. || t > 1.) {
                continue;
            }

            double px = cx[i] + t * dx;
            double py = cy[i] + t * dy;

            // Skip duplicate points (the line passing exactly through a corner)
            bool duplicate = false;
            for (const auto &pt : points) {
                if (fabs(pt.first - px) < 1.e-6 && fabs(pt.second - py) < 1.e-6) {
                    duplicate = true;
                    break;
                }
            }
            if (!duplicate) {
                points.emplace_back(px, py);
            }
        }

        if (points.size() < 2) {
            return false;
        }

        x1 = points[0].first;
        y1 = points[0].second;
        x2 = points[1].first;
        y2 = points[1].second;
        return true;
    }
}
