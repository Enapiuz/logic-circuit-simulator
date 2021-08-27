#include "cl.h"
#include <stdbool.h>

#define DIRECTION_A false;
#define DIRECTION_B true;

typedef struct OpenCLBaseResources {
    cl_device_id device_id;
    cl_context context;
    cl_command_queue queue;
} OpenCLBaseResources;

typedef struct NandChip {
    bool A;
    bool B;
    bool Out;
} NandChip;

typedef struct ChipWire {
    unsigned int from;
    unsigned int to;
    bool direction; // false A, true B
} ChipWire;

void print_versions(void);
cl_device_id get_device_id(int type);

// Base
OpenCLBaseResources *init_opencl_base(void);
void free_opencl_base(OpenCLBaseResources *base);

// Buffers
cl_mem create_nand_chip_buffer(OpenCLBaseResources *base, unsigned int size);
void free_nand_chip_buffers(cl_mem buffer);
cl_mem create_wire_buffer(OpenCLBaseResources *base, unsigned int size);
void free_wire_buffer(cl_mem buffer);

// Host data arrays
NandChip *create_nand_chip_array(unsigned int size);
void free_nand_chip_array(NandChip *array);
ChipWire *create_wire_array(unsigned int size);
void free_wire_array(ChipWire *array);
