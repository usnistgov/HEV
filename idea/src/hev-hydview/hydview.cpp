#include "hyd_data.h"
#include "tables.h"
#include <dtk.h>
#include <dtk/dtkDSO_loader.h>
#include <iris/Augment.h>
#include <iris/SceneGraph.h>
#include <iris/Utils.h>
#include <osg/PolygonMode>
#include <CL/cl.h>
#include <cerrno>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <regex.h>
#include <unistd.h>

static char const* clstrerror(cl_int ec) {
  switch (ec) {
  case CL_SUCCESS: return "Success";
  case CL_DEVICE_NOT_FOUND: return "Device not found";
  case CL_DEVICE_NOT_AVAILABLE: return "Device not available";
  case CL_COMPILER_NOT_AVAILABLE: return "Compiler not available";
  case CL_MEM_OBJECT_ALLOCATION_FAILURE: return "Mem object allocation failure";
  case CL_OUT_OF_RESOURCES: return "Out of resources";
  case CL_OUT_OF_HOST_MEMORY: return "Out of host memory";
  case CL_PROFILING_INFO_NOT_AVAILABLE: return "Profiling info not available";
  case CL_MEM_COPY_OVERLAP: return "Mem copy overlap";
  case CL_IMAGE_FORMAT_MISMATCH: return "Image format mismatch";
  case CL_IMAGE_FORMAT_NOT_SUPPORTED: return "Image format not supported";
  case CL_BUILD_PROGRAM_FAILURE: return "Build program failured";
  case CL_MAP_FAILURE: return "Map failure";
  case CL_MISALIGNED_SUB_BUFFER_OFFSET: return "Misaligned sub buffer offset";
  case CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST:
    return "Exec status error for events in wait list";
  case CL_INVALID_VALUE: return "Invalid value";
  case CL_INVALID_DEVICE_TYPE: return "Invalid device type";
  case CL_INVALID_PLATFORM: return "Invalid platform";
  case CL_INVALID_DEVICE: return "Invalid device";
  case CL_INVALID_CONTEXT: return "Invalid context";
  case CL_INVALID_QUEUE_PROPERTIES: return "Invalid queue properties";
  case CL_INVALID_COMMAND_QUEUE: return "Invalid command queue";
  case CL_INVALID_HOST_PTR: return "Invalid host ptr";
  case CL_INVALID_MEM_OBJECT: return "Invalid mem object";
  case CL_INVALID_IMAGE_FORMAT_DESCRIPTOR:
    return "Invalid image format descriptor";
  case CL_INVALID_IMAGE_SIZE: return "Invalid image size";
  case CL_INVALID_SAMPLER: return "Invalid sampler";
  case CL_INVALID_BINARY: return "Invalid binary";
  case CL_INVALID_BUILD_OPTIONS: return "Invalid build options";
  case CL_INVALID_PROGRAM: return "Invalid program";
  case CL_INVALID_PROGRAM_EXECUTABLE: return "Invalid program executable";
  case CL_INVALID_KERNEL_NAME: return "Invalid kernel name";
  case CL_INVALID_KERNEL_DEFINITION: return "Invalid kernel definition";
  case CL_INVALID_KERNEL: return "Invalid kernel";
  case CL_INVALID_ARG_INDEX: return "Invalid arg index";
  case CL_INVALID_ARG_VALUE: return "Invalid arg value";
  case CL_INVALID_ARG_SIZE: return "Invalid arg size";
  case CL_INVALID_KERNEL_ARGS: return "Invalid kernel args";
  case CL_INVALID_WORK_DIMENSION: return "Invalid work dimension";
  case CL_INVALID_WORK_GROUP_SIZE: return "Invalid work group size";
  case CL_INVALID_WORK_ITEM_SIZE: return "Invalid work item size";
  case CL_INVALID_GLOBAL_OFFSET: return "Invalid global offset";
  case CL_INVALID_EVENT_WAIT_LIST: return "Invalid event wait list";
  case CL_INVALID_EVENT: return "Invalid event";
  case CL_INVALID_OPERATION: return "Invalid operation";
  case CL_INVALID_GL_OBJECT: return "Invalid GL object";
  case CL_INVALID_BUFFER_SIZE: return "Invalid buffer size";
  case CL_INVALID_MIP_LEVEL: return "Invalid mip level";
  case CL_INVALID_GLOBAL_WORK_SIZE: return "Invalid global work size";
  case CL_INVALID_PROPERTY: return "Invalid property";
  }
} // clstrerror

