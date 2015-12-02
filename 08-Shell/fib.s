	.syntax unified
	.text
	.align 2
	.thumb
	.thumb_func

	.global fib
	.type fib, function

fib:
	@ ADD/MODIFY CODE BELOW
	@ PROLOG
	push {r3,r4,r5,r6, lr}
        mov r3,#0xFFFFFFFF
        mov r6,#1
        
loop:      
        add r4,r6,r3	
        mov r3,r6
        mov r6,r4
        add r5,r5,#1
        cmp r5,r0
        ble loop
        mov r0,r6
	pop {r3, r4, r5, r6, pc}		@EPILOG


	@ END CODE MODIFICATION

	.size fib, .-fib
	.end
