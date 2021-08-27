#include "cl.h"
#include "executor.h"
#include <stdio.h>
#include <stdlib.h>

#define MAX_SOURCE_SIZE (0x100000)

int main(void) {
    // Print debug info
    puts("Detected devices:");
    print_versions();
    puts("");

    // Parse ehdl files

    // Init OpenCL environment
    OpenCLBaseResources *base = init_opencl_base();

    // Load device programm
    // TODO - embed it
    FILE *fp;
    char *source_str;
    size_t source_size;
    fp = fopen("src/cl/nand.cl", "r");
    if (!fp) {
        fprintf(stderr, "Failed to load kernel.\n");
        exit(1);
    }
    source_str = (char *)malloc(MAX_SOURCE_SIZE);
    source_size = fread(source_str, 1, MAX_SOURCE_SIZE, fp);
    fclose(fp);

    // Compile device program
    cl_int ret;
    cl_program program = clCreateProgramWithSource(base->context, 1, (const char **)&source_str, (const size_t *)&source_size, &ret);
    ret = clBuildProgram(program, 1, &base->device_id, NULL, NULL, NULL);

    // Alocate buffers on device
    cl_mem nandBuffers = create_nand_chip_buffer(base, 2);
    cl_mem wireBuffers = create_wire_buffer(base, 4);

    // Create host data
    NandChip *chips = create_nand_chip_array(2);
    ChipWire *wires = create_wire_array(4);

    // Connect wires
    wires[0].from = 0;
    wires[0].to = 1;
    wires[0].direction = DIRECTION_A;
    wires[1].from = 0;
    wires[1].to = 1;
    wires[1].direction = DIRECTION_B;
    wires[2].from = 1;
    wires[2].to = 0;
    wires[2].direction = DIRECTION_A;
    wires[3].from = 1;
    wires[3].to = 0;
    wires[3].direction = DIRECTION_B;

    // Upload host data to device
    ret = clEnqueueWriteBuffer(base->queue, nandBuffers, CL_TRUE, 0, 2 * sizeof(NandChip), chips, 0, NULL, NULL);
    ret = clEnqueueWriteBuffer(base->queue, wireBuffers, CL_TRUE, 0, 4 * sizeof(ChipWire), wires, 0, NULL, NULL);

    // Create kernels
    cl_kernel nandKernel = clCreateKernel(program, "nand", &ret);
    cl_kernel wireKernel = clCreateKernel(program, "wire", &ret);

    // Assign kernel arguments
    ret = clSetKernelArg(nandKernel, 0, sizeof(cl_mem), (void *)&nandBuffers);
    ret = clSetKernelArg(wireKernel, 0, sizeof(cl_mem), (void *)&nandBuffers);
    ret = clSetKernelArg(wireKernel, 1, sizeof(cl_mem), (void *)&wireBuffers);

    const int MAX_CYCLES = 3;
    for (int cnt = 0; cnt < MAX_CYCLES; cnt++) {
        // Execute nand chips
        size_t global_item_size = 2; // Process the entire lists
        size_t local_item_size = 1;  // Divide work items into groups of 64
        ret = clEnqueueNDRangeKernel(base->queue, nandKernel, 1, NULL, &global_item_size, &local_item_size, 0, NULL, NULL);
        ret = clEnqueueNDRangeKernel(base->queue, nandKernel, 1, NULL, &global_item_size, &local_item_size, 0, NULL, NULL);

        // Execute wires
        global_item_size = 4;
        local_item_size = 1;
        ret = clEnqueueNDRangeKernel(base->queue, wireKernel, 1, NULL, &global_item_size, &local_item_size, 0, NULL, NULL);
    }

    // Download results from device
    NandChip *result = (NandChip *)malloc(sizeof(NandChip) * 2);
    ret = clEnqueueReadBuffer(base->queue, nandBuffers, CL_TRUE, 0, 2 * sizeof(NandChip), result, 0, NULL, NULL);

    // Print result
    for (int i = 0; i < 2; i++) {
        printf("A: %d, B: %d, Out: %d\n", result[i].A, result[i].B, result[i].Out);
    }

    // Free OpenCL environment
    free(result);
    free(source_str);
    free_nand_chip_buffers(nandBuffers);
    free_wire_buffer(wireBuffers);
    free_opencl_base(base);
}
