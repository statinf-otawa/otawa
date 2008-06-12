	.global main

main:
	str	lr, [sp, #-4]!
	
	add	r0, r0, #4
	bl	subprog
	
	ldr	pc, [sp], #4
	
subprog:
	mov	pc, lr