static std::vector<char> read_program(char const* FN) {
  std::vector<char> bytes;

  FILE* fh = fopen(FN, "r");
  if (!fh) return bytes;

  fseek(fh, 0, SEEK_END);
  bytes.resize(ftell(fh) + 1);
  rewind(fh);
  bytes[bytes.size()] = '\0';

  fread(bytes.data(), sizeof(char), bytes.size(), fh);
  fclose(fh);

  return bytes;
} // read_program

// see kernels/isosurface.cl
#define NTHREADS 32
#define MAX_VERTS_PER_CELL 15

namespace {

class hydview : public iris::Augment {
public:
  hydview();
  virtual ~hydview();

  virtual bool control(std::string const& line,
                       std::vector<std::string> const& vec);

private:
  bool _running;

  iris::FifoReader _reply;

  int _data_shm;
  size_t _dataSize;
  hyd_data* _data;

  cl_platform_id _platform;
  cl_device_id _device;
  cl_context _context;
  cl_command_queue _queue;
  cl_program _program;
  cl_kernel _iso_kernel, _grad_kernel;

  cl_mem _triTable;
  cl_mem _numVertsTable;

  osg::ref_ptr<osg::Group> _group;

  bool init(std::vector<std::string> const& args);

  bool genSurface(float* pVolume, int timestep, int column, float isovalue,
                  int xsize, int ysize, int zsize, osg::Vec3Array* pVertices,
                  osg::Vec3Array* pNormals);

  bool generate(std::vector<std::string> const& args);
  bool remove(std::vector<std::string> const& args);

