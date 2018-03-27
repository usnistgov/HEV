#include <cerrno>
#include <climits>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <strings.h> // POSIX strcasecmp
#include <unistd.h> // POSIX getopt
#include "projection.h"
#include "stb_image_write.h"

namespace {

bool parse_int(char const* str, int* result) {
    errno = 0;
    char* end = NULL;
    long int value = strtol(str, &end, 0);

    if (str=='\0' || *end!='\0') {
        errno = EINVAL;
        return false;
    } else if ((value==LONG_MIN || value==LONG_MAX) && errno==ERANGE) {
        return false;
    } else if (errno==EINVAL) {
        return false;
    }

    *result = static_cast<int>(value);
    return true;
}

} // namespace <anonymous>

int main(int argc, char* argv[]) {
    int oW = 0;
    int oH = 0;
    char *oFN = NULL;

    int opt;
    while ((opt = getopt(argc, argv, ":W:H:O:h")) != -1) {
        switch(opt) {
        case 'W':
            if (!parse_int(optarg, &oW)) {
                std::fprintf(stderr, "invalid argument '%s' for -W: %s\n",
                             optarg, strerror(errno));
                std::exit(EXIT_FAILURE);
            }
            break;

        case 'H':
            if (!parse_int(optarg, &oH)) {
                std::fprintf(stderr, "invalid argument '%s' for -H: %s\n",
                             optarg, strerror(errno));
                std::exit(EXIT_FAILURE);
            }
            break;

        case 'O':
            oFN = optarg;
            break;

        case ':':
            std::fprintf(stderr, "option -%c requires an argument\n", optopt);
            std::exit(EXIT_FAILURE);
            break;

        case 'h':
        default: /* ? */
            std::fprintf(stderr, "usage: %s [options] -O <output> -W <width> "
                "-H <height> <left> <right> <up> <down> <front> <back>\n",
                argv[0]);
            if (opt!='h') std::exit(EXIT_FAILURE);
            std::fprintf(stderr, "options:\n"
                "    -O         output filename\n"
                "    -W         output image width\n"
                "    -H         output image height\n"
            );
            std::exit(EXIT_SUCCESS);
            break;
        }
    }

    if (oW<=0) {
        std::fprintf(stderr, "option -W required\n");
        std::exit(EXIT_FAILURE);
    }

    if (oH<=0) {
        std::fprintf(stderr, "option -H required\n");
        std::exit(EXIT_FAILURE);
    }

    if (!oFN) {
        std::fprintf(stderr, "option -O required\n");
        std::exit(EXIT_FAILURE);
    }

    if (strcasecmp(std::strrchr(oFN, '.'), ".png") != 0) {
        std::fprintf(stderr, "output filename must be png\n");
        std::exit(EXIT_FAILURE);
    }

    if ((optind+cube::eNUM_FACES)!=argc) {
        std::fprintf(stderr, "%s input filenames\n",
            (optind+cube::eNUM_FACES<argc) ? "too many" : "not enough");
        std::exit(EXIT_FAILURE);
    }

    sampling::jittered sampler(4,4);
    cube::faces faces = cube::faces::read(argv+optind);

    unsigned char* pix = new unsigned char[oW*oH*3];
    projection::project(projection::spherical, sampler, faces, oW, oH, 3, pix);

    stbi_write_png(oFN, oW, oH, 3, pix, oW*3*sizeof(unsigned char));
    delete[] pix;
}

