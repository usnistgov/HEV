#ifndef STITCH_SAMPLING_H
#define STITCH_SAMPLING_H

#include <cstdlib>
#include <vector>

namespace sampling {

struct point {
    double x;
    double y;
};

struct jittered {
    int const m, n;

    jittered(int M, int N) : m(M), n(N) {}

    std::vector<point> operator()(int x, int y) const {
        std::vector<point> p(n*m);

        for (int j=0; j<n; ++j) {
        for (int i=0; i<m; ++i) {
            int const o = j*m + i;
            p[o].x = (i + (j + drand48()) / n) / m;
            p[o].y = (j + (i + drand48()) / m) / n;
        }
        }

        for (int j=0; j<n; ++j) {
            int const k = j + drand48() * (n - j);
            for (int i=0; i<m; ++i) std::swap(p[j*m + i].x, p[k*m + i].x);
        }

        for (int i=0; i<m; ++i) {
            int const k = i + drand48() * (m - i);
            for (int j=0; j<n; ++j) std::swap(p[j*m + i].y, p[j*m + k].y);
        }

        for (int i=0; i<n*m; ++i) { p[i].x += x; p[i].y += y; } // scale
        return p;
    }
};

} // namespace sampling

#endif // STITCH_SAMPLING_H

