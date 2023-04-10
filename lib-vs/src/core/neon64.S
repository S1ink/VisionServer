.global memcpy_threshold_asm
.global memcpy_threshold_binary_asm
.global memcpy_compare3_add_asm		// a bit broken?
.global memcpy_wst_asm
.global memcpy_deinterlace_wstb_asm
.global memcpy_deinterlace_togray_asm
.global memcpy_bitwise_or_asm
.global memcpy_bitwise_or_3c_asm
.global memcpy_add_asm
.global memcpy_add_3c_asm
.global memcpy_add_3c2_asm
.global memcpy_subtract_asm


/**
 * --> https://arm-software.github.io/acle/neon_intrinsics/advsimd.html
 * --> https://developer.arm.com/documentation/102159/0400/Load-and-store---example-RGB-conversion
 * --> https://developer.arm.com/documentation/den0024/a
 */

/* For the aarch64 ABI, x0-x7 are callee-saved, so no stack frame push is necessary unless more than 8 args are needed
 * To push the sp and fp, use (change the alloc ammount based on how many extra args there are):
	stp x29, x30, [sp, #-16]!
	mov x29, sp
 * And to restore, use (before ret):
	ldp x29, x30, [sp], #16
 * This is just a reminder in case this is ever needed, currently none of the functions have more than 8 args
 */


/* Arg 0 ~ x0: Source address (single channel frame)
 * Arg 1 ~ x1: Destonation address
 * Arg 2 ~ x2: Pixel count
 * Arg 3 ~ x3: Threshold value */
memcpy_threshold_asm:

	lsr w2, w2, #4
	dup v3.16b, w3
	_thresh_loop:
		ldr q0, [x0], #16
		cmhi v1.16b, v0.16b, v3.16b
		and v2.16b, v1.16b, v0.16b
		str q2, [x1], #16

		subs x2, x2, #1
		bne _thresh_loop

	ret
// END


/* Arg 0 ~ x0: Source address (single channel frame)
 * Arg 1 ~ x1: Destonation address
 * Arg 2 ~ x2: Pixel count
 * Arg 3 ~ x3: Threshold value */
memcpy_threshold_binary_asm:

	lsr w2, w2, #4
	dup v3.8b, w3
	_thresh_bin_loop:
		ldr q0, [x1], #16
		cmhi v1.16b, v0.16b, v3.16b
		str q1, [x0], #16

		subs x2, x2, #1
		bne _thresh_bin_loop

	ret
// END


/* Arg 0 ~ x0: Primary channel address
 * Arg 1 ~ x1: Compare channel 1 address
 * Arg 2 ~ x2: Compare channel 2 address
 * Arg 3 ~ x3: Addition address
 * Arg 4 ~ x4: Destination Address
 * Arg 5 ~ x5: Count */
memcpy_compare3_add_asm:

	lsr w5, w5, #4       // Shift count right by 4 bits (divide by 16, 16 bytes = 128 bits)
	_cmp3_add_loop:
		ldr q0, [x0], #16	// load primary compare
		ldr q1, [x1], #16	// load compare 1
		ldr q2, [x2], #16	// load compare 2
		ldr q3, [x3], #16	// load addition

		cmhi v1.16b, v0.16b, v1.16b		// compare primary and channel 1
		cmhi v2.16b, v0.16b, v2.16b		// compare primary and channel 2
		and v1.16b, v1.16b, v2.16b		// AND the results
		and v2.16b, v0.16b, v1.16b		// AND the threshold to the input
		ushr v1.16b, v2.16b, #6			// rshift result by 6 (div 32)

		neg v1.16b, v1.16b				// invert sign
		uqshl v2.16b, v0.16b, v1.16b	// left shift primary by negated result (right shift, mimics a division)
		uqshl v2.16b, v2.16b, #4		// left shift result to fill all 8 bits
		uqadd v3.16b, v2.16b, v3.16b	// add the threshold to the additive

		str q3, [x4], #16	// Load out of Q3

		subs x5, x5, #1
		bne _cmp3_add_loop

	ret
