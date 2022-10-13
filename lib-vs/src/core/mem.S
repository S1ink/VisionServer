.global memcpy_threshold_asm
.global memcpy_threshold_binary_asm
.global memcpy_compare3_add_asm
.global memcpy_bitwise_or_asm
.global memcpy_subtract_asm


memcpy_threshold_asm:
    push {fp}
    add fp, sp, #0  // Push new Stack Frame

    // Arg 0: r0: Destination Address
    // Arg 1: r1: Source Address
    // Arg 2: r2: Count
    // Arg 3: r3: Threshold Minimum

    lsr r2, #4       // Shift count right by 4 bits (divide by 16, 16 bytes = 128 bits)
    vdup.8 q3, r3    // Write threshold to q3
    _thresh_loop:
        // Load into Q0
        vld1.64 d0, [r1]!
        vld1.64 d1, [r1]!

        vcgt.u8 q1, q0, q3  // Will set all to 1 if Greater
        vand.u8 q2, q1, q0  // AND the new set, result is in q2

        // Load out of Q2
        vst1.64 d4, [r0]!
        vst1.64 d5, [r0]!

        sub r2, r2, #1
        cmp r2, #0
        bgt _thresh_loop

    sub sp, fp, #0  // Pop our Stack Frame
    pop {fp}
    bx lr

memcpy_threshold_binary_asm:
    push {fp}
    add fp, sp, #0  // Push new Stack Frame

    // Arg 0: r0: Destination Address
    // Arg 1: r1: Source Address
    // Arg 2: r2: Count
    // Arg 3: r3: Threshold Minimum

    lsr r2, #4       // Shift count right by 4 bits (divide by 16, 16 bytes = 128 bits)
    vdup.8 q3, r3    // Write threshold to q3
    _thresh_bin_loop:
        // Load into Q0
        vld1.64 d0, [r1]!
        vld1.64 d1, [r1]!

        vcgt.u8 q1, q0, q3  // Will set all to 1 if Greater

        // Load out of Q2
        vst1.64 d2, [r0]!
        vst1.64 d3, [r0]!

        sub r2, r2, #1
        cmp r2, #0
        bgt _thresh_bin_loop

    sub sp, fp, #0  // Pop our Stack Frame
    pop {fp}
    bx lr

memcpy_compare3_add_asm:
    push {fp}
    add fp, sp, #0  // Push new Stack Frame

    // Arg 0: r0: Primary channel address
    // Arg 1: r1: Compare channel 1 address
    // Arg 2: r2: Compare channel 2 address
    // Arg 3: r3: Addition address
    // Arg 4: r4: Destination Address
    // Arg 5: r5: Count

    lsr r5, #4       // Shift count right by 4 bits (divide by 16, 16 bytes = 128 bits)
    _cmp3_add_loop:
        // load primary compare
        vld1.64 d0, [r0]!   // Q0
        vld1.64 d1, [r0]!
        // load compare 1
        vld1.64 d2, [r1]!   // Q1
        vld1.64 d3, [r1]!
        // load compare 2
        vld1.64 d4, [r2]!   // Q2
        vld1.64 d5, [r2]!
        // load addition
        vld1.64 d6, [r3]!   // Q3
        vld1.64 d7, [r3]!

        vcgt.u8 q4, q0, q1  // compare primary and channel 1
        vcgt.u8 q5, q0, q2  // compare primary and channel 2
        vand.u8 q6, q4, q5  // AND the results
//        vand.u8 q1, q0, q1  // AND the threshold to the input
        vqadd.u8 q4, q6, q3  // add the threshold to the additive

        // Load out of Q4
        vst1.64 d8, [r4]!
        vst1.64 d9, [r4]!

        sub r5, r5, #1
        cmp r5, #0
        bgt _cmp3_add_loop

    sub sp, fp, #0  // Pop our Stack Frame
    pop {fp}
    bx lr

memcpy_bitwise_or_asm:
    push {fp}
    add fp, sp, #0  // Push new Stack Frame

    // Arg 0: r0: Source A
    // Arg 1: r1: Source B
    // Arg 2: r2: Dest
    // Arg 3: r3: Count

    lsr r3, #4       // Shift count right by 4 bits (divide by 16, 16 bytes = 128 bits)
    _bitwise_or_loop:
        // Load into Q0
        vld1.64 d0, [r0]!
        vld1.64 d1, [r0]!
        // Load into Q1
        vld1.64 d2, [r1]!
        vld1.64 d3, [r1]!

        vorr.u8 q2, q0, q1

        // Load out of Q2
        vst1.64 d4, [r2]!
        vst1.64 d5, [r2]!

        sub r3, r3, #1
        cmp r3, #0
        bgt _bitwise_or_loop

    sub sp, fp, #0  // Pop our Stack Frame
    pop {fp}
    bx lr


memcpy_subtract_asm:
    push {fp}
    add fp, sp, #0

    // Arg 0: r0: Address of base array that is being subtracted
    // Arg 1: r1: Address of second array that is being subtracted
    // Arg 2: r2: Address of destination array
    // Arg 3: r3: Size of arrays (width * height)

    lsr r3, #4       // get iterations by dividing size by 16
    _subtract_loop:
        // load base
        vld1.64 d0, [r0]!
        vld1.64 d1, [r0]!

        // load subtractor
        vld1.64 d2, [r1]!
        vld1.64 d3, [r1]!

        // subtract
        vsub.u8 q2, q0, q1

        // load Q2 to destination
        vst1.64 d4, [r2]!
        vst1.64 d5, [r2]!

        sub r3, r3, #1
        cmp r3, #0
        bgt _subtract_loop

    sub sp, fp, #0  // Pop our Stack Frame
    pop {fp}
    bx lr