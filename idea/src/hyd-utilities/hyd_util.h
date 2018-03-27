#ifndef HYD_UTIL_H
#define HYD_UTIL_H

#include <stdint.h>
#include <stdlib.h>
#include <string>
#include <vector>

namespace hyd {

struct bimg_header {
    static size_t const NAME_LEN = 128;
    static size_t const MAGIC_LEN = 4;
    static char const* const MAGIC;

    uint8_t magic[MAGIC_LEN];
    uint8_t unused[4];
    double time;
    uint64_t nItemsPerSite;
    uint64_t dims[3];
    uint8_t unused2[2048];
};

// read the header from a bimg file
// FN: the filename of the bimg file (must not be null)
// head: a pointer to the bimg_header (must not be null)
// return: 0 on success, -1 on error (errno is set)
int read_bimg_header(char const* FN, bimg_header* head);

// read a bimg file
// FN: the filename of the bimg file (must not be null)
// head: a pointer to the bimg_header (must not be null)
// names: an array of names that will be allocated (can be null)
// siteData: the data from the file (must not be null)
// return: 0 on success, -1 on error (errno is set)
int read_bimg(char const* FN, bimg_header* head, char** names, double* siteData);

// read all the bimg files from a directory
// DN: the directory of the bimg files (must not be null)
// headers: an array of bimg_headers that will be allocated
// data: an array of site data that will be allocated
// names: an array of names that will be allocated
// nTimesteps: the number of timesteps
// nSites: the number of sites per timestep
// nItemsPerSite: the number of items per site
// return: 0 on success, -1 on error (errno is set)
int read_bimg(char const* DN, bimg_header*& headers, double*& data, char**& names,
              uint64_t& nTimesteps, uint64_t& nSites, uint64_t& nItemsPerSite);

// parse a struct file
// FN: the struct filename (must not be null)
// sizes: the sizes read from FN
// return: the ranks read from FN
std::vector<std::vector<int> > parse_struct(char const* FN, int (&sizes)[4]);

// parse the struct and reaction files from a dat file
// FN: the dat filename (must not be null)
// DN: the relative directory of the dat file
// return a pair where first = the struct file and second = the reaction file
std::pair<std::string, std::string> parse_dat(char const* FN, char const* DN);

} // namespace hyd

#endif // HYD_UTIL_H

