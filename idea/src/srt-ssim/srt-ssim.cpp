#include <osg/Image>
#include <osgDB/FileNameUtils>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>

#include <algorithm>
#include <cstring>
#include <err.h>

extern "C" {
#include <iqa.h>
}

inline osg::Vec4d rgbToXyz(osg::Vec4d const& c) {
    osg::Vec4d xyz;
    xyz[0] = 0.412453 * c[0] + 0.357580 * c[1] + 0.180423 * c[2];
    xyz[1] = 0.212671 * c[0] + 0.715160 * c[1] + 0.072169 * c[2];
    xyz[2] = 0.019334 * c[0] + 0.119193 * c[1] + 0.950227 * c[2];
    return xyz;
}

inline osg::Vec4d xyzToRgb(osg::Vec4d const& c) {
    osg::Vec4d rgb;
    rgb[0] =  3.2404542 * c[0] - 1.5371385 * c[1] - 0.4985314 * c[2];
    rgb[1] = -0.9692660 * c[0] + 1.8760108 * c[1] + 0.0415560 * c[2];
    rgb[2] =  0.0556434 * c[0] - 0.2040259 * c[1] + 1.0572252 * c[2];
    return rgb;
}

inline double toLinear(double d) {
    return pow(d, 2.2);
}

inline double toGamma(double d) {
    return pow(d, 1.0/2.2);
}

inline osg::Vec4d toLinear(osg::Vec4d const& c) {
    return osg::Vec4d(toLinear(c[0]), toLinear(c[1]), toLinear(c[2]), c[3]);
}

inline osg::Vec4d toGamma(osg::Vec4d const& c) {
    return osg::Vec4d(toGamma(c[0]), toGamma(c[1]), toGamma(c[2]), c[3]);
}

inline double f(double t) {
    if (t > pow(6.0/23.0, 3)) return pow(t, 1.0/3.0);
    else return 1.0/3.0 * pow(29.0/6.0, 2) * t + 4.0 / 29.0;
}

inline double finv(double t) {
    if (t > 6.0 / 29.0) return 3.0 * pow(6.0 / 29.0, 2) * (t - 4.0 / 29.0);
    else return pow(t, 3.0);
}

static osg::Vec4d xyzToCieLab(osg::Vec4d const& c) {
    // Normalized white point.
    const double Xn = 0.950456;
    const double Yn = 1.0;
    const double Zn = 1.088754;

    const double Xr = c[0] / Xn;
    const double Yr = c[1] / Yn;
    const double Zr = c[2] / Zn;

    const double fx = f(Xr);
    const double fy = f(Yr);
    const double fz = f(Zr);

    const double L = 116 * fx - 16;
    const double a = 500 * (fx - fy);
    const double b = 200 * (fy - fz);

    return osg::Vec4d(L, a, b, c[3]);
}

static osg::Vec4d rgbToCieLab(osg::Vec4d const& c) {
    return xyzToCieLab(rgbToXyz(toLinear(c)));
}

// h is hue-angle in radians
static osg::Vec4d cieLabToLCh(osg::Vec4d const& c) {
    return osg::Vec4d(c[0], sqrt(c[1]*c[1] + c[2]*c[2]), atan2f(c[1],c[2]), c[3]);
}

osg::Image* rgbToL(osg::Image const& rgb) {
    osg::ref_ptr<osg::Image> l = new osg::Image;
    l->allocateImage(rgb.s(), rgb.t(), 1, GL_LUMINANCE, GL_UNSIGNED_BYTE);

    int const s = rgb.s();
    int const t = rgb.t();
    unsigned char* d = l->data();

    for (int y=0; y<t; ++y) {
    for (int x=0; x<s; ++x) {
        osg::Vec4d const lab = rgbToCieLab(rgb.getColor(x,y));
        d[y*s + x] = static_cast<unsigned char>((lab[0]/100.0) * 255.0);
    }
    }

    return l.release();
}

