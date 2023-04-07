	.global _start
	.global main

_start:

main:
	mov		R0, R0

single:
	@ single condition
	cmp		R0, #0
	movlt	R11, #0
	movlt	R12, #1
	b		disjoint
	@ (A) 	cmp; movlt; movlt
	@ (B)	cmp; NOP; NOP

disjoint:
	@ inverted condition
	cmp		R1, #0
	movlt	R11, #0
	movge	R12, #1
	b		nonused
	@ (A) cmp; movlt; NOP
	@ (B) cmp; NOP; movge

nonused:
	@ non-used condition
	cmp		R2, #0
	movge	R11, #1
	b		nested
	@ the same, 1 BB

nested:
	@ nested condition
	cmp		R3, #0
	movle 	R11, #0
	movlt	R12, #0
	b		separated
	@ (A) cmp; movlt; movlt
	@ (B) cmp; moveq; NOP
	@ (C) cmp; NOP; NOP

separated:
	@ separated condition
	cmp		R3, #0
	movlt 	R11, #0
	movgt	R12, #0
	b		condbranch
	@ (A) cmp; movlt; NOP
	@ (B) cmp; NOP; movgt
	@ (C) cmp; NOP; NOP

condbranch:
	@ branch condition
	cmp		R3, #0
	movlt 	R11, #0
	bgt		taken
	@ (A) cmp; movlt; NOP -> notaken
	@ (B) cmp; NOP, bgt -> taken
notaken:
	mov		R11, #111
taken:

noduplicate:
	cmp		R3, #0
	beq		noduptaken

nodupnotaken:
	mov		R12, #12
noduptaken:

end:
	bx		lr
