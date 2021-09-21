	.globl	morello_init_array
	.type	morello_init_array,@function
morello_init_array:
	stp	c29, c30, [csp, #-80]!  // 32-byte Folded Spill
	cmp	c0, c1
	str	c21, [csp, #32]         // 16-byte Folded Spill
	stp	c20, c19, [csp, #48]    // 32-byte Folded Spill
	mov	c29, csp
	b.hs	.LBB0_3
// %bb.1:                               // %for.body.preheader
	mov	c19, c1
	mov	c20, c0
.LBB0_2:                                // %for.body
                                        // =>This Inner Loop Header: Depth=1
	ldr	x0, [c20]
	cvtp    c0, x0
	blr	c0
	add	c20, c20, #16           // =16
	cmp	c20, c19
	b.lt	.LBB0_2
.LBB0_3:                                // %for.end
	ldp	c20, c19, [csp, #48]    // 32-byte Folded Reload
	ldr	c21, [csp, #32]         // 16-byte Folded Reload
	ldp	c29, c30, [csp], #80    // 32-byte Folded Reload
	ret	c30
.Lfunc_end0:
	.size	morello_init_array, .Lfunc_end0-morello_init_array