// END


/* Arg 0 ~ x0: Primary channel address
 * Arg 1 ~ x1: Compare channel 2 address
 * Arg 2 ~ x2: Compare channel 3 address
 * Arg 3 ~ x3: Destination address
 * Arg 4 ~ x4: Count
 * Arg 5 ~ x5: Alpha
 * Arg 6 ~ x6: Beta
 * Arg 7 ~ x7: Gamma */
memcpy_wst_asm:

	dup v4.16b, w5		// duplicate alpha across 16x8bit lanes
	clz v4.16b, v4.16b	// count the leading 0's (inverse of log2(value))
	neg v4.16b, v4.16b	// negate count (for right shifting)

	dup v5.16b, w6		// ''' for beta
	clz v5.16b, v5.16b
	neg v5.16b, v5.16b

	dup v6.16b, w7		// duplicate gamma across 16x8bit lanes

	lsr x4, x4, #4		// right shift the count by 4 bits (divide by 16, 16 bytes = 128 bits)

	_wst_loop:
		ldr q0, [x0], #16
		ldr q1, [x1], #16
		ldr q2, [x2], #16

		// Q0 --> primary, Q1 --> alpha, Q2 --> beta
		ushl v1.16b, v1.16b, v4.16b		// weight alpha --> shift right by the amount of leading 0's from alpha (lshift by negative)
		ushl v2.16b, v2.16b, v5.16b		// weight beta
		uqadd v3.16b, v1.16b, v2.16b	// add weights
		uqadd v3.16b, v3.16b, v6.16b	// add gamma
		uqsub v3.16b, v0.16b, v3.16b	// subtract result from primary

		str q3, [x1], #16	// Load out of Q3

		subs x4, x4, #1
		bne _wst_loop

	ret
// END


/* Arg 0 ~ x0: Source address (3-channel continuous buffer required)
 * Arg 1 ~ x1: Destination address
 * Arg 2 ~ x2: Count (frame size) --> NOT(size * #channels)
 * Arg 3 ~ x3: C1/Primary channel offset(index) --> ex. 0 for first; alpha/beta follow
 * Arg 4 ~ x4: Alpha
 * Arg 5 ~ x5: Beta
 * Arg 6 ~ x6: Gamma
 * Arg 7 ~ x7: Threshold */
