#define CL_TARGET_OPENCL_VERSION 300
#ifdef __APPLE__
#include <CL/opencl.h>
#else
#include <CL/cl.h>
#endif

#define PRINT_DEBUG