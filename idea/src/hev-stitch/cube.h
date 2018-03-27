#ifndef STITCH_CUBE_H
#define STITCH_CUBE_H

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include "stb_image.h"

namespace cube {

enum index {
    eLEFT = 0,
    eRIGHT,
    eUP,
    eDOWN,
    eFRONT,
    eBACK,
    eNUM_FACES
};

struct coordinate {
    index f;
    double x;
    double y;
};

struct faces {
    int width, height, channels;
    unsigned char* pixels[cube::eNUM_FACES];

    static faces read(char* const FNs[]) {
        faces f;

        for (int i=0; i<eNUM_FACES; ++i) {
            int w, h, n;
            f.pixels[i] = stbi_load(FNs[i], &w, &h, &n, 3);
            if (i==0) { f.width = w; f.height = h; f.channels = 3; }

            if (!f.pixels[i]) {
                std::fprintf(stderr, "error reading %s: %s\n", FNs[i]);
                std::exit(EXIT_FAILURE);
            }

            if (f.width!=w || f.height!=h || f.channels!=3) {
                std::fprintf(stderr,
                    "dimensions of %s (%dx%dx%d) do not match (%dx%dx%d)\n",
                    w, h, n, f.width, f.height, f.channels);
            }
        }

        return f;
    }

    coordinate pick(double x, double y, double z) const {
        coordinate c;

        double const xMag = std::abs(x);
        double const yMag = std::abs(y);
        double const zMag = std::abs(z);

        if (xMag>yMag) {
            if (xMag>zMag) {    // x is the face
                if (x<0) {
                    c.f = eBACK;
                    c.x = -z/xMag;
                } else {
                    c.f = eFRONT;
                    c.x =  z/xMag;
                }

                c.y = y/xMag;
            } else {            // z is the face
                if (z<0) {
                    c.f = eLEFT;
                    c.x =  x/zMag;
                } else {
                    c.f = eRIGHT;
                    c.x = -x/zMag;
                }

                c.y = y/zMag;
            }
        } else {
            if (yMag>zMag) {    // y is the face
                if (y<0) {
                    c.f = eUP;
                    c.y = -z/yMag;
                } else {
                    c.f = eDOWN;
                    c.y =  z/yMag;
                }

                c.x = x/yMag;
            } else {            // z is the face
                if (z<0) {
                    c.f = eLEFT;
                    c.x =  x/zMag;
                } else {
                    c.f = eRIGHT;
                    c.x = -x/zMag;
                }

                c.y = y/zMag;
            }
        }

        return c;
    }

    ~faces() {
        for (int i=0; i<eNUM_FACES; ++i) stbi_image_free(pixels[i]);
    }
};

} // namespace cube

#endif // STITCH_CUBE_H

