.balign 4
.global main
.section .text
.arm
main:
main$L402:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
main$L104:
main$L107:
main$L110:
main$L113:
main$L116:
main$L119:
main$L122:
main$L125:
main$L128:
main$L131:
main$L134:
main$L137:
main$L140:
main$L143:
main$L146:
main$L149:
main$L152:
main$L155:
main$L158:
main$L161:
main$L164:
main$L165:
	bl getint
	movw r1, #0
	cmp r0, r1
	bne main$L170
main$L400:
main$L401:
	movw r0, #0
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr
main$L170:
main$L173:
main$L176:
main$L179:
main$L182:
main$L185:
main$L188:
main$L191:
main$L194:
main$L197:
main$L200:
main$L203:
main$L206:
main$L209:
main$L212:
main$L215:
main$L218:
main$L221:
main$L224:
main$L227:
main$L230:
main$L233:
main$L236:
main$L239:
main$L242:
main$L245:
main$L248:
main$L251:
main$L254:
main$L257:
main$L260:
main$L263:
main$L266:
main$L269:
main$L272:
main$L275:
main$L278:
main$L281:
main$L284:
main$L287:
main$L290:
main$L293:
main$L296:
main$L299:
main$L302:
main$L305:
main$L308:
main$L311:
main$L314:
main$L317:
main$L320:
main$L323:
main$L326:
main$L329:
main$L332:
main$L335:
main$L338:
main$L341:
main$L344:
main$L347:
main$L350:
main$L353:
main$L356:
main$L359:
main$L362:
main$L365:
main$L368:
main$L371:
main$L374:
main$L377:
main$L380:
main$L383:
main$L386:
main$L389:
main$L392:
main$L395:
main$L398:
main$L399:
	movw r0, #100
	bl putint
	movw r0, #10
	bl putch
	b main$L401

.global malloc
.global getint
.global getch
.global getarray
.global putint
.global putch
.global putarray
.global starttime
.global stoptime
