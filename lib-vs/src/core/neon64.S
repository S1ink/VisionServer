.global memcpy_threshold_asm
.global memcpy_threshold_binary_asm
// .global memcpy_compare3_add_asm     // a bit broken
// .global memcpy_wst_asm
// .global memcpy_split_wst_asm
// 
// .global memcpy_bitwise_or_asm
// .global memcpy_subtract_asm



memcpy_threshold_asm:
	stp x29, x30, [sp]
	mov x29, sp

	lsr w2, w2, #4
	dup v3.8b, w3
	_thresh_loop:
		ldr q0, [x1, #0]!

		cmhi v1.8b, v0.8b, v3.8b
		and v2.8b, v1.8b, v0.8b

		str q2, [x0, #0]!

		sub x2, x2, #1
		cmp x2, #0
		b.gt _thresh_loop

	ldp x29, x30, [sp]
	ret


memcpy_threshold_binary_asm:
	stp x29, x30, [sp]
	mov x29, sp

	lsr w2, w2, #4
	dup v3.8b, w3
	_thresh_bin_loop:
		ldr q0, [x1, #0]!

		cmhi v1.8b, v0.8b, v3.8b

		str q1, [x0, #0]!

		sub x2, x2, #1
		cmp x2, #0
		b.gt _thresh_bin_loop

	ldp x29, x30, [sp]
	ret


// Arg 0: r0: Primary channel address
// Arg 1: r1: Compare channel 1 address
// Arg 2: r2: Compare channel 2 address
// Arg 3: r3: Addition address
// Arg 4: r4: Destination Address
// Arg 5: r5: Count
memcpy_compare3_add_asm:
	stp x29, x30, [sp]
	mov x29, sp

	lsr w5, w5, #4       // Shift count right by 4 bits (divide by 16, 16 bytes = 128 bits)
	_cmp3_add_loop:
		ldr q0, [x0, #0]!	// load primary compare
		ldr q1, [x1, #0]!	// load compare 1
		ldr q2, [x2, #0]!	// load compare 2
		ldr q3, [x3, #0]!	// load addition

		cmhi v1.8b, v0.8b, v1.8b	// compare primary and channel 1
		cmhi v2.8b, v0.8b, v2.8b	// compare primary and channel 2
		and v1.8b, v1.8b, v2.8b		// AND the results
		and v2.8b, v0.8b, v1.8b		// AND the threshold to the input
		sri v1.8b, v2.8b, #6		// rshift result by 6 (div 32)

		neg v1.8b, v1.8b			// invert sign
		lsl v2.8b, v0.8b, v1.8b		// left shift primary by negated result (right shift, mimics a division)
		shl v2.8b, v2.8b, #4		// left shift result to fill all 8 bits
		add v3.8b, v2.8b, v3.8b		// add the threshold to the additive

		str q3, [x4, #0]!	// Load out of Q3

		sub x5, x5, #1
		cmp x5, #0
		b.gt _cmp3_add_loop

	ldp x29, x30, [sp]
	ret


// // Arg 0: r0: Primary channel address
// // Arg 1: r1: Compare channel 2 address
// // Arg 2: r2: Compare channel 3 address
// // Arg 3: r3: Destination address
// // Arg 4: r4: Count
// // Arg 5: r5: Alpha
// // Arg 6: r6: Beta
// // Arg 7: r7: Gamma
// memcpy_wst_asm:
// 	push {r4-r7, fp}
// 	add fp, sp, #0  // Push new Stack Frame
// 
// 	ldr r4, [sp, #(0+20)]    // load 5th - 8th args from stack
// 	ldr r5, [sp, #(4+20)]
// 	ldr r6, [sp, #(8+20)]
// 	ldr r7, [sp, #(12+20)]
// 
// 	vdup.8 q8, r5   // duplicate alpha across 16x8bit chunks
// 	vclz.u8 q8, q8  // count the leading 0's (inverse of log2(value))
// 	vneg.s8 q8, q8  // negate count (for right shifting)
// 
// 	vdup.8 q9, r6   // ''' for beta
// 	vclz.u8 q9, q9
// 	vneg.s8 q9, q9
// 
// 	vdup.8 q10, r7  // duplicate gamma across 16x8bit chunks
// 
// 	lsr r4, #4       // right shift the count by 4 bits (divide by 16, 16 bytes = 128 bits)
// 	_wst_loop:
// 		// load primary channel
// 		vld1.64 d0, [r0]!   // Q0
// 		vld1.64 d1, [r0]!
// 		// load channel 2
// 		vld1.64 d2, [r1]!   // Q1
// 		vld1.64 d3, [r1]!
// 		// load channel 3
// 		vld1.64 d4, [r2]!   // Q2
// 		vld1.64 d5, [r2]!
// 
// 		vshl.u8 q1, q1, q8      // weight q1 --> q1
// 		vshl.u8 q2, q2, q9      // weight q2 --> q2
// 		vqadd.u8 q3, q1, q2     // add weights
// 		vqadd.u8 q3, q3, q10    // add gamma
// 
// 		vqsub.u8 q3, q0, q3     // subtract result from primary
// 
// 		// Load out of Q3
// 		vst1.64 d6, [r3]!
// 		vst1.64 d7, [r3]!
// 
// 		sub r4, r4, #1
// 		cmp r4, #0
// 		bgt _wst_loop
// 
// 	sub sp, fp, #0  // Pop our Stack Frame
// 	pop {r4-r7, fp}
// 	bx lr
// 
// 
// // Arg 0: r0: Source address (3-channel continuous buffer required)
// // Arg 1: r1: Destination address
// // Arg 2: r2: Count (frame size) --> NOT(size * #channels)
// // Arg 3: r3: C1/Primary channel offset(index) --> ex. 0 for first; alpha/beta follow
// // Arg 4: r4: Alpha
// // Arg 5: r5: Beta
// // Arg 6: r6: Gamma
// memcpy_split_wst_asm:
// 	push {r4-r6, fp}
// 	add fp, sp, #0  // Push new Stack Frame
// 
// 	ldr r4, [sp, #(0+16)]    // load 5th - 7th args from stack
// 	ldr r5, [sp, #(4+16)]
// 	ldr r6, [sp, #(8+16)]
// 
// 	vdup.8 q8, r4   // duplicate alpha across 16x8bit chunks
// 	vclz.u8 q8, q8  // count the leading 0's (inverse of log2(value))
// 	vneg.s8 q8, q8  // negate count (for right shifting)
// 
// 	vdup.8 q9, r5   // ''' for beta
// 	vclz.u8 q9, q9
// 	vneg.s8 q9, q9
// 
// 	vdup.8 q10, r6  // duplicate gamma across 16x8bit chunks
// 
// 	lsr r2, #4       // right shift the count by 4 bits (divide by 16, 16 bytes = 128 bits)
// 
// 	cmp r3, #1
// 	blt _c0_split_wst_loop
// 	beq _c1_split_wst_loop
// 	bgt _c2_split_wst_loop
// 
// 	_c0_split_wst_loop:
// 		//ld3 { v0.16b, v1.16b, v2.16b }, [r0]!
// 		vld3.8 { d0, d2, d4 }, [r0]!
// 		vld3.8 { d1, d3, d5 }, [r0]!
// 
// 		// Q0 --> primary, Q1 --> alpha, Q2 --> beta
// 		vshl.u8 q1, q1, q8      // weight alpha
// 		vshl.u8 q2, q2, q9      // weight beta
// 		vqadd.u8 q3, q1, q2     // add weights
// 		vqadd.u8 q3, q3, q10    // add gamma
// 
// 		vqsub.u8 q3, q0, q3     // subtract result from primary
// 
// 		// Load out of Q3
// 		vst1.64 { d6, d7 }, [r1]!
// 
// 		sub r2, r2, #1
// 		cmp r2, #0
// 		bgt _c0_split_wst_loop
// 		ble _split_wst_end
// 
// 	_c1_split_wst_loop:
// 		vld3.8 { d0, d2, d4 }, [r0]!
// 		vld3.8 { d1, d3, d5 }, [r0]!
// 
// 		// Q0 --> beta, Q1 --> primary, Q2 --> alpha
// 		vshl.u8 q2, q2, q8      // weight alpha
// 		vshl.u8 q0, q0, q9      // weight beta
// 		vqadd.u8 q3, q2, q0     // add weights
// 		vqadd.u8 q3, q3, q10    // add gamma
// 
// 		vqsub.u8 q3, q1, q3     // subtract result from primary
// 
// 		// Load out of Q3
// 		vst1.64 { d6, d7 }, [r1]!
// 
// 		sub r2, r2, #1
// 		cmp r2, #0
// 		bgt _c1_split_wst_loop
// 		ble _split_wst_end
// 
// 	_c2_split_wst_loop:
// 		vld3.8 { d0, d2, d4 }, [r0]!
// 		vld3.8 { d1, d3, d5 }, [r0]!
// 
// 		// Q0 --> alpha, Q1 --> beta, Q2 --> primary
// 		vshl.u8 q0, q0, q8      // weight alpha
// 		vshl.u8 q1, q1, q9      // weight beta
// 		vqadd.u8 q3, q0, q1     // add weights
// 		vqadd.u8 q3, q3, q10    // add gamma
// 
// 		vqsub.u8 q3, q2, q3     // subtract result from primary
// 
// 		// Load out of Q3
// 		vst1.64 { d6, d7 }, [r1]!
// 
// 		sub r2, r2, #1
// 		cmp r2, #0
// 		bgt _c2_split_wst_loop
// 		ble _split_wst_end
// 
// 	_split_wst_end:
// 		sub sp, fp, #0  // Pop our Stack Frame
// 		pop {r4-r6, fp}
// 		bx lr
// 
// 
// 
// 
// 
// // Arg 0: r0: Source A
// // Arg 1: r1: Source B
// // Arg 2: r2: Dest
// // Arg 3: r3: Count
// memcpy_bitwise_or_asm:
// 	push {fp}
// 	add fp, sp, #0  // Push new Stack Frame
// 
// 	lsr r3, #4       // Shift count right by 4 bits (divide by 16, 16 bytes = 128 bits)
// 	_bitwise_or_loop:
// 		// Load into Q0
// 		vld1.64 d0, [r0]!
// 		vld1.64 d1, [r0]!
// 		// Load into Q1
// 		vld1.64 d2, [r1]!
// 		vld1.64 d3, [r1]!
// 
// 		vorr.u8 q2, q0, q1
// 
// 		// Load out of Q2
// 		vst1.64 d4, [r2]!
// 		vst1.64 d5, [r2]!
// 
// 		sub r3, r3, #1
// 		cmp r3, #0
// 		bgt _bitwise_or_loop
// 
// 	sub sp, fp, #0  // Pop our Stack Frame
// 	pop {fp}
// 	bx lr
// 
// 
// // Arg 0: r0: Address of base array that is being subtracted
// // Arg 1: r1: Address of second array that is being subtracted
// // Arg 2: r2: Address of destination array
// // Arg 3: r3: Size of arrays (width * height)
// memcpy_subtract_asm:
// 	push {fp}
// 	add fp, sp, #0
// 
// 	lsr r3, #4       // get iterations by dividing size by 16
// 	_subtract_loop:
// 		// load base
// 		vld1.64 d0, [r0]!
// 		vld1.64 d1, [r0]!
// 
// 		// load subtractor
// 		vld1.64 d2, [r1]!
// 		vld1.64 d3, [r1]!
// 
// 		// subtract
// 		vqsub.u8 q2, q0, q1
// 
// 		// load Q2 to destination
// 		vst1.64 d4, [r2]!
// 		vst1.64 d5, [r2]!
// 
// 		sub r3, r3, #1
// 		cmp r3, #0
// 		bgt _subtract_loop
// 
// 	sub sp, fp, #0  // Pop our Stack Frame
// 	pop {fp}
// 	bx lr