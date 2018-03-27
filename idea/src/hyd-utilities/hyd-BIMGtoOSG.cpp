#include "hyd_util.h"
#include "osg_util.h"
using namespace hyd;

#include <idea/Utils.h>

#include <osg/Geode>
#include <osgDB/WriteFile>
#include <osgUtil/Optimizer>

#include <assert.h>
#include <err.h>
#include <errno.h>
#include <execinfo.h>
#include <getopt.h>
#include <libgen.h>
#include <limits.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>

#include <string>

static size_t const NUM_BINS = 23;
static osg::Vec4 const BIN_COLORS[] = {
    osg::Vec4(0.066666666666666666, 0.12941176470588237, 0.38039215686274508, 1.0),
    osg::Vec4(0.078431372549019607, 0.19607843137254902, 0.46666666666666667, 1.0),
    osg::Vec4(0.14901960784313725, 0.26666666666666666, 0.59999999999999998, 1.0),
    osg::Vec4(0.2196078431372549, 0.35294117647058826, 0.59999999999999998, 1.0),
    osg::Vec4(0.32549019607843138, 0.43137254901960786, 0.59215686274509804, 1.0),
    osg::Vec4(0.40784313725490196, 0.49803921568627452, 0.5725490196078431, 1.0),
    osg::Vec4(0.47843137254901963, 0.55294117647058827, 0.55294117647058827, 1.0),
    osg::Vec4(0.54117647058823526, 0.59607843137254901, 0.52156862745098043, 1.0),
    osg::Vec4(0.59999999999999998, 0.62745098039215685, 0.49019607843137253, 1.0),
    osg::Vec4(0.65098039215686276, 0.64313725490196083, 0.45098039215686275, 1.0),
    osg::Vec4(0.69411764705882351, 0.65098039215686276, 0.41176470588235292, 1.0),
    osg::Vec4(0.72941176470588232, 0.6470588235294118, 0.36862745098039218, 1.0),
    osg::Vec4(0.75686274509803919, 0.63137254901960782, 0.32156862745098042, 1.0),
    osg::Vec4(0.77254901960784317, 0.60392156862745094, 0.27450980392156865, 1.0),
    osg::Vec4(0.7803921568627451, 0.56862745098039214, 0.22745098039215686, 1.0),
    osg::Vec4(0.7803921568627451, 0.52156862745098043, 0.18431372549019609, 1.0),
    osg::Vec4(0.7686274509803922, 0.46666666666666667, 0.13725490196078433, 1.0),
    osg::Vec4(0.74509803921568629, 0.40392156862745099, 0.094117647058823528, 1.0),
    osg::Vec4(0.71372549019607845, 0.33333333333333331, 0.058823529411764705, 1.0),
    osg::Vec4(0.66666666666666663, 0.25098039215686274, 0.027450980392156862, 1.0),
    osg::Vec4(0.61176470588235299, 0.15686274509803921, 0.011764705882352941, 1.0),
    osg::Vec4(0.54509803921568623, 0.0, 0.0, 1.0),
    osg::Vec4(0.5607843137254902, 0.0, 0.0, 1.0),
    osg::Vec4(1.0, 0.25882352941176473, 0.55294117647058827, 1.0),
};

static int start_read(char const* FN, unsigned long int data_column, bimg_header* head, char** names) {
    assert(FN);
    assert(head);
    memset(head, 0, sizeof(*head));

    int fd = open(FN, O_RDONLY);
    if (fd < 0) return -1;

    // ignore errors from posix_fadvise
    posix_fadvise(fd, 0, 0, POSIX_FADV_SEQUENTIAL);
    posix_fadvise(fd, 0, 0, POSIX_FADV_NOREUSE);
    posix_fadvise(fd, 0, 0, POSIX_FADV_WILLNEED);

    errno = EIO;
    if (read(fd, head, sizeof(*head)) != sizeof(*head)) {
        err(EXIT_FAILURE, "cannot read header from %s", FN);
    }

    errno = EINVAL;
    if (strcmp(reinterpret_cast<char const*>(head->magic), bimg_header::MAGIC) != 0) {
        err(EXIT_FAILURE, "%s does not appear to be a BIMG file", FN);
    }

    if (names) {
        for (uint64_t i=0; i<head->nItemsPerSite; ++i) {
            errno = EIO;
            if (read(fd, names[i], bimg_header::NAME_LEN) != bimg_header::NAME_LEN) {
                err(EXIT_FAILURE, "cannot read names from %s", FN);
            }
        }
    } else {
        if (lseek(fd, bimg_header::NAME_LEN*head->nItemsPerSite, SEEK_CUR) < 0) {
            err(EXIT_FAILURE, "cannot seek to data in %s", FN);
        }
    }

    if (lseek(fd, data_column*sizeof(double), SEEK_CUR) < 0) {
        err(EXIT_FAILURE, "cannot seek to data in %s", FN);
    }

    return fd;
}

