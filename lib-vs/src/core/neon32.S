.global memcpy_threshold_asm
.global memcpy_threshold_binary_asm
.global memcpy_compare3_add_asm     // a bit broken
.global memcpy_wst_asm
.global memcpy_split_wst_asm

.global memcpy_bitwise_or_asm
.global memcpy_subtract_asm



// Arg 0: r0: Destination Address
// Arg 1: r1: Source Address
// Arg 2: r2: Count
// Arg 3: r3: Threshold Minimum
memcpy_threshold_asm:
    push {fp}
    add fp, sp, #0  // Push new Stack Frame

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


// Arg 0: r0: Destination Address
// Arg 1: r1: Source Address
// Arg 2: r2: Count
// Arg 3: r3: Threshold Minimum
memcpy_threshold_binary_asm:
    push {fp}
    add fp, sp, #0  // Push new Stack Frame

    lsr r2, #4       // Shift count right by 4 bits (divide by 16, 16 bytes = 128 bits)
    vdup.8 q3, r3    // Write threshold to q3
    _thresh_bin_loop:
        // Load into Q0
        vld1.64 d0, [r1]!
        vld1.64 d1, [r1]!

        vcgt.u8 q1, q0, q3  // Will set all to 1 if Greater

        // Load out of Q1
        vst1.64 d2, [r0]!
        vst1.64 d3, [r0]!

        sub r2, r2, #1
        cmp r2, #0
        bgt _thresh_bin_loop

    sub sp, fp, #0  // Pop our Stack Frame
    pop {fp}
    bx lr


