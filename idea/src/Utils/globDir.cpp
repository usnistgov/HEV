#include "idea/Utils.h"

#include <assert.h>
#include <err.h>
#include <errno.h>
#include <glob.h>
#include <limits.h>
#include <stdio.h>

std::vector<std::string> idea::globDir(char const* dir, char const* pat) {
    char full[PATH_MAX];
    size_t const full_len = snprintf(full, PATH_MAX, "%s/%s", dir, pat);
    assert(full_len < PATH_MAX);

    errno = 0;
    glob_t gl;
    int const ec = glob(full, 0, NULL, &gl);

    std::vector<std::string> names;
    if (ec != 0) {
        if (ec != GLOB_NOMATCH) {
            if (errno == 0) warnx("%s glob failed: 0x%x", full, ec);
            else warn("%s glob failed", full);
        }
        return names;
    }

    for (ssize_t i=0; i<ssize_t(gl.gl_pathc); ++i) names.push_back(gl.gl_pathv[i]);
    return names;
}

