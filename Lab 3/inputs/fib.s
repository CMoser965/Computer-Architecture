@ Fibonacci seris computation
@
@ r1 = result, r2 = prevresult
@

start:	
	mov 	r0, #0x1f 	@ n=32
	bl 	fib		@ call fibonacci function ...
	mov	r5, #0x104	@ base address
	str	r2, [r5]
	b	exit
	
fib:	mov 	r1, #1
	mov 	r2, #0
	cmp	r0,#0
	beq 	done
loop:	add 	r1, r1, r2
	sub 	r2, r1, r2
	subs 	r0, r0, #1
	bpl 	loop
done:	mov	r0, r2
	mov	pc, lr
exit:	ldr	r6, [r5, #0]
	