memcpy_deinterlace_wstb_asm:

	dup v4.16b, w4		// duplicate alpha across 16x8bit lanes
	clz v4.16b, v4.16b	// count the leading 0's (inverse of log2(value))
	neg v4.16b, v4.16b	// negate count (for right shifting)

	dup v5.16b, w5		// ''' for beta
	clz v5.16b, v5.16b
	neg v5.16b, v5.16b

	dup v6.16b, w6		// duplicate gamma across 16x8bit lanes
	dup v7.16b, w7		// duplicate threshold '''

	lsr x2, x2, #4		// right shift the count by 4 bits (divide by 16, 16 bytes = 128 bits)

	cmp x3, #1
	b.lt _c0_split_wstb_loop
	b.eq _c1_split_wstb_loop
	b.gt _c2_split_wstb_loop

	_c0_split_wstb_loop:
		ld3 { v0.16b, v1.16b, v2.16b }, [x0], #48	// deinterlace 48 bytes of RGB into 3x16byte V registers

		// Q0 --> primary, Q1 --> alpha, Q2 --> beta
		ushl v1.16b, v1.16b, v4.16b		// weight alpha --> shift right by the amount of leading 0's from alpha (lshift by negative)
		ushl v2.16b, v2.16b, v5.16b		// weight beta
		uqadd v3.16b, v1.16b, v2.16b	// add weights
		uqadd v3.16b, v3.16b, v6.16b	// add gamma
		uqsub v3.16b, v0.16b, v3.16b	// subtract result from primary
		cmhi v3.16b, v3.16b, v7.16b		// threshold result

		str q3, [x1], #16	// Load out of Q3

		subs x2, x2, #1
		bne _c0_split_wstb_loop
		beq _split_wstb_end

	_c1_split_wstb_loop:
		ld3 { v0.16b, v1.16b, v2.16b }, [x0], #48

		// Q0 --> beta, Q1 --> primary, Q2 --> alpha
		ushl v2.16b, v2.16b, v4.16b		// weight alpha
		ushl v0.16b, v0.16b, v5.16b		// weight beta
		uqadd v3.16b, v2.16b, v0.16b	// add weights
		uqadd v3.16b, v3.16b, v6.16b	// add gamma
		uqsub v3.16b, v1.16b, v3.16b	// subtract result from primary
		cmhi v3.16b, v3.16b, v7.16b		// threshold result

		str q3, [x1], #16	// Load out of Q3

		subs x2, x2, #1
		bne _c1_split_wstb_loop
		beq _split_wstb_end

	_c2_split_wstb_loop:
		ld3 { v0.16b, v1.16b, v2.16b }, [x0], #48

		// Q0 --> alpha, Q1 --> beta, Q2 --> primary
		ushl v0.16b, v0.16b, v4.16b		// weight alpha
		ushl v1.16b, v1.16b, v5.16b		// weight beta
		uqadd v3.16b, v0.16b, v1.16b	// add weights
		uqadd v3.16b, v3.16b, v6.16b	// add gamma
		uqsub v3.16b, v2.16b, v3.16b	// subtract result from primary
		cmhi v3.16b, v3.16b, v7.16b		// threshold result

		str q3, [x1], #16	// Load out of Q3

		subs x2, x2, #1
		bne _c2_split_wstb_loop
		beq _split_wstb_end

	_split_wstb_end:
		ret
// END


/* Arg 0 ~ x0: Source array address (3-channel interlaced buffer)
 * Arg 1 ~ x1: Destination array address
 * Arg 2 ~ x2: Pixel total -- size of destination, size of source / 3 */
memcpy_deinterlace_togray_asm:

	cmp x2, #16
	b.lo _togray_single
	_togray_loop:
		ld3 { v0.16b - v2.16b }, [x0], #48

		ushr v0.16b, v0.16b, #2		// divide red/blue by 4
		ushr v1.16b, v1.16b, #1		// divide green by 2
		ushr v2.16b, v2.16b, #2		// divide blue/red by 4

		uqadd v3.16b, v0.16b, v1.16b
		uqadd v3.16b, v2.16b, v3.16b	// 1/4 + 1/2 + 1/4 = 1/1
		str q3, [x1], #16

		subs x2, x2, #16
		beq _togray_end		// exactly 0 bytes left --> end
		cmp x2, #16
		b.hs _togray_loop	// >= 16 --> keep going

	_togray_single:
		ld3 { v0.b - v2.b }[0], [x0], #3

		ushr v0.16b, v0.16b, #2		// divide red/blue by 4
		ushr v1.16b, v1.16b, #1		// divide green by 2
		ushr v2.16b, v2.16b, #2		// divide blue/red by 4

		uqadd v3.16b, v0.16b, v1.16b
		uqadd v3.16b, v2.16b, v3.16b	// 1/4 + 1/2 + 1/4 = 1/1
		st1 {v2.b}[0], [x1], #1

		subs x2, x2, #1
		bne _togray_single

	_togray_end:
		ret
// END



/* Arg 0 ~ x0: Source A
 * Arg 1 ~ x1: Source B
 * Arg 2 ~ x2: Destination
 * Arg 3 ~ x3: Count */