  bool inserted();
  bool timestep(std::vector<std::string> const& args);
  bool nodemask(std::vector<std::string> const& args);
  bool query();
}; // class hydview

hydview::hydview()
  : iris::Augment("hydview")
  , _running(false)
  , _reply(std::string("/tmp/hydviewFifo-") + std::string(getenv("USER")))
  , _data_shm(-1) {
  setDescription("hydview");
  validate();
} // hydview::hydview

hydview::~hydview() {
  clReleaseMemObject(_triTable);
  clReleaseMemObject(_numVertsTable);

  clReleaseKernel(_iso_kernel);
  clReleaseKernel(_grad_kernel);
  clReleaseProgram(_program);
  clReleaseCommandQueue(_queue);
  clReleaseContext(_context);

  if (_data_shm >= 0) {
    munmap(_data, sizeof(hyd_data) + _dataSize);
    close(_data_shm);
  }
} // hydview::~hydview

bool hydview::control(std::string const& line,
                      std::vector<std::string> const& vec) {
  if (vec.size() == 2 && iris::IsSubstring("init", vec[0], 3)) {
    return init(vec);
  } else if (vec.size() == 7 && iris::IsSubstring("generate", vec[0], 3)) {
    return generate(vec);
  } else if (vec.size() == 2 && iris::IsSubstring("remove", vec[0], 3)) {
    return remove(vec);
  } else if (vec.size() == 1 && iris::IsSubstring("inserted", vec[0], 3)) {
    return inserted();
  } else if (vec.size() == 2 && iris::IsSubstring("timestep", vec[0], 3)) {
    return timestep(vec);
  } else if (vec.size() == 4 && iris::IsSubstring("nodemask", vec[0], 3)) {
    return nodemask(vec);
  } else if (vec.size() == 1 && iris::IsSubstring("query", vec[0], 3)) {
    return query();
  }
  return false;
} // hydview::control

bool hydview::init(std::vector<std::string> const& args) {
  _reply.open(O_WRONLY);

  // open the shared memory segment
  _data_shm = shm_open(args[1].c_str(), O_RDONLY, 0);
  if (_data_shm < 0) {
    dtkMsg.add(DTKMSG_ERROR, "hydview: cannot open shared memory '%s': %s\n",
               args[1].c_str(), strerror(errno));
    _reply.write("cannot open shared memory: " + std::string(strerror(errno)));
    return false;
  }

  // map just the header to get the datasize
  _data = reinterpret_cast<hyd_data*>(
    mmap(NULL, sizeof(hyd_data), PROT_READ, MAP_SHARED, _data_shm, 0));
  if (_data == MAP_FAILED) {
    dtkMsg.add(DTKMSG_ERROR, "hydview: cannot map shared memory: %s\n",
               strerror(errno));
    _reply.write("cannot map shared memory: " + std::string(strerror(errno)));
    return false;
  }

  _dataSize = _data->dataSize;

  // now remap to get the full data
  munmap(_data, sizeof(hyd_data));
  _data = reinterpret_cast<hyd_data*>(mmap(
    NULL, sizeof(hyd_data) + _dataSize, PROT_READ, MAP_SHARED, _data_shm, 0));
  if (_data == MAP_FAILED) {
    dtkMsg.add(DTKMSG_ERROR, "hydview: cannot map shared memory: %s\n",
               strerror(errno));
    _reply.write("cannot map shared memory: " + std::string(strerror(errno)));
    return false;
  }

  // Create platform, device, and context
  cl_int err = clGetPlatformIDs(1, &_platform, NULL);
  if (err != CL_SUCCESS) {
    dtkMsg.add(DTKMSG_ERROR, "hydview: cannot get OpenCL platform: %s\n",
               clstrerror(err));
    _reply.write("cannot get OpenCL platform: " + std::string(clstrerror(err)));
    return false;
  }

  err = clGetDeviceIDs(_platform, CL_DEVICE_TYPE_GPU, 1, &_device,
                       NULL);
  if (err != CL_SUCCESS) {
    dtkMsg.add(DTKMSG_ERROR, "hydview: cannot get OpenCL GPU device: %s\n",
               clstrerror(err));
    _reply.write("cannot get OpenCL GPU device: " +
                 std::string(clstrerror(err)));
    return false;
  }

  _context = clCreateContext(NULL, 1, &_device, NULL, NULL, &err);
  if (err != CL_SUCCESS) {
    dtkMsg.add(DTKMSG_ERROR, "hydview: cannot create OpenCL context: %s\n",
               clstrerror(err));
    _reply.write("cannot create OpenCL context: " +
                 std::string(clstrerror(err)));
    return false;
  }

  // Create a command queue
  cl_command_queue_properties props = CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE;
  _queue = clCreateCommandQueue(_context, _device, props, &err);
  if (err != CL_SUCCESS) {
    dtkMsg.add(DTKMSG_ERROR,
               "hydview: cannot create OpenCL command queue: %s\n",
               clstrerror(err));
    _reply.write("cannot create OpenCL command queue: " +
                 std::string(clstrerror(err)));
    return false;
  }

  // Load and build the program
  std::string fn = std::string(IDEA_PRIV_DATA_DIR) + "isosurface.cl";
  std::vector<char> src = read_program(fn.c_str());
  char const* pSrc = src.data();
  size_t srcSize = src.size();

  _program = clCreateProgramWithSource(_context, 1, &pSrc, &srcSize, &err);
  if (err != CL_SUCCESS) {
    dtkMsg.add(DTKMSG_ERROR, "hydview: cannot create OpenCL program: %s\n",
               clstrerror(err));
    _reply.write("cannot create OpenCL program: " +
                 std::string(clstrerror(err)));
    return false;
  }

  // Build program
  err = clBuildProgram(_program, 0, NULL, NULL, NULL, NULL);
  if (err != CL_SUCCESS) {
    size_t logSize;
    // Find size of log and print to std output
    clGetProgramBuildInfo(_program, _device, CL_PROGRAM_BUILD_LOG,
                          0, NULL, &logSize);

    std::vector<char> log(logSize + 1);
    log[log.size()] = '\0';

    clGetProgramBuildInfo(_program, _device, CL_PROGRAM_BUILD_LOG,
                          logSize + 1, log.data(), NULL);
    dtkMsg.add(DTKMSG_ERROR, "hydview: cannot build OpenCL program: %s\n%s\n",
               clstrerror(err), log.data());
    _reply.write("cannot build OpenCL program: " +
                 std::string(clstrerror(err)));

    return false;
  }

  // Create the kernels
  _iso_kernel = clCreateKernel(_program, "isosurface", &err);
  if (err != CL_SUCCESS) {
    dtkMsg.add(DTKMSG_ERROR, "hydview: cannot create isosurface kernel: %s\n",
               clstrerror(err));
    _reply.write("cannot create isosurface kernel: " +
                 std::string(clstrerror(err)));
    return false;
  }

  _grad_kernel = clCreateKernel(_program, "gradient", &err);
  if (err != CL_SUCCESS) {
    dtkMsg.add(DTKMSG_ERROR, "hydview: cannot create gradient kernel: %s\n",
               clstrerror(err));
    _reply.write("cannot create gradient kernel: " +
                 std::string(clstrerror(err)));
    return false;
  }

  // Allocate the lookup tables as 1D textures (found in "tables.h")
  cl_image_format imageFormat = {CL_R, CL_UNSIGNED_INT8};

  _triTable =
    clCreateImage2D(_context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                    &imageFormat, 16, 256, 0, (void*)triTable, &err);
  if (err != CL_SUCCESS) {
    dtkMsg.add(DTKMSG_ERROR, "hydview: cannot create tri lookup table: %s\n",
               clstrerror(err));
    _reply.write("cannot create tri lookup table: " +
                 std::string(clstrerror(err)));
    return false;
  }

  _numVertsTable =
    clCreateImage2D(_context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                    &imageFormat, 256, 1, 0, (void*)numVertsTable, &err);
  if (err != CL_SUCCESS) {
    dtkMsg.add(DTKMSG_ERROR,
               "hydview: cannot create num verts lookup table: %s\n",
               clstrerror(err));
    _reply.write("cannot create num verts table: " +
                 std::string(clstrerror(err)));
    return false;
  }

  // Init the scenegraph bits
  iris::SceneGraph* sg = iris::SceneGraph::instance();

  _group = new osg::Group;
  _group->setName("hydview");
  _group->setNodeMask(1);

  if (sg->findNode(_group->getName())) {
    sg->removeNode(_group->getName());
  }

  sg->addNode(_group->getName(), _group);
  sg->findNode("world")->asGroup()->addChild(_group);

  dtkMsg.add(DTKMSG_INFO, "hydview initialized");
  _reply.write("");

  _running = true;
  return true;
} // hydview::init

bool hydview::genSurface(float* pVolume, int timestep, int column,
                         float isovalue, int xsize, int ysize, int zsize,
                         osg::Vec3Array* pVertices, osg::Vec3Array* pNormals) {
  osg::Vec3Array& vertices = *pVertices;
  osg::Vec3Array& normals = *pNormals;

  cl_uint const gridSize[4] = {static_cast<cl_uint>(xsize),
                               static_cast<cl_uint>(ysize),
                               static_cast<cl_uint>(zsize), 0};
  uint const numVoxels =
    (gridSize[0] - 1) * (gridSize[1] - 1) * (gridSize[2] - 1);
  uint const nSites = xsize * ysize * zsize;

  // maximum number of allowed vertices (also set is kernels/isosurface.cl)
  uint const maxVerts = (MAX_VERTS_PER_CELL * numVoxels);

  unsigned int const memSize = 4 * sizeof(float) * maxVerts;
  float scale = 1.f / static_cast<float>(std::max(
                        std::max(gridSize[0], gridSize[1]), gridSize[2]));

  size_t const offset[3] = {0, 0, 0};
  size_t const region[3] = {gridSize[0], gridSize[1], gridSize[2]};

  size_t const localSize = NTHREADS;
  size_t globalSize;

  cl_image_format volumeFormat = {CL_R, CL_FLOAT};
  cl_image_format gradientFormat = {CL_RGBA, CL_FLOAT};

  cl_int ec;
  cl_event copy_event, run_event;
  std::vector<cl_event> finish_events(2);

  cl_mem d_volume = 0;
  cl_mem d_grad_array = 0;
  cl_mem d_gradient = 0;
  cl_mem d_vertices = 0;
  cl_mem d_normals = 0;

  std::vector<float> h_vertices(maxVerts * 4), h_normals(maxVerts * 4);
  size_t num_vertices = 0, pos = 0;
  bool ret = false;

  // allocate volume buffer
  d_volume = clCreateImage3D(_context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                             &volumeFormat, gridSize[0], gridSize[1],
                             gridSize[2], 0, 0, pVolume, &ec);
  if (ec != CL_SUCCESS) {
    dtkMsg.add(DTKMSG_ERROR, "hydview: allocating volume buffer failed: %s\n",
               clstrerror(ec));
    goto err;
  }

  // allocate gradient array buffer
  d_grad_array = clCreateBuffer(_context, CL_MEM_READ_WRITE,
                                sizeof(cl_float4) * nSites, 0, &ec);
  if (ec != CL_SUCCESS) {
    dtkMsg.add(DTKMSG_ERROR, "hydview: allocating grad buffer failed: %s\n",
               clstrerror(ec));
    goto err;
  }

  // allocate gradient texture
  d_gradient =
    clCreateImage3D(_context, CL_MEM_READ_ONLY, &gradientFormat, gridSize[0],
                    gridSize[1], gridSize[2], 0, 0, NULL, &ec);
  if (ec) {
    dtkMsg.add(DTKMSG_ERROR,
               "hydview: allocating gradient texture failed: %s\n",
               clstrerror(ec));
    goto err;
  }

  // allocate vertices buffer
  d_vertices = clCreateBuffer(_context, CL_MEM_READ_WRITE, memSize, 0, &ec);
  if (ec != CL_SUCCESS) {
    dtkMsg.add(DTKMSG_ERROR, "hydview: allocating vertices buffer failed: %s\n",
               clstrerror(ec));
    goto err;
  }

  d_normals = clCreateBuffer(_context, CL_MEM_READ_WRITE, memSize, 0, &ec);
  if (ec != CL_SUCCESS) {
    dtkMsg.add(DTKMSG_ERROR, "hydview: allocating normals buffer failed: %s\n",
               clstrerror(ec));
    goto err;
  }

  // CREATE THE GRADIENT ARRAY
  // Create kernel arguments
  ec = clSetKernelArg(_grad_kernel, 0, sizeof(cl_mem), &d_volume);
  ec |= clSetKernelArg(_grad_kernel, 1, sizeof(cl_mem), &d_grad_array);
  ec |= clSetKernelArg(_grad_kernel, 2, 4 * sizeof(cl_uint), gridSize);
  if (ec != CL_SUCCESS) {
    dtkMsg.add(DTKMSG_ERROR, "hydview: setting kernel argument failed: %s\n",
               clstrerror(ec));
    goto err;
  }

  // Enqueue kernel
  globalSize =
    ((gridSize[0] * gridSize[1] * gridSize[2]) / localSize + 1) * localSize;

  ec = clEnqueueNDRangeKernel(_queue, _grad_kernel, 1, NULL, &globalSize,
                               &localSize, 0, NULL, &run_event);
  if (ec != CL_SUCCESS) {
    dtkMsg.add(DTKMSG_ERROR, "hydview: enqueing grad kernel failed: %s\n",
               clstrerror(ec));
    goto err;
  }

  // Copy the gradient buffer into the gradient texture
  ec = clEnqueueCopyBufferToImage(_queue, d_grad_array, d_gradient, 0, offset,
                                  region, 1, &run_event, &copy_event);
  if (ec != CL_SUCCESS) {
    dtkMsg.add(DTKMSG_ERROR, "hydview: copying gradient buffer failed: %s\n",
               clstrerror(ec));
    goto err;
  }

  // CREATE THE MESH
  // Create kernel arguments
  ec = clSetKernelArg(_iso_kernel, 0, sizeof(cl_mem), &d_vertices);
  ec |= clSetKernelArg(_iso_kernel, 1, sizeof(cl_mem), &d_normals);
  ec |= clSetKernelArg(_iso_kernel, 2, sizeof(cl_mem), &d_volume);
  ec |= clSetKernelArg(_iso_kernel, 3, sizeof(cl_mem), &d_gradient);
  ec |= clSetKernelArg(_iso_kernel, 4, 4 * sizeof(cl_uint), gridSize);
  ec |= clSetKernelArg(_iso_kernel, 5, sizeof(uint), &numVoxels);
  ec |= clSetKernelArg(_iso_kernel, 6, sizeof(float), &isovalue);
  ec |= clSetKernelArg(_iso_kernel, 7, sizeof(cl_mem), &_numVertsTable);
  ec |= clSetKernelArg(_iso_kernel, 8, sizeof(cl_mem), &_triTable);
  if (ec != CL_SUCCESS) {
    dtkMsg.add(DTKMSG_ERROR, "hydview: setting kernel argument failed: %s\n",
               clstrerror(ec));
    goto err;
  }

  // Enqueue kernel
  globalSize = (numVoxels / localSize + 1) * localSize;

  ec = clEnqueueNDRangeKernel(_queue, _iso_kernel, 1, NULL, &globalSize,
                               &localSize, 1, &copy_event, &run_event);
  if (ec != CL_SUCCESS) {
    dtkMsg.add(DTKMSG_ERROR, "hydview: enqueing iso kernel failed: %s\n",
               clstrerror(ec));
    goto err;
  }

  // Read the vertices array
  ec = clEnqueueReadBuffer(_queue, d_vertices, CL_FALSE, 0, memSize,
                           h_vertices.data(), 1, &run_event,
                           &finish_events[0]);
  if (ec != CL_SUCCESS) {
    dtkMsg.add(DTKMSG_ERROR, "hydview: reading vertices buffer failed: %s\n",
               clstrerror(ec));
    goto err;
  }

  // Read the normals array
  ec = clEnqueueReadBuffer(_queue, d_normals, CL_FALSE, 0, memSize,
                           h_normals.data(), 1, &run_event, &finish_events[1]);
  if (ec != CL_SUCCESS) {
    dtkMsg.add(DTKMSG_ERROR, "hydview: reading normals buffer failed: %s\n",
               clstrerror(ec));
    goto err;
  }

  // Wait for the reads to finish
  ec = clWaitForEvents(static_cast<cl_uint>(finish_events.size()),
                       finish_events.data());
  if (ec != CL_SUCCESS) {
    dtkMsg.add(DTKMSG_ERROR, "hydview: reading buffers failed: %s\n",
               clstrerror(ec));
    goto err;
  }

  // compact the mesh vertices
  for (int i = 0; i < maxVerts * 4; i += 4) {
    if (h_vertices[i + 3] > 0.0) {
      num_vertices += 1;
    }
  }

  vertices.resize(num_vertices);
  normals.resize(num_vertices);

  for (int i = 0; i < maxVerts * 4; i += 4) {
    if (h_vertices[i + 3] > 0.0) {
      vertices[pos] =
        osg::Vec3(h_vertices[i], h_vertices[i + 1], h_vertices[i + 2]);
      normals[pos] =
        osg::Vec3(-h_normals[i], -h_normals[i + 1], -h_normals[i + 2]);
      pos += 1;
    }
  }

  ret = true;
err:
  // Deallocate resources
  if (d_volume) clReleaseMemObject(d_volume);
  if (d_gradient) clReleaseMemObject(d_gradient);
  if (d_grad_array) clReleaseMemObject(d_grad_array);
  if (d_vertices) clReleaseMemObject(d_vertices);
  if (d_normals) clReleaseMemObject(d_normals);

  return ret;
} // hydview::genSurface

bool hydview::generate(std::vector<std::string> const& args) {
  if (!_running) return false;

  // parse the control command arguments
  int timestep, column;
  float iso, red, green, blue;

  if (!iris::StringToInt(args[1], &timestep)) {
    dtkMsg.add(DTKMSG_ERROR, "hydview: invalid timestep value '%s'\n",
               args[1].c_str());
    return false;
  } else if (timestep >= _data->nTimesteps) {
    dtkMsg.add(DTKMSG_ERROR, "hydview: invalid timestep value '%s' (too big)\n",
               args[1].c_str());
    return false;
  }

  if (!iris::StringToInt(args[2], &column)) {
    dtkMsg.add(DTKMSG_ERROR, "hydview: invalid column value '%s'\n",
               args[2].c_str());
    return false;
  }

  if (!iris::StringToFloat(args[3], &iso)) {
    dtkMsg.add(DTKMSG_ERROR, "hydview: invalid iso value '%s'\n",
               args[3].c_str());
    return false;
  }

  if (!iris::StringToFloat(args[4], &red)) {
    dtkMsg.add(DTKMSG_ERROR, "hydview: invalid red value '%s'\n",
               args[4].c_str());
    return false;
  }

  if (!iris::StringToFloat(args[5], &green)) {
    dtkMsg.add(DTKMSG_ERROR, "hydview: invalid green value '%s'\n",
               args[5].c_str());
    return false;
  }

  if (!iris::StringToFloat(args[6], &blue)) {
    dtkMsg.add(DTKMSG_ERROR, "hydview: invalid blue value '%s'\n",
               args[6].c_str());
    return false;
  }

  int const nTimesteps = _data->nTimesteps;
  int const nSites = _data->nSites;
  int const nItemsPerSite = _data->nItemsPerSite;
  int const dimX = _data->dimX;
  int const dimY = _data->dimY;
  int const dimZ = _data->dimZ;
  int const cind = column;
  float const isov = iso;

  std::vector<osg::Vec3Array*> ts_vertices(nTimesteps);
  std::vector<osg::Vec3Array*> ts_normals(nTimesteps);
  std::vector<osg::ref_ptr<osg::Geode> > nodes(nTimesteps);

  osg::Vec4 const color(red, green, blue, 1.0);

  for (int i = 0; i < nTimesteps; ++i) {
    ts_vertices[i] = new osg::Vec3Array;
    ts_normals[i] = new osg::Vec3Array;
    nodes[i] = new osg::Geode;

    osg::ref_ptr<osg::Geode>& node = nodes[i];

    char nodeName[64];
    snprintf(nodeName, 64, "isosurface_%d_%d", i, cind);

    node->setName(nodeName);

    node->getOrCreateStateSet()->setAttributeAndModes(
      new osg::PolygonMode(osg::PolygonMode::FRONT_AND_BACK,
                           osg::PolygonMode::FILL),
      osg::StateAttribute::ON);

    node->setNodeMask((i == timestep));
  }

  bool abort = false;

  //#pragma omp parallel for default(none) shared(ts_vertices, ts_normals, nodes, dtkMsg, abort)
  for (int i = 0; i < nTimesteps; ++i) {
    osg::Vec3Array* verts = ts_vertices[i];
    osg::Vec3Array* norms = ts_normals[i];
    osg::ref_ptr<osg::Geode>* node = &nodes[i];

    float* src;
    if (cind < nItemsPerSite) {
      src = _data->data + (i * nItemsPerSite * nSites) + (cind * nSites);
    } else {
      src = _data->data + (nTimesteps - 1) * nItemsPerSite * nSites +
            (cind - nItemsPerSite) * nTimesteps * nSites + i * nSites;
    }

    #pragma omp flush(abort)
    if (!abort) {
      bool ok = genSurface(src, i, cind, isov, dimX, dimY, dimZ, verts, norms);
      if (!ok) {
        dtkMsg.add(DTKMSG_ERROR,
                   "hydview: error generating isosurface for timestep %d\n", i);
        abort = true;
        #pragma omp flush(abort)
      }
    }

    osg::ref_ptr<osg::Geometry> d = new osg::Geometry;
    d->setVertexArray(verts);
    d->setNormalArray(norms, osg::Array::BIND_PER_VERTEX);
    d->setColorArray(new osg::Vec4Array(1, &color), osg::Array::BIND_OVERALL);
    d->addPrimitiveSet(
      new osg::DrawArrays(osg::PrimitiveSet::TRIANGLES, 0, verts->size()));
    d->setUseVertexBufferObjects(true);

    (*node)->addDrawable(d);
  }

  if (abort) {
    for (int i = 0; i < nTimesteps; ++i) {
      delete &ts_vertices[i];
      delete &ts_normals[i];
    }
    return false;
  }

  iris::SceneGraph* sg = iris::SceneGraph::instance();

  for (int i = 0; i < nTimesteps; ++i) {
    osg::ref_ptr<osg::Geode>& node = nodes[i];

    osg::Node* existingNode = sg->findNode(node->getName());
    if (existingNode) {
      _group->removeChild(_group->getChildIndex(existingNode));
      sg->removeNode(node->getName());
    }

    sg->addNode(node->getName(), node);
    _group->addChild(node);
  }

  return true;
} // hydview::generate

bool hydview::remove(std::vector<std::string> const& args) {
  if (!_running) return false;

  int column;

  if (!iris::StringToInt(args[1], &column)) {
    dtkMsg.add(DTKMSG_ERROR, "hydview: invalid column value '%s'\n",
               args[1].c_str());
    return false;
  }

  iris::SceneGraph* sg = iris::SceneGraph::instance();

  char szMatch[64];
  snprintf(szMatch, 64, "isosurface_[0-9]+_%d", column);

  regex_t re;
  regcomp(&re, szMatch, REG_EXTENDED | REG_NOSUB);

  std::vector<osg::Node*> nodesToRemove;

  int const nChildren = _group->getNumChildren();
  for (int i = 0; i < nChildren; ++i) {
    osg::Node* node = _group->getChild(i);
    if (regexec(&re, node->getName().c_str(), 0, NULL, 0) == 0) {
      nodesToRemove.push_back(node);
    }
  }

  for (int i = 0; i < nodesToRemove.size(); ++i) {
    osg::Node* node = nodesToRemove[i];
    _group->removeChild(_group->getChildIndex(node));
    sg->removeNode(node->getName());
  }

  return true;
} // hydview::remove

bool hydview::inserted() {
  if (!_running) return false;

  // unmap the shared memory segment
  if (munmap(_data, sizeof(hyd_data) + _dataSize) == -1) {
    dtkMsg.add(DTKMSG_ERROR, "hydview: cannot map shared memory: %s\n",
               strerror(errno));
    return false;
  }

  // map just the header to get the datasize
  _data = reinterpret_cast<hyd_data*>(
    mmap(NULL, sizeof(hyd_data), PROT_READ, MAP_SHARED, _data_shm, 0));
  if (_data == MAP_FAILED) {
    dtkMsg.add(DTKMSG_ERROR, "hydview: cannot map shared memory: %s\n",
               strerror(errno));
    return false;
  }

  _dataSize = _data->dataSize;

  // now remap to get the full data
  munmap(_data, sizeof(hyd_data));
  _data = reinterpret_cast<hyd_data*>(mmap(
    NULL, sizeof(hyd_data) + _dataSize, PROT_READ, MAP_SHARED, _data_shm, 0));
  if (_data == MAP_FAILED) {
    dtkMsg.add(DTKMSG_ERROR, "hydview: cannot map shared memory: %s\n",
               strerror(errno));
    return false;
  }

  return true;
} // hydview::inserted

bool hydview::timestep(std::vector<std::string> const& args) {
  if (!_running) return false;

  int timestep;

  if (!iris::StringToInt(args[1], &timestep)) {
    dtkMsg.add(DTKMSG_ERROR, "hydview: invalid timestep value '%s'\n",
               args[1].c_str());
    return false;
  } else if (timestep >= _data->nTimesteps) {
    dtkMsg.add(DTKMSG_ERROR, "hydview: invalid timestep value '%s' (too big)\n",
               args[1].c_str());
    return false;
  }

  char szMatch[64];
  snprintf(szMatch, 64, "isosurface_%d_[0-9]+", timestep);

  regex_t re;
  regcomp(&re, szMatch, REG_EXTENDED | REG_NOSUB);

  int const nChildren = _group->getNumChildren();
  for (int i = 0; i < nChildren; ++i) {
    osg::Node* node = _group->getChild(i);
    if (regexec(&re, node->getName().c_str(), 0, NULL, 0) == 0) {
      node->setNodeMask(1);
    } else {
      node->setNodeMask(0);
    }
  }

  return true;
} // hydview::timestep

bool hydview::nodemask(std::vector<std::string> const& args) {
  if (!_running) return false;

  int timestep;
  int column;
  bool onOff;

  if (!iris::StringToInt(args[1], &timestep)) {
    dtkMsg.add(DTKMSG_ERROR, "hydview: invalid column value '%s'\n",
               args[1].c_str());
    return false;
  } else if (timestep >= _data->nTimesteps) {
    dtkMsg.add(DTKMSG_ERROR, "hydview: invalid timestep value '%s' (too big)\n",
               args[1].c_str());
    return false;
  }
  if (!iris::StringToInt(args[2], &column)) {
    dtkMsg.add(DTKMSG_ERROR, "hydview: invalid column value '%s'\n",
               args[2].c_str());
    return false;
  }
  if (!iris::OnOff(args[3], &onOff)) {
    dtkMsg.add(DTKMSG_ERROR, "hydview: invalid on/off value '%s'\n",
               args[3].c_str());
    return false;
  }

  char szMatch[64];
  snprintf(szMatch, 64, "isosurface_%d_%d", timestep, column);

  regex_t re;
  regcomp(&re, szMatch, REG_EXTENDED | REG_NOSUB);

  int const nChildren = _group->getNumChildren();
  for (int i = 0; i < nChildren; ++i) {
    osg::Node* node = _group->getChild(i);
    if (regexec(&re, node->getName().c_str(), 0, NULL, 0) == 0) {
      dtkMsg.add(DTKMSG_ERROR, "%s setNodeMask %d\n", node->getName().c_str(),
                 (onOff ? 1 : 0));
      node->setNodeMask(onOff ? 1 : 0);
      break;
    }
  }

  return true;
} // hydview::nodemask

bool hydview::query() {
  int const nChildren = _group->getNumChildren();
  for (int i = 0; i < nChildren; ++i) {
    osg::Node* node = _group->getChild(i);
    dtkMsg.add(DTKMSG_ERROR, "node %s nodemask: %d\n", node->getName().c_str(),
               node->getNodeMask());
  }
  return true;
} // hydview::query

} // namespace

static dtkAugment* dtkDSO_loader(dtkManager*, void*) {
  return new hydview;
}

static int dtkDSO_unloader(dtkAugment*) {
  return DTKDSO_UNLOAD_CONTINUE;
}

