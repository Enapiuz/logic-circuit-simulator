struct NandChip {
    bool A;
    bool B;
    bool Out;
};

struct ChipWire {
    unsigned int from;
    unsigned int to;
    bool direction; // false A, true B
};

// Places (A nand B) to C.
// A - first input
// B - second input
// C - result
__kernel void nand(__global struct NandChip *chips) {
    int i = get_global_id(0);

    chips[i].Out = !(chips[i].A && chips[i].B);
}

// A - first input
// B - second input
// C - calculated result
// fromAddr - from which C copy value
// direction - to which input copy value (false - A, true - B)
// toAddr - to which A or B copy value
__kernel void wire(__global struct NandChip *chips, __global const struct ChipWire *targets) {
    int i = get_global_id(0);

    // to A
    if (targets[i].direction == false) {
        chips[targets[i].to].A = chips[targets[i].from].Out;
    } else { // to B
        chips[targets[i].to].B = chips[targets[i].from].Out;
    }
}