// Arg 0: r0: Primary channel address
// Arg 1: r1: Compare channel 1 address
// Arg 2: r2: Compare channel 2 address
// Arg 3: r3: Addition address
// Arg 4: r4: Destination Address
// Arg 5: r5: Count
memcpy_compare3_add_asm:
    push {r4, r5, fp}
    add fp, sp, #0  // Push new Stack Frame

    ldr r4, [sp, #12]    // load 5th and 6th args from stack
    ldr r5, [sp, #16]

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

        vcgt.u8 q1, q0, q1  // compare primary and channel 1
        vcgt.u8 q2, q0, q2  // compare primary and channel 2
        vand.u8 q1, q1, q2  // AND the results
        vand.u8 q2, q0, q1  // AND the threshold to the input
        vshr.u8 q1, q2, #6  // rshift result by 6 (div 32)

        vneg.s8 q1, q1      // invert sign
        vshl.s8 q2, q0, q1  // left shift primary by negated result (right shift, mimics a division)
        vshl.u8 q2, q2, #4  // left shift result to fill all 8 bits
        vqadd.u8 q3, q2, q3  // add the threshold to the additive

        // Load out of Q3
        vst1.64 d6, [r4]!
        vst1.64 d7, [r4]!

        sub r5, r5, #1
        cmp r5, #0
        bgt _cmp3_add_loop

    sub sp, fp, #0  // Pop our Stack Frame
    pop {r4, r5, fp}
    bx lr


// Arg 0: r0: Primary channel address
// Arg 1: r1: Compare channel 2 address
// Arg 2: r2: Compare channel 3 address
// Arg 3: r3: Destination address
// Arg 4: r4: Count
// Arg 5: r5: Alpha
// Arg 6: r6: Beta
// Arg 7: r7: Gamma
memcpy_wst_asm:
    push {r4-r7, fp}
    add fp, sp, #0  // Push new Stack Frame

    ldr r4, [sp, #(0+20)]    // load 5th - 8th args from stack
    ldr r5, [sp, #(4+20)]
    ldr r6, [sp, #(8+20)]
    ldr r7, [sp, #(12+20)]

    vdup.8 q8, r5   // duplicate alpha across 16x8bit chunks
    vclz.u8 q8, q8  // count the leading 0's (inverse of log2(value))
    vneg.s8 q8, q8  // negate count (for right shifting)

    vdup.8 q9, r6   // ''' for beta
    vclz.u8 q9, q9
    vneg.s8 q9, q9

    vdup.8 q10, r7  // duplicate gamma across 16x8bit chunks

    lsr r4, #4       // right shift the count by 4 bits (divide by 16, 16 bytes = 128 bits)
    _wst_loop:
        // load primary channel
        vld1.64 d0, [r0]!   // Q0
        vld1.64 d1, [r0]!
        // load channel 2
        vld1.64 d2, [r1]!   // Q1
        vld1.64 d3, [r1]!
        // load channel 3
        vld1.64 d4, [r2]!   // Q2
        vld1.64 d5, [r2]!

        vshl.u8 q1, q1, q8      // weight q1 --> q1
        vshl.u8 q2, q2, q9      // weight q2 --> q2
        vqadd.u8 q3, q1, q2     // add weights
        vqadd.u8 q3, q3, q10    // add gamma

        vqsub.u8 q3, q0, q3     // subtract result from primary

        // Load out of Q3
        vst1.64 d6, [r3]!
        vst1.64 d7, [r3]!

        sub r4, r4, #1
        cmp r4, #0
        bgt _wst_loop

    sub sp, fp, #0  // Pop our Stack Frame
    pop {r4-r7, fp}
    bx lr


// Arg 0: r0: Source address (3-channel continuous buffer required)
// Arg 1: r1: Destination address
// Arg 2: r2: Count (frame size) --> NOT(size * #channels)
// Arg 3: r3: C1/Primary channel offset(index) --> ex. 0 for first; alpha/beta follow
// Arg 4: r4: Alpha
// Arg 5: r5: Beta
// Arg 6: r6: Gamma
memcpy_split_wst_asm:
    push {r4-r6, fp}
    add fp, sp, #0  // Push new Stack Frame

    ldr r4, [sp, #(0+16)]    // load 5th - 7th args from stack
    ldr r5, [sp, #(4+16)]
    ldr r6, [sp, #(8+16)]

    vdup.8 q8, r4   // duplicate alpha across 16x8bit chunks
    vclz.u8 q8, q8  // count the leading 0's (inverse of log2(value))
    vneg.s8 q8, q8  // negate count (for right shifting)

    vdup.8 q9, r5   // ''' for beta
    vclz.u8 q9, q9
    vneg.s8 q9, q9

    vdup.8 q10, r6  // duplicate gamma across 16x8bit chunks

    lsr r2, #4       // right shift the count by 4 bits (divide by 16, 16 bytes = 128 bits)
    
    cmp r3, #1
    blt _c0_split_wst_loop
    beq _c1_split_wst_loop
    bgt _c2_split_wst_loop

    _c0_split_wst_loop:
        //ld3 { v0.16b, v1.16b, v2.16b }, [r0]!
        vld3.8 { d0, d2, d4 }, [r0]!
        vld3.8 { d1, d3, d5 }, [r0]!

        // Q0 --> primary, Q1 --> alpha, Q2 --> beta
        vshl.u8 q1, q1, q8      // weight alpha
        vshl.u8 q2, q2, q9      // weight beta
        vqadd.u8 q3, q1, q2     // add weights
        vqadd.u8 q3, q3, q10    // add gamma

        vqsub.u8 q3, q0, q3     // subtract result from primary

        // Load out of Q3
        vst1.64 { d6, d7 }, [r1]!

        sub r2, r2, #1
        cmp r2, #0
        bgt _c0_split_wst_loop
        ble _split_wst_end

    _c1_split_wst_loop:
        vld3.8 { d0, d2, d4 }, [r0]!
        vld3.8 { d1, d3, d5 }, [r0]!

        // Q0 --> beta, Q1 --> primary, Q2 --> alpha
        vshl.u8 q2, q2, q8      // weight alpha
        vshl.u8 q0, q0, q9      // weight beta
        vqadd.u8 q3, q2, q0     // add weights
        vqadd.u8 q3, q3, q10    // add gamma

        vqsub.u8 q3, q1, q3     // subtract result from primary

        // Load out of Q3
        vst1.64 { d6, d7 }, [r1]!

        sub r2, r2, #1
        cmp r2, #0
        bgt _c1_split_wst_loop
        ble _split_wst_end

    _c2_split_wst_loop:
        vld3.8 { d0, d2, d4 }, [r0]!
        vld3.8 { d1, d3, d5 }, [r0]!

        // Q0 --> alpha, Q1 --> beta, Q2 --> primary
        vshl.u8 q0, q0, q8      // weight alpha
        vshl.u8 q1, q1, q9      // weight beta
        vqadd.u8 q3, q0, q1     // add weights
        vqadd.u8 q3, q3, q10    // add gamma

        vqsub.u8 q3, q2, q3     // subtract result from primary

        // Load out of Q3
        vst1.64 { d6, d7 }, [r1]!

        sub r2, r2, #1
        cmp r2, #0
        bgt _c2_split_wst_loop
        ble _split_wst_end

    _split_wst_end:
        sub sp, fp, #0  // Pop our Stack Frame
        pop {r4-r6, fp}
        bx lr





// Arg 0: r0: Source A
// Arg 1: r1: Source B
// Arg 2: r2: Dest
// Arg 3: r3: Count
memcpy_bitwise_or_asm:
    push {fp}
    add fp, sp, #0  // Push new Stack Frame

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


// Arg 0: r0: Address of base array that is being subtracted
// Arg 1: r1: Address of second array that is being subtracted
// Arg 2: r2: Address of destination array
// Arg 3: r3: Size of arrays (width * height)
memcpy_subtract_asm:
    push {fp}
    add fp, sp, #0

    lsr r3, #4       // get iterations by dividing size by 16
    _subtract_loop:
        // load base
        vld1.64 d0, [r0]!
        vld1.64 d1, [r0]!

        // load subtractor
        vld1.64 d2, [r1]!
        vld1.64 d3, [r1]!

        // subtract
        vqsub.u8 q2, q0, q1

        // load Q2 to destination
        vst1.64 d4, [r2]!
        vst1.64 d5, [r2]!

        sub r3, r3, #1
        cmp r3, #0
        bgt _subtract_loop

    sub sp, fp, #0  // Pop our Stack Frame
    pop {fp}
    bx lr