memcpy_bitwise_or_asm:

	lsr x3, x3, #4		// Shift count right by 4 bits (divide by 16, 16 bytes = 128 bits)
	_bitwise_or_loop:
		ldr q0, [x0], #16
		ldr q1, [x1], #16
		orr v2.16b, v0.16b, v1.16b
		str q2, [x2], #16

		subs x3, x3, #1
		bne _bitwise_or_loop

	ret
// END

/* Arg 0 ~ x0: Source A -- >> 3 channels <<
 * Arg 1 ~ x1: Source B
 * Arg 2 ~ x2: Destination -- >> 3 channels <<
 * Arg 3 ~ x3: Pixel count */
memcpy_bitwise_or_3c_asm:

	lsr x3, x3, #4
	_bitwise_or_3c_loop:
		ld3 { v0.16b - v2.16b }, [x0], #48	// deinterlace load rgb channels in q0-q2
		ldr q3, [x1], #16					// load binary frame normally in q3
		orr v0.16b, v0.16b, v3.16b
		orr v1.16b, v1.16b, v3.16b
		orr v2.16b, v2.16b, v3.16b			// or each channel with binary frame
		st3 { v0.16b - v2.16b }, [x2], #48	// interlace store the channels back

		subs x3, x3, #1
		bne _bitwise_or_loop

	ret
// END


/* Arg 0 ~ x0: Source A
 * Arg 1 ~ x1: Source B
 * Arg 2 ~ x2: Destination
 * Arg 3 ~ x3: Pixel count */
memcpy_add_asm:

	lsr x3, x3, #4		// get iterations by dividing size by 16
	_add_loop:
		ldr q0, [x0], #16
		ldr q1, [x1], #16
		uqadd v2.16b, v0.16b, v1.16b
		str q2, [x2], #16

		subs x3, x3, #1
		bne _add_loop

	ret
// END

/* Arg 0 ~ x0: Source A -- >> 3 channels <<
 * Arg 1 ~ x1: Source B
 * Arg 2 ~ x2: Destination -- >> 3 channels <<
 * Arg 3 ~ x3: Pixel count */
memcpy_add_3c_asm:

	lsr x3, x3, #4
	_add_3c_loop:
		ld3 { v0.16b - v2.16b }, [x0], #48
		ldr q3, [x1], #16
		uqadd v0.16b, v0.16b, v3.16b
		uqadd v1.16b, v1.16b, v3.16b
		uqadd v2.16b, v2.16b, v3.16b
		st3 { v0.16b - v2.16b }, [x2], #48

		subs x3, x3, #1
		bne _add_3c_loop

	ret
// END

/* Arg 0 ~ x0: Source A -- >> 3 channels <<
 * Arg 1 ~ x1: Source B -- >> 3 channels <<
 * Arg 2 ~ x2: Destination -- >> 3 channels <<
 * Arg 3 ~ x3: Pixel count */
memcpy_add_3c2_asm:

	lsr x3, x3, #4
	_add_3c2_loop:
		ld3 { v0.16b - v2.16b }, [x0], #48
		ld3 { v3.16b - v5.16b }, [x1], #48
		uqadd v0.16b, v0.16b, v3.16b
		uqadd v1.16b, v1.16b, v4.16b
		uqadd v2.16b, v2.16b, v5.16b
		st3 { v0.16b - v2.16b }, [x2], #48

		subs x3, x3, #1
		bne _add_3c_loop

	ret
// END


/* Arg 0 ~ x0: Address of base array that is being subtracted
 * Arg 1 ~ x1: Address of second array that is being subtracted
 * Arg 2 ~ x2: Address of destination array
 * Arg 3 ~ x3: Size of arrays (width * height) */
memcpy_subtract_asm:

	lsr x3, x3, #4		// get iterations by dividing size by 16
	_subtract_loop:
		ldr q0, [x0], #16
		ldr q1, [x1], #16
		uqsub v2.16b, v0.16b, v1.16b
		str q2, [x2], #16

		subs x3, x3, #1
		bne _subtract_loop

	ret
// END
