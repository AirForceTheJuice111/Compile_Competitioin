.balign 4
.global main
.section .text
.arm
main:
main$L402:
	push {r4-r10, fp, lr}
	sub sp, sp, #812
	add fp, sp, #844
	mov r10, #0
	str r10, [fp, #-40]
	movw r1, #2
	movw r0, #1
	cmp r1, r0
	bgt main$L104
	ldr r9, [fp, #-40]
	mov r10, r9
	str r10, [fp, #-44]
main$L400:
	movw r1, #100
	movw r0, #100
	mul r10, r1, r0
	str r10, [fp, #-224]
	movw r1, #100
	movw r0, #100
	mul r10, r1, r0
	str r10, [fp, #-560]
	movw r1, #100
	movw r0, #100
	mul r10, r1, r0
	str r10, [fp, #-568]
	movw r1, #100
	movw r0, #100
	mul r10, r1, r0
	str r10, [fp, #-576]
	movw r1, #100
	movw r0, #100
	mul r10, r1, r0
	str r10, [fp, #-584]
	movw r1, #100
	movw r0, #100
	mul r10, r1, r0
	str r10, [fp, #-592]
	movw r1, #100
	movw r0, #100
	mul r10, r1, r0
	str r10, [fp, #-600]
	movw r1, #100
	movw r0, #100
	mul r10, r1, r0
	str r10, [fp, #-608]
	movw r1, #100
	movw r0, #100
	mul r10, r1, r0
	str r10, [fp, #-616]
	movw r1, #100
	movw r0, #100
	mul r10, r1, r0
	str r10, [fp, #-624]
	movw r1, #100
	movw r0, #100
	mul r10, r1, r0
	str r10, [fp, #-632]
	movw r1, #100
	movw r0, #100
	mul r10, r1, r0
	str r10, [fp, #-640]
	movw r1, #100
	movw r0, #100
	mul r10, r1, r0
	str r10, [fp, #-648]
	movw r1, #100
	movw r0, #100
	mul r10, r1, r0
	str r10, [fp, #-656]
	movw r1, #100
	movw r0, #100
	mul r10, r1, r0
	str r10, [fp, #-664]
	movw r1, #100
	movw r0, #100
	mul r10, r1, r0
	str r10, [fp, #-672]
	movw r1, #100
	movw r0, #100
	mul r10, r1, r0
	str r10, [fp, #-680]
	movw r1, #100
	movw r0, #100
	mul r10, r1, r0
	str r10, [fp, #-688]
	movw r1, #100
	movw r0, #100
	mul r10, r1, r0
	str r10, [fp, #-696]
	movw r1, #100
	movw r0, #100
	mul r10, r1, r0
	str r10, [fp, #-704]
	movw r1, #100
	movw r0, #100
	mul r10, r1, r0
	str r10, [fp, #-712]
	movw r1, #100
	movw r0, #100
	mul r10, r1, r0
	str r10, [fp, #-720]
	movw r1, #100
	movw r0, #100
	mul r10, r1, r0
	str r10, [fp, #-728]
	movw r1, #100
	movw r0, #100
	mul r10, r1, r0
	str r10, [fp, #-736]
	movw r1, #100
	movw r0, #100
	mul r10, r1, r0
	str r10, [fp, #-744]
	movw r1, #100
	movw r0, #100
	mul r10, r1, r0
	str r10, [fp, #-752]
	movw r1, #100
	movw r0, #100
	mul r10, r1, r0
	str r10, [fp, #-760]
	movw r1, #100
	movw r0, #100
	mul r10, r1, r0
	str r10, [fp, #-768]
	movw r1, #100
	movw r0, #100
	mul r10, r1, r0
	str r10, [fp, #-776]
	movw r1, #100
	movw r0, #100
	mul r10, r1, r0
	str r10, [fp, #-784]
	movw r1, #100
	movw r0, #100
	mul r10, r1, r0
	str r10, [fp, #-792]
	movw r1, #100
	movw r0, #100
	mul r10, r1, r0
	str r10, [fp, #-800]
	movw r1, #100
	movw r0, #100
	mul r10, r1, r0
	str r10, [fp, #-808]
	movw r1, #100
	movw r0, #100
	mul r10, r1, r0
	str r10, [fp, #-816]
	movw r1, #100
	movw r0, #100
	mul r10, r1, r0
	str r10, [fp, #-824]
	movw r1, #100
	movw r0, #100
	mul r10, r1, r0
	str r10, [fp, #-832]
	movw r1, #100
	movw r0, #100
	mul r10, r1, r0
	str r10, [fp, #-840]
	movw r1, #100
	movw r0, #100
	mul r10, r1, r0
	str r10, [fp, #-208]
	movw r1, #100
	movw r0, #100
	mul r10, r1, r0
	str r10, [fp, #-264]
	movw r1, #100
	movw r0, #100
	mul r10, r1, r0
	str r10, [fp, #-272]
	movw r1, #100
	movw r0, #100
	mul r10, r1, r0
	str r10, [fp, #-280]
	movw r1, #100
	movw r0, #100
	mul r10, r1, r0
	str r10, [fp, #-288]
	movw r1, #100
	movw r0, #100
	mul r10, r1, r0
	str r10, [fp, #-296]
	movw r1, #100
	movw r0, #100
	mul r10, r1, r0
	str r10, [fp, #-248]
	movw r1, #100
	movw r0, #100
	mul r10, r1, r0
	str r10, [fp, #-256]
	movw r1, #100
	movw r0, #100
	mul r10, r1, r0
	str r10, [fp, #-232]
	movw r1, #100
	movw r0, #100
	mul r10, r1, r0
	str r10, [fp, #-240]
	movw r1, #100
	movw r0, #100
	mul r10, r1, r0
	str r10, [fp, #-60]
	movw r1, #100
	movw r0, #100
	mul r10, r1, r0
	str r10, [fp, #-216]
	movw r1, #100
	movw r0, #100
	mul r10, r1, r0
	str r10, [fp, #-440]
	movw r1, #100
	movw r0, #100
	mul r10, r1, r0
	str r10, [fp, #-448]
	movw r1, #100
	movw r0, #100
	mul r10, r1, r0
	str r10, [fp, #-456]
	movw r1, #100
	movw r0, #100
	mul r10, r1, r0
	str r10, [fp, #-464]
	movw r1, #100
	movw r0, #100
	mul r10, r1, r0
	str r10, [fp, #-472]
	movw r1, #100
	movw r0, #100
	mul r10, r1, r0
	str r10, [fp, #-480]
	movw r1, #100
	movw r0, #100
	mul r10, r1, r0
	str r10, [fp, #-488]
	movw r1, #100
	movw r0, #100
	mul r10, r1, r0
	str r10, [fp, #-496]
	movw r1, #100
	movw r0, #100
	mul r10, r1, r0
	str r10, [fp, #-504]
	movw r1, #100
	movw r0, #100
	mul r10, r1, r0
	str r10, [fp, #-512]
	movw r1, #100
	movw r0, #100
	mul r10, r1, r0
	str r10, [fp, #-520]
	movw r1, #100
	movw r0, #100
	mul r10, r1, r0
	str r10, [fp, #-528]
	movw r1, #100
	movw r0, #100
	mul r10, r1, r0
	str r10, [fp, #-536]
	movw r1, #100
	movw r0, #100
	mul r10, r1, r0
	str r10, [fp, #-132]
	movw r1, #100
	movw r0, #100
	mul r10, r1, r0
	str r10, [fp, #-100]
	movw r1, #100
	movw r0, #100
	mul r10, r1, r0
	str r10, [fp, #-92]
	movw r1, #100
	movw r0, #100
	mul r10, r1, r0
	str r10, [fp, #-84]
	movw r1, #100
	movw r0, #100
	mul r10, r1, r0
	str r10, [fp, #-76]
	movw r1, #100
	movw r0, #100
	mul r10, r1, r0
	str r10, [fp, #-68]
	movw r1, #100
	movw r0, #100
	mul r10, r1, r0
	str r10, [fp, #-124]
	movw r1, #100
	movw r0, #100
	mul r10, r1, r0
	str r10, [fp, #-148]
	movw r1, #100
	movw r0, #100
	mul r10, r1, r0
	str r10, [fp, #-116]
	movw r1, #100
	movw r0, #100
	mul r10, r1, r0
	str r10, [fp, #-108]
	movw r1, #100
	movw r0, #100
	mul r10, r1, r0
	str r10, [fp, #-156]
	movw r1, #100
	movw r0, #100
	mul r10, r1, r0
	str r10, [fp, #-140]
	movw r1, #100
	movw r0, #100
	mul r10, r1, r0
	str r10, [fp, #-164]
	movw r1, #100
	movw r0, #100
	mul r10, r1, r0
	str r10, [fp, #-172]
	movw r1, #100
	movw r0, #100
	mul r10, r1, r0
	str r10, [fp, #-180]
	movw r1, #100
	movw r0, #100
	mul r10, r1, r0
	str r10, [fp, #-188]
	movw r1, #100
	movw r0, #100
	mul r10, r1, r0
	str r10, [fp, #-196]
	movw r1, #100
	movw r0, #100
	mul r2, r1, r0
	movw r1, #100
	movw r0, #100
	mul r10, r1, r0
	str r10, [fp, #-52]
	movw r1, #100
	movw r0, #100
	mul r10, r1, r0
	str r10, [fp, #-48]
	movw r1, #100
	movw r0, #100
	mul r10, r1, r0
	str r10, [fp, #-544]
	movw r1, #100
	movw r0, #100
	mul r10, r1, r0
	str r10, [fp, #-552]
	movw r1, #100
	movw r0, #100
	mul r10, r1, r0
	str r10, [fp, #-304]
	movw r1, #100
	movw r0, #100
	mul r10, r1, r0
	str r10, [fp, #-312]
	movw r1, #100
	movw r0, #100
	mul r10, r1, r0
	str r10, [fp, #-320]
	movw r1, #100
	movw r0, #100
	mul r10, r1, r0
	str r10, [fp, #-328]
	movw r1, #100
	movw r0, #100
	mul r10, r1, r0
	str r10, [fp, #-336]
	movw r1, #100
	movw r0, #100
	mul r10, r1, r0
	str r10, [fp, #-344]
	movw r1, #100
	movw r0, #100
	mul r10, r1, r0
	str r10, [fp, #-352]
	movw r1, #100
	movw r0, #100
	mul r10, r1, r0
	str r10, [fp, #-360]
	movw r1, #100
	movw r0, #100
	mul r10, r1, r0
	str r10, [fp, #-368]
	movw r1, #100
	movw r0, #100
	mul r10, r1, r0
	str r10, [fp, #-376]
	movw r1, #100
	movw r0, #100
	mul r10, r1, r0
	str r10, [fp, #-384]
	movw r1, #100
	movw r0, #100
	mul r10, r1, r0
	str r10, [fp, #-392]
	movw r1, #100
	movw r0, #100
	mul r10, r1, r0
	str r10, [fp, #-400]
	movw r1, #100
	movw r0, #100
	mul r10, r1, r0
	str r10, [fp, #-408]
	movw r1, #100
	movw r0, #100
	mul r10, r1, r0
	str r10, [fp, #-416]
	movw r1, #100
	movw r0, #100
	mul r10, r1, r0
	str r10, [fp, #-424]
	movw r1, #100
	movw r0, #100
	mul r10, r1, r0
	str r10, [fp, #-432]
	movw r1, #100
	movw r0, #100
	mul r0, r1, r0
	movw r1, #100
	ldr r9, [fp, #-224]
	sdiv r10, r9, r1
	str r10, [fp, #-228]
	movw r1, #100
	ldr r9, [fp, #-560]
	sdiv r10, r9, r1
	str r10, [fp, #-564]
	movw r1, #100
	ldr r9, [fp, #-568]
	sdiv r10, r9, r1
	str r10, [fp, #-572]
	movw r1, #100
	ldr r9, [fp, #-576]
	sdiv r10, r9, r1
	str r10, [fp, #-580]
	movw r1, #100
	ldr r9, [fp, #-584]
	sdiv r10, r9, r1
	str r10, [fp, #-588]
	movw r1, #100
	ldr r9, [fp, #-592]
	sdiv r10, r9, r1
	str r10, [fp, #-596]
	movw r1, #100
	ldr r9, [fp, #-600]
	sdiv r10, r9, r1
	str r10, [fp, #-604]
	movw r1, #100
	ldr r9, [fp, #-608]
	sdiv r10, r9, r1
	str r10, [fp, #-612]
	movw r1, #100
	ldr r9, [fp, #-616]
	sdiv r10, r9, r1
	str r10, [fp, #-620]
	movw r1, #100
	ldr r9, [fp, #-624]
	sdiv r10, r9, r1
	str r10, [fp, #-628]
	movw r1, #100
	ldr r9, [fp, #-632]
	sdiv r10, r9, r1
	str r10, [fp, #-636]
	movw r1, #100
	ldr r9, [fp, #-640]
	sdiv r10, r9, r1
	str r10, [fp, #-644]
	movw r1, #100
	ldr r9, [fp, #-648]
	sdiv r10, r9, r1
	str r10, [fp, #-652]
	movw r1, #100
	ldr r9, [fp, #-656]
	sdiv r10, r9, r1
	str r10, [fp, #-660]
	movw r1, #100
	ldr r9, [fp, #-664]
	sdiv r10, r9, r1
	str r10, [fp, #-668]
	movw r1, #100
	ldr r9, [fp, #-672]
	sdiv r10, r9, r1
	str r10, [fp, #-676]
	movw r1, #100
	ldr r9, [fp, #-680]
	sdiv r10, r9, r1
	str r10, [fp, #-684]
	movw r1, #100
	ldr r9, [fp, #-688]
	sdiv r10, r9, r1
	str r10, [fp, #-692]
	movw r1, #100
	ldr r9, [fp, #-696]
	sdiv r10, r9, r1
	str r10, [fp, #-700]
	movw r1, #100
	ldr r9, [fp, #-704]
	sdiv r10, r9, r1
	str r10, [fp, #-708]
	movw r1, #100
	ldr r9, [fp, #-712]
	sdiv r10, r9, r1
	str r10, [fp, #-716]
	movw r1, #100
	ldr r9, [fp, #-720]
	sdiv r10, r9, r1
	str r10, [fp, #-724]
	movw r1, #100
	ldr r9, [fp, #-728]
	sdiv r10, r9, r1
	str r10, [fp, #-732]
	movw r1, #100
	ldr r9, [fp, #-736]
	sdiv r10, r9, r1
	str r10, [fp, #-740]
	movw r1, #100
	ldr r9, [fp, #-744]
	sdiv r10, r9, r1
	str r10, [fp, #-748]
	movw r1, #100
	ldr r9, [fp, #-752]
	sdiv r10, r9, r1
	str r10, [fp, #-756]
	movw r1, #100
	ldr r9, [fp, #-760]
	sdiv r10, r9, r1
	str r10, [fp, #-764]
	movw r1, #100
	ldr r9, [fp, #-768]
	sdiv r10, r9, r1
	str r10, [fp, #-772]
	movw r1, #100
	ldr r9, [fp, #-776]
	sdiv r10, r9, r1
	str r10, [fp, #-780]
	movw r1, #100
	ldr r9, [fp, #-784]
	sdiv r10, r9, r1
	str r10, [fp, #-788]
	movw r1, #100
	ldr r9, [fp, #-792]
	sdiv r10, r9, r1
	str r10, [fp, #-796]
	movw r1, #100
	ldr r9, [fp, #-800]
	sdiv r10, r9, r1
	str r10, [fp, #-804]
	movw r1, #100
	ldr r9, [fp, #-808]
	sdiv r10, r9, r1
	str r10, [fp, #-812]
	movw r1, #100
	ldr r9, [fp, #-816]
	sdiv r10, r9, r1
	str r10, [fp, #-820]
	movw r1, #100
	ldr r9, [fp, #-824]
	sdiv r10, r9, r1
	str r10, [fp, #-828]
	movw r1, #100
	ldr r9, [fp, #-832]
	sdiv r10, r9, r1
	str r10, [fp, #-836]
	movw r1, #100
	ldr r9, [fp, #-840]
	sdiv r10, r9, r1
	str r10, [fp, #-844]
	movw r1, #100
	ldr r9, [fp, #-208]
	sdiv r10, r9, r1
	str r10, [fp, #-212]
	movw r1, #100
	ldr r9, [fp, #-264]
	sdiv r10, r9, r1
	str r10, [fp, #-268]
	movw r1, #100
	ldr r9, [fp, #-272]
	sdiv r10, r9, r1
	str r10, [fp, #-276]
	movw r1, #100
	ldr r9, [fp, #-280]
	sdiv r10, r9, r1
	str r10, [fp, #-284]
	movw r1, #100
	ldr r9, [fp, #-288]
	sdiv r10, r9, r1
	str r10, [fp, #-292]
	movw r1, #100
	ldr r9, [fp, #-296]
	sdiv r10, r9, r1
	str r10, [fp, #-300]
	movw r1, #100
	ldr r9, [fp, #-248]
	sdiv r10, r9, r1
	str r10, [fp, #-252]
	movw r1, #100
	ldr r9, [fp, #-256]
	sdiv r10, r9, r1
	str r10, [fp, #-260]
	movw r1, #100
	ldr r9, [fp, #-232]
	sdiv r10, r9, r1
	str r10, [fp, #-236]
	movw r1, #100
	ldr r9, [fp, #-240]
	sdiv r10, r9, r1
	str r10, [fp, #-244]
	movw r1, #100
	ldr r9, [fp, #-60]
	sdiv r10, r9, r1
	str r10, [fp, #-64]
	movw r1, #100
	ldr r9, [fp, #-216]
	sdiv r10, r9, r1
	str r10, [fp, #-220]
	movw r1, #100
	ldr r9, [fp, #-440]
	sdiv r10, r9, r1
	str r10, [fp, #-444]
	movw r1, #100
	ldr r9, [fp, #-448]
	sdiv r10, r9, r1
	str r10, [fp, #-452]
	movw r1, #100
	ldr r9, [fp, #-456]
	sdiv r10, r9, r1
	str r10, [fp, #-460]
	movw r1, #100
	ldr r9, [fp, #-464]
	sdiv r10, r9, r1
	str r10, [fp, #-468]
	movw r1, #100
	ldr r9, [fp, #-472]
	sdiv r10, r9, r1
	str r10, [fp, #-476]
	movw r1, #100
	ldr r9, [fp, #-480]
	sdiv r10, r9, r1
	str r10, [fp, #-484]
	movw r1, #100
	ldr r9, [fp, #-488]
	sdiv r10, r9, r1
	str r10, [fp, #-492]
	movw r1, #100
	ldr r9, [fp, #-496]
	sdiv r10, r9, r1
	str r10, [fp, #-500]
	movw r1, #100
	ldr r9, [fp, #-504]
	sdiv r10, r9, r1
	str r10, [fp, #-508]
	movw r1, #100
	ldr r9, [fp, #-512]
	sdiv r10, r9, r1
	str r10, [fp, #-516]
	movw r1, #100
	ldr r9, [fp, #-520]
	sdiv r10, r9, r1
	str r10, [fp, #-524]
	movw r1, #100
	ldr r9, [fp, #-528]
	sdiv r10, r9, r1
	str r10, [fp, #-532]
	movw r1, #100
	ldr r9, [fp, #-536]
	sdiv r10, r9, r1
	str r10, [fp, #-540]
	movw r1, #100
	ldr r9, [fp, #-132]
	sdiv r10, r9, r1
	str r10, [fp, #-136]
	movw r1, #100
	ldr r9, [fp, #-100]
	sdiv r10, r9, r1
	str r10, [fp, #-104]
	movw r1, #100
	ldr r9, [fp, #-92]
	sdiv r10, r9, r1
	str r10, [fp, #-96]
	movw r1, #100
	ldr r9, [fp, #-84]
	sdiv r10, r9, r1
	str r10, [fp, #-88]
	movw r1, #100
	ldr r9, [fp, #-76]
	sdiv r10, r9, r1
	str r10, [fp, #-80]
	movw r1, #100
	ldr r9, [fp, #-68]
	sdiv r10, r9, r1
	str r10, [fp, #-72]
	movw r1, #100
	ldr r9, [fp, #-124]
	sdiv r10, r9, r1
	str r10, [fp, #-128]
	movw r1, #100
	ldr r9, [fp, #-148]
	sdiv r10, r9, r1
	str r10, [fp, #-152]
	movw r1, #100
	ldr r9, [fp, #-116]
	sdiv r10, r9, r1
	str r10, [fp, #-120]
	movw r1, #100
	ldr r9, [fp, #-108]
	sdiv r10, r9, r1
	str r10, [fp, #-112]
	movw r1, #100
	ldr r9, [fp, #-156]
	sdiv r10, r9, r1
	str r10, [fp, #-160]
	movw r1, #100
	ldr r9, [fp, #-140]
	sdiv r10, r9, r1
	str r10, [fp, #-144]
	movw r1, #100
	ldr r9, [fp, #-164]
	sdiv r10, r9, r1
	str r10, [fp, #-168]
	movw r1, #100
	ldr r9, [fp, #-172]
	sdiv r10, r9, r1
	str r10, [fp, #-176]
	movw r1, #100
	ldr r9, [fp, #-180]
	sdiv r10, r9, r1
	str r10, [fp, #-184]
	movw r1, #100
	ldr r9, [fp, #-188]
	sdiv r10, r9, r1
	str r10, [fp, #-192]
	movw r1, #100
	ldr r9, [fp, #-196]
	sdiv r10, r9, r1
	str r10, [fp, #-200]
	movw r1, #100
	sdiv r10, r2, r1
	str r10, [fp, #-204]
	movw r1, #100
	ldr r9, [fp, #-52]
	sdiv r10, r9, r1
	str r10, [fp, #-56]
	movw r1, #100
	ldr r9, [fp, #-48]
	sdiv r2, r9, r1
	movw r1, #100
	ldr r9, [fp, #-544]
	sdiv r10, r9, r1
	str r10, [fp, #-548]
	movw r1, #100
	ldr r9, [fp, #-552]
	sdiv r10, r9, r1
	str r10, [fp, #-556]
	movw r1, #100
	ldr r9, [fp, #-304]
	sdiv r10, r9, r1
	str r10, [fp, #-308]
	movw r1, #100
	ldr r9, [fp, #-312]
	sdiv r10, r9, r1
	str r10, [fp, #-316]
	movw r1, #100
	ldr r9, [fp, #-320]
	sdiv r10, r9, r1
	str r10, [fp, #-324]
	movw r1, #100
	ldr r9, [fp, #-328]
	sdiv r10, r9, r1
	str r10, [fp, #-332]
	movw r1, #100
	ldr r9, [fp, #-336]
	sdiv r10, r9, r1
	str r10, [fp, #-340]
	movw r1, #100
	ldr r9, [fp, #-344]
	sdiv r10, r9, r1
	str r10, [fp, #-348]
	movw r1, #100
	ldr r9, [fp, #-352]
	sdiv r10, r9, r1
	str r10, [fp, #-356]
	movw r1, #100
	ldr r9, [fp, #-360]
	sdiv r10, r9, r1
	str r10, [fp, #-364]
	movw r1, #100
	ldr r9, [fp, #-368]
	sdiv r10, r9, r1
	str r10, [fp, #-372]
	movw r1, #100
	ldr r9, [fp, #-376]
	sdiv r10, r9, r1
	str r10, [fp, #-380]
	movw r1, #100
	ldr r9, [fp, #-384]
	sdiv r10, r9, r1
	str r10, [fp, #-388]
	movw r1, #100
	ldr r9, [fp, #-392]
	sdiv r10, r9, r1
	str r10, [fp, #-396]
	movw r1, #100
	ldr r9, [fp, #-400]
	sdiv r10, r9, r1
	str r10, [fp, #-404]
	movw r1, #100
	ldr r9, [fp, #-408]
	sdiv r10, r9, r1
	str r10, [fp, #-412]
	movw r1, #100
	ldr r9, [fp, #-416]
	sdiv r10, r9, r1
	str r10, [fp, #-420]
	movw r1, #100
	ldr r9, [fp, #-424]
	sdiv r10, r9, r1
	str r10, [fp, #-428]
	movw r1, #100
	ldr r9, [fp, #-432]
	sdiv r10, r9, r1
	str r10, [fp, #-436]
	movw r1, #100
	sdiv r0, r0, r1
	ldr r9, [fp, #-44]
	add r1, r9, r2
	sub r1, r1, #100
	ldr r10, [fp, #-56]
	add r1, r1, r10
	sub r1, r1, #100
	ldr r10, [fp, #-64]
	add r1, r1, r10
	sub r1, r1, #100
	ldr r10, [fp, #-72]
	add r1, r1, r10
	sub r1, r1, #100
	ldr r10, [fp, #-80]
	add r1, r1, r10
	sub r1, r1, #100
	ldr r10, [fp, #-88]
	add r1, r1, r10
	sub r1, r1, #100
	ldr r10, [fp, #-96]
	add r1, r1, r10
	sub r1, r1, #100
	ldr r10, [fp, #-104]
	add r1, r1, r10
	sub r1, r1, #100
	ldr r10, [fp, #-112]
	add r1, r1, r10
	sub r1, r1, #100
	ldr r10, [fp, #-120]
	add r1, r1, r10
	sub r1, r1, #100
	ldr r10, [fp, #-128]
	add r1, r1, r10
	sub r1, r1, #100
	ldr r10, [fp, #-136]
	add r1, r1, r10
	sub r1, r1, #100
	ldr r10, [fp, #-144]
	add r1, r1, r10
	sub r1, r1, #100
	ldr r10, [fp, #-152]
	add r1, r1, r10
	sub r1, r1, #100
	ldr r10, [fp, #-160]
	add r1, r1, r10
	sub r1, r1, #100
	ldr r10, [fp, #-168]
	add r1, r1, r10
	sub r1, r1, #100
	ldr r10, [fp, #-176]
	add r1, r1, r10
	sub r1, r1, #100
	ldr r10, [fp, #-184]
	add r1, r1, r10
	sub r1, r1, #100
	ldr r10, [fp, #-192]
	add r1, r1, r10
	sub r1, r1, #100
	ldr r10, [fp, #-200]
	add r1, r1, r10
	sub r1, r1, #100
	ldr r10, [fp, #-204]
	add r1, r1, r10
	sub r1, r1, #100
	ldr r10, [fp, #-212]
	add r1, r1, r10
	sub r1, r1, #100
	ldr r10, [fp, #-220]
	add r1, r1, r10
	sub r1, r1, #100
	ldr r10, [fp, #-228]
	add r1, r1, r10
	sub r1, r1, #100
	ldr r10, [fp, #-236]
	add r1, r1, r10
	sub r1, r1, #100
	ldr r10, [fp, #-244]
	add r1, r1, r10
	sub r1, r1, #100
	ldr r10, [fp, #-252]
	add r1, r1, r10
	sub r1, r1, #100
	ldr r10, [fp, #-260]
	add r1, r1, r10
	sub r1, r1, #100
	ldr r10, [fp, #-268]
	add r1, r1, r10
	sub r1, r1, #100
	ldr r10, [fp, #-276]
	add r1, r1, r10
	sub r1, r1, #100
	ldr r10, [fp, #-284]
	add r1, r1, r10
	sub r1, r1, #100
	ldr r10, [fp, #-292]
	add r1, r1, r10
	sub r1, r1, #100
	ldr r10, [fp, #-300]
	add r1, r1, r10
	sub r1, r1, #100
	ldr r10, [fp, #-308]
	add r1, r1, r10
	sub r1, r1, #100
	ldr r10, [fp, #-316]
	add r1, r1, r10
	sub r1, r1, #100
	ldr r10, [fp, #-324]
	add r1, r1, r10
	sub r1, r1, #100
	ldr r10, [fp, #-332]
	add r1, r1, r10
	sub r1, r1, #100
	ldr r10, [fp, #-340]
	add r1, r1, r10
	sub r1, r1, #100
	ldr r10, [fp, #-348]
	add r1, r1, r10
	sub r1, r1, #100
	ldr r10, [fp, #-356]
	add r1, r1, r10
	sub r1, r1, #100
	ldr r10, [fp, #-364]
	add r1, r1, r10
	sub r1, r1, #100
	ldr r10, [fp, #-372]
	add r1, r1, r10
	sub r1, r1, #100
	ldr r10, [fp, #-380]
	add r1, r1, r10
	sub r1, r1, #100
	ldr r10, [fp, #-388]
	add r1, r1, r10
	sub r1, r1, #100
	ldr r10, [fp, #-396]
	add r1, r1, r10
	sub r1, r1, #100
	ldr r10, [fp, #-404]
	add r1, r1, r10
	sub r1, r1, #100
	ldr r10, [fp, #-412]
	add r1, r1, r10
	sub r1, r1, #100
	ldr r10, [fp, #-420]
	add r1, r1, r10
	sub r1, r1, #100
	ldr r10, [fp, #-428]
	add r1, r1, r10
	sub r1, r1, #100
	ldr r10, [fp, #-436]
	add r1, r1, r10
	sub r1, r1, #100
	add r0, r1, r0
	sub r0, r0, #100
	ldr r10, [fp, #-444]
	add r0, r0, r10
	sub r0, r0, #100
	ldr r10, [fp, #-452]
	add r0, r0, r10
	sub r0, r0, #100
	ldr r10, [fp, #-460]
	add r0, r0, r10
	sub r0, r0, #100
	ldr r10, [fp, #-468]
	add r0, r0, r10
	sub r0, r0, #100
	ldr r10, [fp, #-476]
	add r0, r0, r10
	sub r0, r0, #100
	ldr r10, [fp, #-484]
	add r0, r0, r10
	sub r0, r0, #100
	ldr r10, [fp, #-492]
	add r0, r0, r10
	sub r0, r0, #100
	ldr r10, [fp, #-500]
	add r0, r0, r10
	sub r0, r0, #100
	ldr r10, [fp, #-508]
	add r0, r0, r10
	sub r0, r0, #100
	ldr r10, [fp, #-516]
	add r0, r0, r10
	sub r0, r0, #100
	ldr r10, [fp, #-524]
	add r0, r0, r10
	sub r0, r0, #100
	ldr r10, [fp, #-532]
	add r0, r0, r10
	sub r0, r0, #100
	ldr r10, [fp, #-540]
	add r0, r0, r10
	sub r0, r0, #100
	ldr r10, [fp, #-548]
	add r0, r0, r10
	sub r0, r0, #100
	ldr r10, [fp, #-556]
	add r0, r0, r10
	sub r0, r0, #100
	ldr r10, [fp, #-564]
	add r0, r0, r10
	sub r0, r0, #100
	ldr r10, [fp, #-572]
	add r0, r0, r10
	sub r0, r0, #100
	ldr r10, [fp, #-580]
	add r0, r0, r10
	sub r0, r0, #100
	ldr r10, [fp, #-588]
	add r0, r0, r10
	sub r0, r0, #100
	ldr r10, [fp, #-596]
	add r0, r0, r10
	sub r0, r0, #100
	ldr r10, [fp, #-604]
	add r0, r0, r10
	sub r0, r0, #100
	ldr r10, [fp, #-612]
	add r0, r0, r10
	sub r0, r0, #100
	ldr r10, [fp, #-620]
	add r0, r0, r10
	sub r0, r0, #100
	ldr r10, [fp, #-628]
	add r0, r0, r10
	sub r0, r0, #100
	ldr r10, [fp, #-636]
	add r0, r0, r10
	sub r0, r0, #100
	ldr r10, [fp, #-644]
	add r0, r0, r10
	sub r0, r0, #100
	ldr r10, [fp, #-652]
	add r0, r0, r10
	sub r0, r0, #100
	ldr r10, [fp, #-660]
	add r0, r0, r10
	sub r0, r0, #100
	ldr r10, [fp, #-668]
	add r0, r0, r10
	sub r0, r0, #100
	ldr r10, [fp, #-676]
	add r0, r0, r10
	sub r0, r0, #100
	ldr r10, [fp, #-684]
	add r0, r0, r10
	sub r0, r0, #100
	ldr r10, [fp, #-692]
	add r0, r0, r10
	sub r0, r0, #100
	ldr r10, [fp, #-700]
	add r0, r0, r10
	sub r0, r0, #100
	ldr r10, [fp, #-708]
	add r0, r0, r10
	sub r0, r0, #100
	ldr r10, [fp, #-716]
	add r0, r0, r10
	sub r0, r0, #100
	ldr r10, [fp, #-724]
	add r0, r0, r10
	sub r0, r0, #100
	ldr r10, [fp, #-732]
	add r0, r0, r10
	sub r0, r0, #100
	ldr r10, [fp, #-740]
	add r0, r0, r10
	sub r0, r0, #100
	ldr r10, [fp, #-748]
	add r0, r0, r10
	sub r0, r0, #100
	ldr r10, [fp, #-756]
	add r0, r0, r10
	sub r0, r0, #100
	ldr r10, [fp, #-764]
	add r0, r0, r10
	sub r0, r0, #100
	ldr r10, [fp, #-772]
	add r0, r0, r10
	sub r0, r0, #100
	ldr r10, [fp, #-780]
	add r0, r0, r10
	sub r0, r0, #100
	ldr r10, [fp, #-788]
	add r0, r0, r10
	sub r0, r0, #100
	ldr r10, [fp, #-796]
	add r0, r0, r10
	sub r0, r0, #100
	ldr r10, [fp, #-804]
	add r0, r0, r10
	sub r0, r0, #100
	ldr r10, [fp, #-812]
	add r0, r0, r10
	sub r0, r0, #100
	ldr r10, [fp, #-820]
	add r0, r0, r10
	sub r0, r0, #100
	ldr r10, [fp, #-828]
	add r0, r0, r10
	sub r0, r0, #100
	ldr r10, [fp, #-836]
	add r0, r0, r10
	sub r0, r0, #100
	ldr r10, [fp, #-844]
	add r0, r0, r10
	sub r0, r0, #100
	bl putint
	movw r0, #10
	bl putch
	movw r0, #0
	sub sp, fp, #844
	add sp, sp, #812
	pop {r4-r10, fp, lr}
	bx lr
main$L104:
	movw r1, #3
	movw r0, #2
	cmp r1, r0
	ble main$L404
main$L107:
	movw r1, #4
	movw r0, #3
	cmp r1, r0
	ble main$L405
main$L110:
	movw r1, #5
	movw r0, #4
	cmp r1, r0
	ble main$L406
main$L113:
	movw r1, #6
	movw r0, #5
	cmp r1, r0
	ble main$L407
main$L116:
	movw r1, #7
	movw r0, #6
	cmp r1, r0
	ble main$L408
main$L119:
	movw r1, #8
	movw r0, #7
	cmp r1, r0
	ble main$L409
main$L122:
	movw r1, #9
	movw r0, #8
	cmp r1, r0
	ble main$L410
main$L125:
	movw r1, #10
	movw r0, #9
	cmp r1, r0
	ble main$L411
main$L128:
	movw r1, #11
	movw r0, #10
	cmp r1, r0
	ble main$L412
main$L131:
	movw r1, #12
	movw r0, #11
	cmp r1, r0
	ble main$L413
main$L134:
	movw r1, #13
	movw r0, #12
	cmp r1, r0
	ble main$L414
main$L137:
	movw r1, #14
	movw r0, #13
	cmp r1, r0
	ble main$L415
main$L140:
	movw r1, #15
	movw r0, #14
	cmp r1, r0
	ble main$L416
main$L143:
	movw r1, #16
	movw r0, #15
	cmp r1, r0
	ble main$L417
main$L146:
	movw r1, #17
	movw r0, #16
	cmp r1, r0
	ble main$L418
main$L149:
	movw r1, #18
	movw r0, #17
	cmp r1, r0
	ble main$L419
main$L152:
	movw r1, #19
	movw r0, #18
	cmp r1, r0
	ble main$L420
main$L155:
	movw r1, #20
	movw r0, #19
	cmp r1, r0
	ble main$L421
main$L158:
	movw r1, #21
	movw r0, #20
	cmp r1, r0
	ble main$L422
main$L161:
	movw r1, #22
	movw r0, #21
	cmp r1, r0
	ble main$L423
main$L164:
	movw r1, #23
	movw r0, #22
	cmp r1, r0
	ble main$L424
main$L165:
	bl getint
	movw r1, #0
	cmp r0, r1
	beq main$L425
main$L170:
	movw r1, #24
	movw r0, #23
	cmp r1, r0
	ble main$L426
main$L173:
	movw r1, #25
	movw r0, #24
	cmp r1, r0
	ble main$L427
main$L176:
	movw r1, #26
	movw r0, #25
	cmp r1, r0
	ble main$L428
main$L179:
	movw r1, #27
	movw r0, #26
	cmp r1, r0
	ble main$L429
main$L182:
	movw r1, #28
	movw r0, #27
	cmp r1, r0
	ble main$L430
main$L185:
	movw r1, #29
	movw r0, #28
	cmp r1, r0
	ble main$L431
main$L188:
	movw r1, #30
	movw r0, #29
	cmp r1, r0
	ble main$L432
main$L191:
	movw r1, #31
	movw r0, #30
	cmp r1, r0
	ble main$L433
main$L194:
	movw r1, #32
	movw r0, #31
	cmp r1, r0
	ble main$L434
main$L197:
	movw r1, #33
	movw r0, #32
	cmp r1, r0
	ble main$L435
main$L200:
	movw r1, #34
	movw r0, #33
	cmp r1, r0
	ble main$L436
main$L203:
	movw r1, #35
	movw r0, #34
	cmp r1, r0
	ble main$L437
main$L206:
	movw r1, #36
	movw r0, #35
	cmp r1, r0
	ble main$L438
main$L209:
	movw r1, #37
	movw r0, #36
	cmp r1, r0
	ble main$L439
main$L212:
	movw r1, #38
	movw r0, #37
	cmp r1, r0
	ble main$L440
main$L215:
	movw r1, #39
	movw r0, #38
	cmp r1, r0
	ble main$L441
main$L218:
	movw r1, #40
	movw r0, #39
	cmp r1, r0
	ble main$L442
main$L221:
	movw r1, #41
	movw r0, #40
	cmp r1, r0
	ble main$L443
main$L224:
	movw r1, #42
	movw r0, #41
	cmp r1, r0
	ble main$L444
main$L227:
	movw r1, #43
	movw r0, #42
	cmp r1, r0
	ble main$L445
main$L230:
	movw r1, #44
	movw r0, #43
	cmp r1, r0
	ble main$L446
main$L233:
	movw r1, #45
	movw r0, #44
	cmp r1, r0
	ble main$L447
main$L236:
	movw r1, #46
	movw r0, #45
	cmp r1, r0
	ble main$L448
main$L239:
	movw r1, #47
	movw r0, #46
	cmp r1, r0
	ble main$L449
main$L242:
	movw r1, #48
	movw r0, #47
	cmp r1, r0
	ble main$L450
main$L245:
	movw r1, #49
	movw r0, #48
	cmp r1, r0
	ble main$L451
main$L248:
	movw r1, #50
	movw r0, #49
	cmp r1, r0
	ble main$L452
main$L251:
	movw r1, #51
	movw r0, #50
	cmp r1, r0
	ble main$L453
main$L254:
	movw r1, #52
	movw r0, #51
	cmp r1, r0
	ble main$L454
main$L257:
	movw r1, #53
	movw r0, #52
	cmp r1, r0
	ble main$L455
main$L260:
	movw r1, #54
	movw r0, #53
	cmp r1, r0
	ble main$L456
main$L263:
	movw r1, #55
	movw r0, #54
	cmp r1, r0
	ble main$L457
main$L266:
	movw r1, #56
	movw r0, #55
	cmp r1, r0
	ble main$L458
main$L269:
	movw r1, #57
	movw r0, #56
	cmp r1, r0
	ble main$L459
main$L272:
	movw r1, #58
	movw r0, #57
	cmp r1, r0
	ble main$L460
main$L275:
	movw r1, #59
	movw r0, #58
	cmp r1, r0
	ble main$L461
main$L278:
	movw r1, #60
	movw r0, #59
	cmp r1, r0
	ble main$L462
main$L281:
	movw r1, #61
	movw r0, #60
	cmp r1, r0
	ble main$L463
main$L284:
	movw r1, #62
	movw r0, #61
	cmp r1, r0
	ble main$L464
main$L287:
	movw r1, #63
	movw r0, #62
	cmp r1, r0
	ble main$L465
main$L290:
	movw r1, #64
	movw r0, #63
	cmp r1, r0
	ble main$L466
main$L293:
	movw r1, #65
	movw r0, #64
	cmp r1, r0
	ble main$L467
main$L296:
	movw r1, #66
	movw r0, #65
	cmp r1, r0
	ble main$L468
main$L299:
	movw r1, #67
	movw r0, #66
	cmp r1, r0
	ble main$L469
main$L302:
	movw r1, #68
	movw r0, #67
	cmp r1, r0
	ble main$L470
main$L305:
	movw r1, #69
	movw r0, #68
	cmp r1, r0
	ble main$L471
main$L308:
	movw r1, #70
	movw r0, #69
	cmp r1, r0
	ble main$L472
main$L311:
	movw r1, #71
	movw r0, #70
	cmp r1, r0
	ble main$L473
main$L314:
	movw r1, #72
	movw r0, #71
	cmp r1, r0
	ble main$L474
main$L317:
	movw r1, #73
	movw r0, #72
	cmp r1, r0
	ble main$L475
main$L320:
	movw r1, #74
	movw r0, #73
	cmp r1, r0
	ble main$L476
main$L323:
	movw r1, #75
	movw r0, #74
	cmp r1, r0
	ble main$L477
main$L326:
	movw r1, #76
	movw r0, #75
	cmp r1, r0
	ble main$L478
main$L329:
	movw r1, #77
	movw r0, #76
	cmp r1, r0
	ble main$L479
main$L332:
	movw r1, #78
	movw r0, #77
	cmp r1, r0
	ble main$L480
main$L335:
	movw r1, #79
	movw r0, #78
	cmp r1, r0
	ble main$L481
main$L338:
	movw r1, #80
	movw r0, #79
	cmp r1, r0
	ble main$L482
main$L341:
	movw r1, #81
	movw r0, #80
	cmp r1, r0
	ble main$L483
main$L344:
	movw r1, #82
	movw r0, #81
	cmp r1, r0
	ble main$L484
main$L347:
	movw r1, #83
	movw r0, #82
	cmp r1, r0
	ble main$L485
main$L350:
	movw r1, #84
	movw r0, #83
	cmp r1, r0
	ble main$L486
main$L353:
	movw r1, #85
	movw r0, #84
	cmp r1, r0
	ble main$L487
main$L356:
	movw r1, #86
	movw r0, #85
	cmp r1, r0
	ble main$L488
main$L359:
	movw r1, #87
	movw r0, #86
	cmp r1, r0
	ble main$L489
main$L362:
	movw r1, #88
	movw r0, #87
	cmp r1, r0
	ble main$L490
main$L365:
	movw r1, #89
	movw r0, #88
	cmp r1, r0
	ble main$L491
main$L368:
	movw r1, #90
	movw r0, #89
	cmp r1, r0
	ble main$L492
main$L371:
	movw r1, #91
	movw r0, #90
	cmp r1, r0
	ble main$L493
main$L374:
	movw r1, #92
	movw r0, #91
	cmp r1, r0
	ble main$L494
main$L377:
	movw r1, #93
	movw r0, #92
	cmp r1, r0
	ble main$L495
main$L380:
	movw r1, #94
	movw r0, #93
	cmp r1, r0
	ble main$L496
main$L383:
	movw r1, #95
	movw r0, #94
	cmp r1, r0
	ble main$L497
main$L386:
	movw r1, #96
	movw r0, #95
	cmp r1, r0
	ble main$L498
main$L389:
	movw r1, #97
	movw r0, #96
	cmp r1, r0
	ble main$L499
main$L392:
	movw r1, #98
	movw r0, #97
	cmp r1, r0
	ble main$L500
main$L395:
	movw r1, #99
	movw r0, #98
	cmp r1, r0
	ble main$L501
main$L398:
	movw r1, #100
	movw r0, #99
	cmp r1, r0
	ble main$L502
main$L399:
	mov r10, #1
	str r10, [fp, #-44]
	ldr r9, [fp, #-44]
	mov r10, r9
	str r10, [fp, #-44]
	b main$L400
main$L502:
	ldr r9, [fp, #-40]
	mov r10, r9
	str r10, [fp, #-44]
	b main$L400
main$L501:
	ldr r9, [fp, #-40]
	mov r10, r9
	str r10, [fp, #-44]
	b main$L400
main$L500:
	ldr r9, [fp, #-40]
	mov r10, r9
	str r10, [fp, #-44]
	b main$L400
main$L499:
	ldr r9, [fp, #-40]
	mov r10, r9
	str r10, [fp, #-44]
	b main$L400
main$L498:
	ldr r9, [fp, #-40]
	mov r10, r9
	str r10, [fp, #-44]
	b main$L400
main$L497:
	ldr r9, [fp, #-40]
	mov r10, r9
	str r10, [fp, #-44]
	b main$L400
main$L496:
	ldr r9, [fp, #-40]
	mov r10, r9
	str r10, [fp, #-44]
	b main$L400
main$L495:
	ldr r9, [fp, #-40]
	mov r10, r9
	str r10, [fp, #-44]
	b main$L400
main$L494:
	ldr r9, [fp, #-40]
	mov r10, r9
	str r10, [fp, #-44]
	b main$L400
main$L493:
	ldr r9, [fp, #-40]
	mov r10, r9
	str r10, [fp, #-44]
	b main$L400
main$L492:
	ldr r9, [fp, #-40]
	mov r10, r9
	str r10, [fp, #-44]
	b main$L400
main$L491:
	ldr r9, [fp, #-40]
	mov r10, r9
	str r10, [fp, #-44]
	b main$L400
main$L490:
	ldr r9, [fp, #-40]
	mov r10, r9
	str r10, [fp, #-44]
	b main$L400
main$L489:
	ldr r9, [fp, #-40]
	mov r10, r9
	str r10, [fp, #-44]
	b main$L400
main$L488:
	ldr r9, [fp, #-40]
	mov r10, r9
	str r10, [fp, #-44]
	b main$L400
main$L487:
	ldr r9, [fp, #-40]
	mov r10, r9
	str r10, [fp, #-44]
	b main$L400
main$L486:
	ldr r9, [fp, #-40]
	mov r10, r9
	str r10, [fp, #-44]
	b main$L400
main$L485:
	ldr r9, [fp, #-40]
	mov r10, r9
	str r10, [fp, #-44]
	b main$L400
main$L484:
	ldr r9, [fp, #-40]
	mov r10, r9
	str r10, [fp, #-44]
	b main$L400
main$L483:
	ldr r9, [fp, #-40]
	mov r10, r9
	str r10, [fp, #-44]
	b main$L400
main$L482:
	ldr r9, [fp, #-40]
	mov r10, r9
	str r10, [fp, #-44]
	b main$L400
main$L481:
	ldr r9, [fp, #-40]
	mov r10, r9
	str r10, [fp, #-44]
	b main$L400
main$L480:
	ldr r9, [fp, #-40]
	mov r10, r9
	str r10, [fp, #-44]
	b main$L400
main$L479:
	ldr r9, [fp, #-40]
	mov r10, r9
	str r10, [fp, #-44]
	b main$L400
main$L478:
	ldr r9, [fp, #-40]
	mov r10, r9
	str r10, [fp, #-44]
	b main$L400
main$L477:
	ldr r9, [fp, #-40]
	mov r10, r9
	str r10, [fp, #-44]
	b main$L400
main$L476:
	ldr r9, [fp, #-40]
	mov r10, r9
	str r10, [fp, #-44]
	b main$L400
main$L475:
	ldr r9, [fp, #-40]
	mov r10, r9
	str r10, [fp, #-44]
	b main$L400
main$L474:
	ldr r9, [fp, #-40]
	mov r10, r9
	str r10, [fp, #-44]
	b main$L400
main$L473:
	ldr r9, [fp, #-40]
	mov r10, r9
	str r10, [fp, #-44]
	b main$L400
main$L472:
	ldr r9, [fp, #-40]
	mov r10, r9
	str r10, [fp, #-44]
	b main$L400
main$L471:
	ldr r9, [fp, #-40]
	mov r10, r9
	str r10, [fp, #-44]
	b main$L400
main$L470:
	ldr r9, [fp, #-40]
	mov r10, r9
	str r10, [fp, #-44]
	b main$L400
main$L469:
	ldr r9, [fp, #-40]
	mov r10, r9
	str r10, [fp, #-44]
	b main$L400
main$L468:
	ldr r9, [fp, #-40]
	mov r10, r9
	str r10, [fp, #-44]
	b main$L400
main$L467:
	ldr r9, [fp, #-40]
	mov r10, r9
	str r10, [fp, #-44]
	b main$L400
main$L466:
	ldr r9, [fp, #-40]
	mov r10, r9
	str r10, [fp, #-44]
	b main$L400
main$L465:
	ldr r9, [fp, #-40]
	mov r10, r9
	str r10, [fp, #-44]
	b main$L400
main$L464:
	ldr r9, [fp, #-40]
	mov r10, r9
	str r10, [fp, #-44]
	b main$L400
main$L463:
	ldr r9, [fp, #-40]
	mov r10, r9
	str r10, [fp, #-44]
	b main$L400
main$L462:
	ldr r9, [fp, #-40]
	mov r10, r9
	str r10, [fp, #-44]
	b main$L400
main$L461:
	ldr r9, [fp, #-40]
	mov r10, r9
	str r10, [fp, #-44]
	b main$L400
main$L460:
	ldr r9, [fp, #-40]
	mov r10, r9
	str r10, [fp, #-44]
	b main$L400
main$L459:
	ldr r9, [fp, #-40]
	mov r10, r9
	str r10, [fp, #-44]
	b main$L400
main$L458:
	ldr r9, [fp, #-40]
	mov r10, r9
	str r10, [fp, #-44]
	b main$L400
main$L457:
	ldr r9, [fp, #-40]
	mov r10, r9
	str r10, [fp, #-44]
	b main$L400
main$L456:
	ldr r9, [fp, #-40]
	mov r10, r9
	str r10, [fp, #-44]
	b main$L400
main$L455:
	ldr r9, [fp, #-40]
	mov r10, r9
	str r10, [fp, #-44]
	b main$L400
main$L454:
	ldr r9, [fp, #-40]
	mov r10, r9
	str r10, [fp, #-44]
	b main$L400
main$L453:
	ldr r9, [fp, #-40]
	mov r10, r9
	str r10, [fp, #-44]
	b main$L400
main$L452:
	ldr r9, [fp, #-40]
	mov r10, r9
	str r10, [fp, #-44]
	b main$L400
main$L451:
	ldr r9, [fp, #-40]
	mov r10, r9
	str r10, [fp, #-44]
	b main$L400
main$L450:
	ldr r9, [fp, #-40]
	mov r10, r9
	str r10, [fp, #-44]
	b main$L400
main$L449:
	ldr r9, [fp, #-40]
	mov r10, r9
	str r10, [fp, #-44]
	b main$L400
main$L448:
	ldr r9, [fp, #-40]
	mov r10, r9
	str r10, [fp, #-44]
	b main$L400
main$L447:
	ldr r9, [fp, #-40]
	mov r10, r9
	str r10, [fp, #-44]
	b main$L400
main$L446:
	ldr r9, [fp, #-40]
	mov r10, r9
	str r10, [fp, #-44]
	b main$L400
main$L445:
	ldr r9, [fp, #-40]
	mov r10, r9
	str r10, [fp, #-44]
	b main$L400
main$L444:
	ldr r9, [fp, #-40]
	mov r10, r9
	str r10, [fp, #-44]
	b main$L400
main$L443:
	ldr r9, [fp, #-40]
	mov r10, r9
	str r10, [fp, #-44]
	b main$L400
main$L442:
	ldr r9, [fp, #-40]
	mov r10, r9
	str r10, [fp, #-44]
	b main$L400
main$L441:
	ldr r9, [fp, #-40]
	mov r10, r9
	str r10, [fp, #-44]
	b main$L400
main$L440:
	ldr r9, [fp, #-40]
	mov r10, r9
	str r10, [fp, #-44]
	b main$L400
main$L439:
	ldr r9, [fp, #-40]
	mov r10, r9
	str r10, [fp, #-44]
	b main$L400
main$L438:
	ldr r9, [fp, #-40]
	mov r10, r9
	str r10, [fp, #-44]
	b main$L400
main$L437:
	ldr r9, [fp, #-40]
	mov r10, r9
	str r10, [fp, #-44]
	b main$L400
main$L436:
	ldr r9, [fp, #-40]
	mov r10, r9
	str r10, [fp, #-44]
	b main$L400
main$L435:
	ldr r9, [fp, #-40]
	mov r10, r9
	str r10, [fp, #-44]
	b main$L400
main$L434:
	ldr r9, [fp, #-40]
	mov r10, r9
	str r10, [fp, #-44]
	b main$L400
main$L433:
	ldr r9, [fp, #-40]
	mov r10, r9
	str r10, [fp, #-44]
	b main$L400
main$L432:
	ldr r9, [fp, #-40]
	mov r10, r9
	str r10, [fp, #-44]
	b main$L400
main$L431:
	ldr r9, [fp, #-40]
	mov r10, r9
	str r10, [fp, #-44]
	b main$L400
main$L430:
	ldr r9, [fp, #-40]
	mov r10, r9
	str r10, [fp, #-44]
	b main$L400
main$L429:
	ldr r9, [fp, #-40]
	mov r10, r9
	str r10, [fp, #-44]
	b main$L400
main$L428:
	ldr r9, [fp, #-40]
	mov r10, r9
	str r10, [fp, #-44]
	b main$L400
main$L427:
	ldr r9, [fp, #-40]
	mov r10, r9
	str r10, [fp, #-44]
	b main$L400
main$L426:
	ldr r9, [fp, #-40]
	mov r10, r9
	str r10, [fp, #-44]
	b main$L400
main$L425:
	ldr r9, [fp, #-40]
	mov r10, r9
	str r10, [fp, #-44]
	b main$L400
main$L424:
	ldr r9, [fp, #-40]
	mov r10, r9
	str r10, [fp, #-44]
	b main$L400
main$L423:
	ldr r9, [fp, #-40]
	mov r10, r9
	str r10, [fp, #-44]
	b main$L400
main$L422:
	ldr r9, [fp, #-40]
	mov r10, r9
	str r10, [fp, #-44]
	b main$L400
main$L421:
	ldr r9, [fp, #-40]
	mov r10, r9
	str r10, [fp, #-44]
	b main$L400
main$L420:
	ldr r9, [fp, #-40]
	mov r10, r9
	str r10, [fp, #-44]
	b main$L400
main$L419:
	ldr r9, [fp, #-40]
	mov r10, r9
	str r10, [fp, #-44]
	b main$L400
main$L418:
	ldr r9, [fp, #-40]
	mov r10, r9
	str r10, [fp, #-44]
	b main$L400
main$L417:
	ldr r9, [fp, #-40]
	mov r10, r9
	str r10, [fp, #-44]
	b main$L400
main$L416:
	ldr r9, [fp, #-40]
	mov r10, r9
	str r10, [fp, #-44]
	b main$L400
main$L415:
	ldr r9, [fp, #-40]
	mov r10, r9
	str r10, [fp, #-44]
	b main$L400
main$L414:
	ldr r9, [fp, #-40]
	mov r10, r9
	str r10, [fp, #-44]
	b main$L400
main$L413:
	ldr r9, [fp, #-40]
	mov r10, r9
	str r10, [fp, #-44]
	b main$L400
main$L412:
	ldr r9, [fp, #-40]
	mov r10, r9
	str r10, [fp, #-44]
	b main$L400
main$L411:
	ldr r9, [fp, #-40]
	mov r10, r9
	str r10, [fp, #-44]
	b main$L400
main$L410:
	ldr r9, [fp, #-40]
	mov r10, r9
	str r10, [fp, #-44]
	b main$L400
main$L409:
	ldr r9, [fp, #-40]
	mov r10, r9
	str r10, [fp, #-44]
	b main$L400
main$L408:
	ldr r9, [fp, #-40]
	mov r10, r9
	str r10, [fp, #-44]
	b main$L400
main$L407:
	ldr r9, [fp, #-40]
	mov r10, r9
	str r10, [fp, #-44]
	b main$L400
main$L406:
	ldr r9, [fp, #-40]
	mov r10, r9
	str r10, [fp, #-44]
	b main$L400
main$L405:
	ldr r9, [fp, #-40]
	mov r10, r9
	str r10, [fp, #-44]
	b main$L400
main$L404:
	ldr r9, [fp, #-40]
	mov r10, r9
	str r10, [fp, #-44]
	b main$L400

.global malloc
.global getint
.global getch
.global getarray
.global putint
.global putch
.global putarray
.global starttime
.global stoptime
