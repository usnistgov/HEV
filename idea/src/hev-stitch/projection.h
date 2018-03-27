#ifndef STITCH_PROJECTION_H
#define STITCH_PROJECTION_H

#include "cube.h"
#include "sampling.h"
#include <cmath>

namespace projection {

struct ray {
    double x;
    double y;
    double z;
};

inline ray spherical(int width, int height, sampling::point const& point) {
    double const theta = M_PI   * ((2*point.x)/width - 1);
    double const phi =   M_PI_2 * ((2*point.y)/height - 1);

    ray r;
    r.x = std::cos(phi) * std::cos(theta);
    r.y = std::sin(phi);
    r.z = std::cos(phi) * std::sin(theta);

    return r;
}

template <typename Projection, typename Sampler>
void project(Projection& proj, Sampler& samp, cube::faces const& faces,
             int width, int height, int channels, unsigned char* pixels) {
    int const nChannels = std::min(channels, faces.channels);

    for (int j=0; j<height; ++j) {
    for (int i=0; i<width; ++i) {
        std::vector<sampling::point> const points = samp(i,j);
        double const nPoints = points.size();

        double v[nChannels];
        memset(v, 0, nChannels*sizeof(double));

        for (int k=0; k<points.size(); ++k) {
            ray const l = proj(width, height, points[k]);
            cube::coordinate const c = faces.pick(l.x,l.y,l.z);
            int const x = static_cast<int>((c.x + 1.0) * faces.width  / 2.0);
            int const y = static_cast<int>((c.y + 1.0) * faces.height / 2.0);

            int const o = ((y*faces.width) + x)*faces.channels;
            for (int l=0; l<nChannels; ++l) {
                v[l] += static_cast<double>(faces.pixels[c.f][o+l])/255;
            }
        }

        int const o = ((j*width) + i)*channels;
        for (int k=0; k<nChannels; ++k) {
            pixels[o+k] = static_cast<unsigned char>(255*(v[k]/nPoints));
        }
    }
    }
}

} // namespace projection

#endif // STITCH_PROJECTION_H

