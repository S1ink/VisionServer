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

		cmgt v1.8b, v0.8b, v3.8b
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

		cmgt v1.8b, v0.8b, v3.8b

		str q1, [x0, #0]!

		sub x2, x2, #1
		cmp x2, #0
		b.gt _thresh_bin_loop

	ldp x29, x30, [sp]
	ret