#include "executor.h"
#include "cl.h"
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Prints versions of all OpenCL capable devices to stdout.
void print_versions(void) {
    cl_uint num_devices, i;
    clGetDeviceIDs(NULL, CL_DEVICE_TYPE_ALL, 0, NULL, &num_devices);

    cl_device_id *devices = calloc(sizeof(cl_device_id), num_devices);
    clGetDeviceIDs(NULL, CL_DEVICE_TYPE_ALL, num_devices, devices, NULL);

    char buf[128];
    for (i = 0; i < num_devices; i++) {
        clGetDeviceInfo(devices[i], CL_DEVICE_NAME, 128, buf, NULL);
        fprintf(stdout, "[%d] Device %s supports ", i, buf);

        clGetDeviceInfo(devices[i], CL_DEVICE_VERSION, 128, buf, NULL);
        fprintf(stdout, "%s\n", buf);
    }

    free(devices);
}

// Initializes all base structures for OpenCL.
OpenCLBaseResources *init_opencl_base(void) {
    OpenCLBaseResources *r = (OpenCLBaseResources *)malloc(sizeof(OpenCLBaseResources));
    cl_int ret;
    r->device_id = get_device_id(0);
    r->context = clCreateContext(NULL, 1, &r->device_id, NULL, NULL, &ret);
    r->queue = clCreateCommandQueue(r->context, r->device_id, 0, &ret);
    return r;
}

void free_opencl_base(OpenCLBaseResources *base) {
    // free(&base->device_id);
    // free(&base->context);
    // free(&base->queue);
    clReleaseCommandQueue(base->queue);
    clReleaseContext(base->context);
    free(base);
}

// Creates buffer of nand chips for OpenCL
cl_mem create_nand_chip_buffer(OpenCLBaseResources *base, unsigned int size) {
    cl_int ret;
    cl_mem nandChipBuffer = clCreateBuffer(base->context, CL_MEM_READ_WRITE, size * sizeof(NandChip), NULL, &ret);
    return nandChipBuffer;
}

void free_nand_chip_buffers(cl_mem buffer) { clReleaseMemObject(buffer); }

// Creates wire buffer for OpenCL
cl_mem create_wire_buffer(OpenCLBaseResources *base, unsigned int size) {
    cl_int ret;
    cl_mem wireBuffer = clCreateBuffer(base->context, CL_MEM_READ_ONLY, size * sizeof(ChipWire), NULL, &ret);
    return wireBuffer;
}

void free_wire_buffer(cl_mem buffer) { clReleaseMemObject(buffer); }

// Creates initial nand array to pass data to OpenCL
NandChip *create_nand_chip_array(unsigned int size) {
    NandChip *arr = (NandChip *)malloc(size * sizeof(NandChip));
    for (int i = 0; i < size; i++) {
        arr[i].A = false;
        arr[i].B = false;
        arr[i].Out = false;
    }
    return arr;
}

void free_nand_chip_array(NandChip *array) { free(array); }

ChipWire *create_wire_array(unsigned int size) {
    ChipWire *arr = (ChipWire *)malloc(size * sizeof(ChipWire));
    return arr;
}

void free_wire_array(ChipWire *array) { free(array); }

cl_device_id get_device_id(int type) {
    if (type == 1) {
        type = CL_DEVICE_TYPE_GPU;
    } else {
        type = CL_DEVICE_TYPE_CPU;
    }
    cl_uint num_devices, i;
    clGetDeviceIDs(NULL, type, 0, NULL, &num_devices);
    cl_device_id *devices = calloc(sizeof(cl_device_id), num_devices);
    clGetDeviceIDs(NULL, type, num_devices, devices, NULL);
    char buf[128];
    printf("Num devices: %d\n", num_devices);
    for (i = 0; i < num_devices; i++) {
        clGetDeviceInfo(devices[i], CL_DEVICE_NAME, 128, buf, NULL);
        if (type == CL_DEVICE_TYPE_CPU) {
            printf("Selected device: %s\n", buf);
            return devices[i];
        }
        for (int cnt = 0; buf[cnt]; cnt++) {
            buf[cnt] = tolower(buf[cnt]);
        }
        // Trying to get the most powerful GPU
        if (type == CL_DEVICE_TYPE_GPU) {
            if (strstr((const char *)buf, "amd") != NULL) {
                printf("Selected device: %s\n", buf);
                return devices[i];
            }
            if (strstr((const char *)buf, "nvidia") != NULL) {
                printf("Selected device: %s\n", buf);
                return devices[i];
            }
        }
    }
    // TODO: return something if no nvidia/amd gpu found
    return NULL;
}