int main(int argc, char* argv[]) {
    osg::ArgumentParser args(&argc, argv);
    osg::ApplicationUsage* usage = args.getApplicationUsage();
    usage->setApplicationName(args.getApplicationName());
    usage->setDescription("Compute the SSIM between two images.");
    usage->setCommandLineUsage(args.getApplicationName() + " [options] ref dst");
    usage->addCommandLineOption("-m,--map <filename>", "write SSIM map (no)");

    unsigned int help = args.readHelpType();
    if (help > 0) {
        args.getApplicationUsage()->write(std::cerr, help);
        exit(EXIT_SUCCESS);
    }

    if (args.errors()) {
        args.writeErrorMessages(std::cerr);
        exit(EXIT_FAILURE);
    }

    bool doMS = false;
    while (args.read("--multiscale", doMS));

    std::string map_filename;
    while (args.read("-m", map_filename) || args.read("--map", map_filename));

    args.reportRemainingOptionsAsUnrecognized();
    if (args.errors()) {
        args.writeErrorMessages(std::cerr);
        exit(EXIT_FAILURE);
    }

    if (args.argc() != 3) errx(EXIT_FAILURE, "ref and dst required");

    osg::ref_ptr<osg::Image> ri = osgDB::readImageFile(args[1]);
    if (!ri) errx(EXIT_FAILURE, "unable to load %s", args[1]);

    osg::ref_ptr<osg::Image> di = osgDB::readImageFile(args[2]);
    if (!di) errx(EXIT_FAILURE, "unable to load %s", args[2]);

    if (ri->s() != di->s() || ri->t() != di->t() ||
        ri->getRowSizeInBytes() != di->getRowSizeInBytes()) {
        errx(EXIT_FAILURE, "%s and %s have different dimensions", args[1], args[2]);
    }

    if (ri->getPixelFormat() != di->getPixelFormat()) {
        errx(EXIT_FAILURE, "%s and %s have different formats", args[1], args[2]);
    }

    if (ri->getPixelFormat() == GL_RGB || ri->getPixelFormat() == GL_RGBA) {
        ri = rgbToL(*ri);
        di = rgbToL(*di);
    }

    float mean_ssim;

    if (!map_filename.empty()) {
        float* ssim_map;
        int ssim_map_w, ssim_map_h;
        mean_ssim = iqa_ssim(ri->data(), di->data(), ri->s(), ri->t(), ri->s(),
                             0, NULL, &ssim_map, &ssim_map_w, &ssim_map_h);

        std::string const ext = osgDB::getFileExtension(map_filename);
        if (ext=="mha") {
            FILE* fh = fopen(map_filename.c_str(), "wb");
            if (!fh) errx(EXIT_FAILURE, "cannot open %s", map_filename.c_str());

            fprintf(fh, "ObjectType = Image\n");
            fprintf(fh, "NDims = 2\n");
            fprintf(fh, "DimSize = %d %d\n", ssim_map_w, ssim_map_h);
            fprintf(fh, "ElementNumberOfChannels = 1\n");
            fprintf(fh, "BinaryDataByteOrderMSB = False\n");
            fprintf(fh, "ElementType = MET_FLOAT\n");
            fprintf(fh, "ElementDataFile = LOCAL\n");

            ssize_t nwrite = fwrite(ssim_map, sizeof(float), ssim_map_w*ssim_map_h, fh);
            if (nwrite != ssim_map_w*ssim_map_h || ferror(fh)) {
                errx(EXIT_FAILURE, "cannot write %s", map_filename.c_str());
            }

            if (fclose(fh) != 0) {
                err(EXIT_FAILURE, "cannot write %s", map_filename.c_str());
            }
        } else {
            osg::ref_ptr<osg::Image> map = new osg::Image;
            map->allocateImage(ssim_map_w, ssim_map_h, 1, GL_LUMINANCE, GL_UNSIGNED_BYTE);
            unsigned char* data = map->data();

            for (int y=0; y<ssim_map_h; ++y) {
            for (int x=0; x<ssim_map_w; ++x) {
                size_t const off = y*ssim_map_w + x;

                data[off] = static_cast<unsigned char>(ssim_map[off]*255.0f);
            }
            }

            osgDB::writeImageFile(*map, map_filename);
        }
    } else {
        mean_ssim = iqa_ssim(ri->data(), di->data(), ri->s(), ri->t(), ri->s(),
                             0, NULL, NULL, NULL, NULL);
    }

    std::cout << mean_ssim << std::endl;
}