static double read_site(char const* FN, int fd, size_t offset) {
    double data;
    size_t const nread = read(fd, &data, sizeof(data));

    if (nread == 0) errx(EXIT_FAILURE, "reached EOF in %s", FN);
    if (nread != sizeof(data)) err(EXIT_FAILURE, "cannot read data from %s", FN);

    if (lseek(fd, offset, SEEK_CUR) < 0) err(EXIT_FAILURE, "cannot seek in %s", FN);
    return data;
}

static void write_data(std::vector<std::string> const& bimgs, int (&sizes)[4],
    bimg_header* headers, unsigned long int data_column,
    long int start_ts, long int end_ts) {
    osg::Vec3f const scale(0.8f, 0.8f, 0.8f);
    double const bin_width = 1.0/double(NUM_BINS-1);
    size_t const offset = (headers[0].nItemsPerSite-1)*sizeof(double);
    size_t const num_sites = headers[0].dims[0]*headers[0].dims[1]*headers[0].dims[2];

    #pragma omp parallel for
    for (uint64_t i=start_ts; i<end_ts; ++i) {
        char const* fname = bimgs[i].c_str();
        bimg_header* header = &headers[i];

        char nodeName[LINE_MAX];
        snprintf(nodeName, LINE_MAX, "dc%02lu_ts%03lu", data_column, i);

        osg::Geode* node = new osg::Geode;
        node->setName(nodeName);

        int x=0, y=0, z=0;
        int fd = start_read(fname, data_column, header, NULL);

        for (size_t j=0; j<num_sites; ++j) {
            double const data = read_site(fname, fd, offset);

            if (data > 0.0) {
                for (size_t bin=0; bin<NUM_BINS; ++bin) {
                    if (data <= bin*bin_width) {
                        node->addDrawable(unit_cube(scale, osg::Vec3(x,y,z), BIN_COLORS[bin]));
                        break;
                    }
                }
            }

            x+=1;
            if (x>=sizes[1]) { x=0; y+=1; }
            if (y>=sizes[2]) { y=0; z+=1; }
        }

        close(fd);

        osgUtil::Optimizer optimizer;
        optimizer.optimize(node, osgUtil::Optimizer::MERGE_GEOMETRY);

        char fileName[PATH_MAX];
        snprintf(fileName, PATH_MAX, "%s.osgb", nodeName);

        osgDB::writeNodeFile(*node, fileName);
    }
}

static void write_iris(char const* fileName, char const* groupName,
                       bimg_header* headers, unsigned long int data_column,
                       long int start_ts, long int end_ts, bool geom) {
    FILE* fh = fopen(fileName, "w");
    if (!fh) err(EXIT_FAILURE, "cannot open %s", fileName);

    fprintf(fh, "GROUP %s\n", groupName);

    char nodeName[LINE_MAX];
    for (uint64_t i=start_ts; i<end_ts; ++i) {
        snprintf(nodeName, LINE_MAX, "dc%02lu_ts%03lu", data_column, i);

        if (geom) fprintf(fh, "LOAD %s %s.osgb\n", nodeName, nodeName);
        else fprintf(fh, "GROUP %s\n", nodeName);

        fprintf(fh, "NODEMASK %s %s\n", nodeName, (i==0)?"ON":"OFF");
        fprintf(fh, "ADDCHILD %s %s\n", nodeName, groupName);
    }

    fprintf(fh, "RETURN %s\n", groupName);
    fclose(fh);
}

static void write_times(char const* fileName, bimg_header* headers,
                        long int start_ts, long int end_ts) {
    FILE* fh = fopen(fileName, "w");
    if (!fh) err(EXIT_FAILURE, "cannot open %s", fileName);

    for (uint64_t i=start_ts; i<end_ts; ++i) fprintf(fh, "%g\n", headers[i].time);
    fclose(fh);
}

static void backtrace_signal(int signo, siginfo_t*, void*) {
    char* sigdesc = strsignal(signo);
    write(STDERR_FILENO, sigdesc, strlen(sigdesc));
    write(STDERR_FILENO, "\n", 1);

    void* buffer[1024];
    int nptrs = backtrace(buffer, 1024);
    backtrace_symbols_fd(buffer, nptrs, STDERR_FILENO);

    abort();
}

