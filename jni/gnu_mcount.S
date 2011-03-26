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
        pop {r0}
        mov lr, r0
        pop {r0-r3}
        @ pop previous address
        sub sp, #4
        bx lr
