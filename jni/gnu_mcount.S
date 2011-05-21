.align 2
.thumb_func
.global __gnu_mcount_nc
.type __gnu_mcount_nc,function

__gnu_mcount_nc:
        push {r0-r3}
        push {lr}
        ldr r0, [sp, #20]  @ r0 = lr pushed by calling routine
        mov r1, lr    @ address of calling routine
        bl profCount
        pop {r2}           @ this routine's return address
        pop {r0, r1}
        @ stack contains r2, r3 and lr
        ldr r3, [sp , #8]  @ r3 = lr pushed by calling routine
        str r2, [sp, #8]   @ return address now last on the stack
        mov lr, r3         @ lr = caller's expected lr
        pop {r2, r3}
        pop {pc}   @ pop caller's expected r2, r3 and return
