#include "hyd_util.h"
#include "idea/Utils.h"

#include <assert.h>
#include <err.h>
#include <errno.h>
#include <glob.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

namespace hyd {

char const* const bimg_header::MAGIC = "HYIB";

static int read_header(char const* FN, bimg_header* head) {
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
    if (read(fd, head, sizeof(*head)) != sizeof(*head)) goto err;

    errno = EINVAL;
    if (strcmp(reinterpret_cast<char const*>(head->magic), bimg_header::MAGIC) != 0) goto err;

    return fd;
err:
    close(fd);
    return -1;
}

int read_bimg_header(char const* FN, bimg_header* head) {
    assert(FN);
    assert(head);

    int fd = read_header(FN, head);
    if (fd < 0) return -1;

    close(fd);
    return 0;
}

int read_bimg(char const* FN, bimg_header* head, char** names, double* siteData) {
    assert(FN);
    assert(head);
    assert(siteData);
    int ret = -1;
    size_t nbytes;

    int fd = read_header(FN, head);
    if (fd < 0) goto err;

    if (names) {
        for (uint64_t i=0; i<head->nItemsPerSite; ++i) {
            errno = EIO;
            if (read(fd, names[i], bimg_header::NAME_LEN) != bimg_header::NAME_LEN) goto err;
        }
    } else {
        if (lseek(fd, bimg_header::NAME_LEN*head->nItemsPerSite, SEEK_CUR) < 0) goto err;
    }

    errno = EIO;
    nbytes = head->dims[0]*head->dims[1]*head->dims[2]*head->nItemsPerSite*sizeof(double);
    if (read(fd, siteData, nbytes) != nbytes) goto err;

    ret = 0;
err:
    if (fd >= 0) close(fd);
    return ret;
}

int read_bimg(char const* DN, bimg_header*& headers, double*& data, char**& names, uint64_t& nTimesteps,
              uint64_t& nSites, uint64_t& nItemsPerSite) {
    std::vector<std::string> bimgs = idea::globDir(DN, "*.bimg");
    nTimesteps = bimgs.size();
    if (nTimesteps == 0) return -1;

    headers = new bimg_header[nTimesteps];
    if (read_bimg_header(bimgs[0].c_str(), &headers[0]) != 0) {
        warn("cannot read header from %s", bimgs[0].c_str());
        return -1;
    }

    nItemsPerSite = headers[0].nItemsPerSite;
    nSites = headers[0].dims[0]*headers[0].dims[1]*headers[0].dims[2];

    data = new double[nTimesteps*nItemsPerSite*nSites];

    names = new char*[nItemsPerSite];
    for (uint64_t i=0; i<nItemsPerSite; ++i) names[i] = new char[bimg_header::NAME_LEN];

    bool abort = false;

    #pragma omp parallel for
    for (uint64_t i=0; i<nTimesteps; ++i) {
        #pragma omp flush(abort)
        if (!abort) {
            bimg_header* header = &headers[i];
            double* ts_data = data + i*nItemsPerSite*nSites;

            if (read_bimg(bimgs[i].c_str(), header, (i==0) ? names : NULL, ts_data)!=0) {
                warn("cannot read %s", bimgs[i].c_str());
                abort = true;
                #pragma omp flush(abort)
            }
        }
    }

    if (abort) return -1;
    return 0;
}

std::vector<std::vector<int> > parse_struct(char const* FN, int (&sizes)[4]) {
    assert(FN);
    std::vector<std::vector<int> > ranks;

    char* line = NULL;
    char* ldup = NULL;
    char* strtok_state = NULL;
    size_t len;
    ssize_t nread;

    int junk;
    std::vector<int> rank(6, 0);

    FILE* fp = fopen(FN, "r");
    if (!fp) goto err;

    if (getline(&line, &len, fp) == -1) goto err;
    ldup = strdupa(line);
    if (strcmp("Proc", strtok_r(ldup, " ", &strtok_state)) != 0) goto err;

    if (fscanf(fp, "%d %d %d %d\n", &sizes[0], &sizes[1], &sizes[2], &sizes[3]) == -1) goto err;

    if (getline(&line, &len, fp) == -1) goto err;
    ldup = strdup(line);
    if (strcmp("Rank", strtok_r(ldup, " ", &strtok_state)) != 0) goto err;

    for (int i=0; i<sizes[0]; ++i) {
        if (getline(&line, &len, fp) == -1) goto err;
        if (sscanf(line, "%d %d %d %d %d %d %d", &junk, &rank[0], &rank[1], &rank[2],
                   &rank[3], &rank[4], &rank[5]) != 7) goto err;
        ranks.push_back(rank);
    }

err:
    free(line);
    fclose(fp);
    return ranks;
}

std::pair<std::string, std::string> parse_dat(char const* FN, char const* DN) {
    assert(FN);
    assert(DN);
    static std::string const DELIM = "/";
    std::pair<std::string, std::string> fnames;

    char* strtok_state;
    char* line = NULL;
    size_t len = 0;
    ssize_t nread;
    bool rf = false;

    FILE* fp = fopen(FN, "r");
    if (!fp) goto err;

    while ((nread = getline(&line, &len, fp)) != -1) {
        if (line[strlen(line)-1] == '\n') line[strlen(line)-1] = '\0';
        char* save = strdup(line);

        char* tok = strtok_r(line, "/", &strtok_state);
        char* val = strtok_r(NULL, "", &strtok_state);

        if (strcmp("reactions", tok) == 0) {
            fnames.second = DN + DELIM + save;
        } else if (val == NULL && !fnames.second.empty()) {
            fnames.first = DN + DELIM + tok + ".struct";
        }

        free(save);
    }

err:
    free(line);
    fclose(fp);
    return fnames;
}

} // namespace hyd