static void usage(char const* argv0, bool help) {
    printf("usage: %s [options] <input.dat> <data_column>\n", basename(strdupa(argv0)));
    if (!help) return;
    printf("options:\n");
    printf("    --start <timestep>    start at <timestep>\n");
    printf("    --end <timestep>      end at <timestep>\n");
}

int main(int argc, char* argv[]) {
    struct sigaction sa = {0};
    sa.sa_sigaction = backtrace_signal;
    sa.sa_flags = SA_SIGINFO;
    sigemptyset(&sa.sa_mask);
    if (sigaction(SIGSEGV, &sa, NULL) != 0) errx(EXIT_FAILURE, "cannot install signal handler");

    long int start_ts = 0, end_ts = -1;

    static struct option lopts[] = {
        { "help", no_argument, NULL, 'h' },
        { "start", required_argument, NULL, 0 },
        { "end", required_argument, NULL, 0 },
        { NULL, 0, NULL, 0 }
    };

    char* end = NULL;
    static char const* sopts = "h";

    int sopt, lopt;
    while ((sopt = getopt_long(argc, argv, sopts, lopts, &lopt)) != -1) {
        switch(sopt) {
        case 0:
            if (strcmp("start", lopts[lopt].name) == 0) {
                errno = 0;
                end = NULL;
                start_ts = strtol(optarg, &end, 10);
                if (*end != '\0') errx(EXIT_FAILURE, "invalid start %s", optarg);
                if (errno != 0) err(EXIT_FAILURE, "invalid start %s", optarg);
            } else if (strcmp("end", lopts[lopt].name) == 0) {
                errno = 0;
                end = NULL;
                end_ts = strtol(optarg, &end, 10);
                if (*end != '\0') errx(EXIT_FAILURE, "invalid end %s", optarg);
                if (errno != 0) err(EXIT_FAILURE, "invalid end %s", optarg);
            }
            break;

        case 'h':
        default: /* '?' */
            usage(argv[0], sopt=='h');
            exit((sopt=='h') ? EXIT_SUCCESS : EXIT_FAILURE);
            break;
        }
    }

    if (optind+2 != argc) errx(EXIT_FAILURE, "either input.dat or data_column not specified");

    std::string input_dat = argv[optind++];
    std::string input_dir = dirname(strdupa(input_dat.c_str()));

    errno = 0;
    end = NULL;
    unsigned long int data_column = strtoul(argv[optind], &end, 10);
    if (*end != '\0') errx(EXIT_FAILURE, "invalid data_column %s", argv[optind]);
    if (errno != 0) err(EXIT_FAILURE, "invalid data_column %s", argv[optind]);


    std::pair<std::string, std::string> sr = parse_dat(input_dat.c_str(), input_dir.c_str());
    if (sr.first.empty()||sr.second.empty()) errx(EXIT_FAILURE, "cannot parse %s", input_dat.c_str());

    int sizes[4] = {0, 0, 0, 0};
    std::vector<std::vector<int> > ranks = parse_struct(sr.first.c_str(), sizes);
    if (ranks.empty()) errx(EXIT_FAILURE, "cannot parse %s", sr.first.c_str());

    std::vector<std::string> bimgs = idea::globDir(input_dir.c_str(), "*.bimg");
    if (bimgs.size() == 0) errx(EXIT_FAILURE, "cannot read bimg files from %s\n", input_dir.c_str());

    bimg_header* headers = new bimg_header[bimgs.size()];
    if (read_bimg_header(bimgs[0].c_str(), &headers[0]) != 0) {
        err(EXIT_FAILURE, "cannot read header from %s", bimgs[0].c_str());
    }

    if (end_ts < 0) end_ts = bimgs.size();

    write_data(bimgs, sizes, headers, data_column, start_ts, end_ts);

    char groupName[LINE_MAX], fileName[PATH_MAX];
    snprintf(groupName, LINE_MAX, "dc%02lu", data_column);

    snprintf(fileName, PATH_MAX, "%s.iris", groupName);
    write_iris(fileName, groupName, headers, data_column, start_ts, end_ts, true);

    snprintf(fileName, PATH_MAX, "%s_anim.iris", groupName);
    write_iris(fileName, groupName, headers, data_column, start_ts, end_ts, false);

    snprintf(fileName, PATH_MAX, "%s_times.txt", groupName);
    write_times(fileName, headers, start_ts, end_ts);
}

