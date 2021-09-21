	.text
	.globl	morello_fini_array      // -- Begin function morello_fini_array
	.p2align	2
	.type	morello_fini_array,@function
morello_fini_array:                     // @morello_fini_array
// %bb.0:                               // %entry
	stp	c29, c30, [csp, #-64]!  // 32-byte Folded Spill
	stp	c20, c19, [csp, #32]    // 32-byte Folded Spill
	mov	c29, csp
	mov	c19, c1
	alignd	c20, c0, #16
.LBB0_1:                                // %while.body
                                        // =>This Inner Loop Header: Depth=1
	cmp	c20, c19
	b.ls	.LBB0_3
// %bb.2:                               // %while.body
                                        //   in Loop: Header=BB0_1 Depth=1
	ldr	c0, [c20, #-16]!
	blr	c0
	b	.LBB0_1
.LBB0_3:                                // %while.end
	ldp	c20, c19, [csp, #32]    // 32-byte Folded Reload
	ldp	c29, c30, [csp], #64    // 32-byte Folded Reload
	ret	c30
.Lfunc_end0:
	.size	morello_fini_array, .Lfunc_end0-morello_fini_array
                                        // -- End function
	.ident	"clang version 11.0.0 "
	.section	".note.GNU-stack","",@progbits
	.addrsig
