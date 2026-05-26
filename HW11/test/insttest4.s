.global C^m
C^m:
L105:
push {r4-r10, fp, lr}
sub sp, sp, #4
add fp, sp, #36
mov t100, r0
mov t101, r1
movw t107, #0
cmp t101, t107
bgt L102
L103:
ldr t10600, [t100, #4]
mov r0, t10600
sub sp, fp, #36
add sp, sp, #4
pop {r4-r10, fp, lr}
bx lr
L102:
ldr t10400, [t100]
mov r0, t10400
sub sp, fp, #36
add sp, sp, #4
pop {r4-r10, fp, lr}
bx lr

.global __$main__^main
__$main__^main:
L108:
push {r4-r10, fp, lr}
sub sp, sp, #4
add fp, sp, #36
movw t148, #12
mov r0, t148
bl malloc
mov t10300, r0
movw t149, #20
mov r0, t149
bl malloc
mov t10400, r0
movw t150, #4
str t150, [t10400]
mov t12900, t10300
add t13600, t10300, #4
add t13700, t10300, #8
mov t10600, t10300
movw t151, #1
str t151, [t10400, #4]
movw t152, #2
str t152, [t10400, #8]
movw t153, #3
str t153, [t10400, #12]
str t150, [t10400, #16]
str t10400, [t12900]
movw t154, #24
mov r0, t154
bl malloc
mov t10500, r0
movw t155, #5
str t155, [t10500]
add t13800, t10600, #8
mov t11400, t10600
str t155, [t10500, #4]
movw t156, #6
str t156, [t10500, #8]
movw t157, #7
str t157, [t10500, #12]
movw t158, #8
str t158, [t10500, #16]
movw t159, #9
str t159, [t10500, #20]
str t10500, [t13600]
adr t160, C^m
str t160, [t13700]
ldr t11200, [t13800]
bl getint
mov t11300, r0
mov t11500, t11200
mov r0, t11400
mov r1, t11300
blx t11500
mov t11600, r0
mov t10000, t11600
ldr t10700, [t10000]
mov t10100, t10700
mov t10101, t10100
L102:
movw t164, #0
cmp t10101, t164
bge L103
L104:
movw t163, #10
mov r0, t163
bl putch
movw t165, #2
mov r0, t165
sub sp, fp, #36
add sp, sp, #4
pop {r4-r10, fp, lr}
bx lr
L103:
sub t10102, t10101, #1
mov t11800, t10000
ldr t10800, [t10000]
movw t166, #0
cmp t10102, t166
bge L106
L105:
movw t167, #65535
movt t167, #65535
mov r0, t167
bl exit
L106:
cmp t10102, t10800
bge L105
L107:
add t14200, t10102, #1
movw t161, #4
mul t14300, t14200, t161
ldr t11900, [t11800, t14300]
mov r0, t11900
bl putint
movw t162, #32
mov r0, t162
bl putch
mov t10101, t10102
b L102

