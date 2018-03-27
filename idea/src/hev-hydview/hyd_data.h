#ifndef HEV_HYDVIEW_HYD_DATA_H
#define HEV_HYDVIEW_HYD_DATA_H

#include <cstddef>

struct hyd_data {
  size_t dataSize;
  size_t nSites;
  size_t nItemsPerSite;
  size_t dimX;
  size_t dimY;
  size_t dimZ;
  size_t nTimesteps;
  size_t nAddedItems;
  float data[];
};

#endif // HEV_HYDVIEW_HYD_DATA_H

