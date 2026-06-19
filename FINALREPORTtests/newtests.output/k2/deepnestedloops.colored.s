.balign 4
.global main
.section .text
.arm
main:
main$L595:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
main$L102:
	movw r1, #2
	movw r0, #1
	cmp r1, r0
	bgt main$L103
main$L104:
	movw r0, #100
	bl putint
	movw r0, #10
	bl putch
	movw r0, #0
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr
main$L103:
main$L107:
	movw r1, #3
	movw r0, #2
	cmp r1, r0
	bgt main$L108
main$L109:
	b main$L104
main$L108:
main$L112:
	movw r1, #4
	movw r0, #3
	cmp r1, r0
	bgt main$L113
main$L114:
	b main$L109
main$L113:
main$L117:
	movw r1, #5
	movw r0, #4
	cmp r1, r0
	bgt main$L118
main$L119:
	b main$L114
main$L118:
main$L122:
	movw r1, #6
	movw r0, #5
	cmp r1, r0
	bgt main$L123
main$L124:
	b main$L119
main$L123:
main$L127:
	movw r1, #7
	movw r0, #6
	cmp r1, r0
	bgt main$L128
main$L129:
	b main$L124
main$L128:
main$L132:
	movw r1, #8
	movw r0, #7
	cmp r1, r0
	bgt main$L133
main$L134:
	b main$L129
main$L133:
main$L137:
	movw r1, #9
	movw r0, #8
	cmp r1, r0
	bgt main$L138
main$L139:
	b main$L134
main$L138:
main$L142:
	movw r1, #10
	movw r0, #9
	cmp r1, r0
	bgt main$L143
main$L144:
	b main$L139
main$L143:
main$L147:
	movw r1, #11
	movw r0, #10
	cmp r1, r0
	bgt main$L148
main$L149:
	b main$L144
main$L148:
main$L152:
	movw r1, #12
	movw r0, #11
	cmp r1, r0
	bgt main$L153
main$L154:
	b main$L149
main$L153:
main$L157:
	movw r1, #13
	movw r0, #12
	cmp r1, r0
	bgt main$L158
main$L159:
	b main$L154
main$L158:
main$L162:
	movw r1, #14
	movw r0, #13
	cmp r1, r0
	bgt main$L163
main$L164:
	b main$L159
main$L163:
main$L167:
	movw r1, #15
	movw r0, #14
	cmp r1, r0
	bgt main$L168
main$L169:
	b main$L164
main$L168:
main$L172:
	movw r1, #16
	movw r0, #15
	cmp r1, r0
	bgt main$L173
main$L174:
	b main$L169
main$L173:
main$L177:
	movw r1, #17
	movw r0, #16
	cmp r1, r0
	bgt main$L178
main$L179:
	b main$L174
main$L178:
main$L182:
	movw r1, #18
	movw r0, #17
	cmp r1, r0
	bgt main$L183
main$L184:
	b main$L179
main$L183:
main$L187:
	movw r1, #19
	movw r0, #18
	cmp r1, r0
	bgt main$L188
main$L189:
	b main$L184
main$L188:
main$L192:
	movw r1, #20
	movw r0, #19
	cmp r1, r0
	bgt main$L193
main$L194:
	b main$L189
main$L193:
main$L197:
	movw r1, #21
	movw r0, #20
	cmp r1, r0
	bgt main$L198
main$L199:
	b main$L194
main$L198:
main$L202:
	movw r1, #22
	movw r0, #21
	cmp r1, r0
	bgt main$L203
main$L204:
	b main$L199
main$L203:
main$L207:
	movw r1, #23
	movw r0, #22
	cmp r1, r0
	bgt main$L208
main$L209:
	b main$L204
main$L208:
main$L212:
	movw r1, #24
	movw r0, #23
	cmp r1, r0
	bgt main$L213
main$L214:
	b main$L209
main$L213:
main$L217:
	movw r1, #25
	movw r0, #24
	cmp r1, r0
	bgt main$L218
main$L219:
	b main$L214
main$L218:
main$L222:
	movw r1, #26
	movw r0, #25
	cmp r1, r0
	bgt main$L223
main$L224:
	b main$L219
main$L223:
main$L227:
	movw r1, #27
	movw r0, #26
	cmp r1, r0
	bgt main$L228
main$L229:
	b main$L224
main$L228:
main$L232:
	movw r1, #28
	movw r0, #27
	cmp r1, r0
	bgt main$L233
main$L234:
	b main$L229
main$L233:
main$L237:
	movw r1, #29
	movw r0, #28
	cmp r1, r0
	bgt main$L238
main$L239:
	b main$L234
main$L238:
main$L242:
	movw r1, #30
	movw r0, #29
	cmp r1, r0
	bgt main$L243
main$L244:
	b main$L239
main$L243:
main$L247:
	movw r1, #31
	movw r0, #30
	cmp r1, r0
	bgt main$L248
main$L249:
	b main$L244
main$L248:
main$L252:
	movw r1, #32
	movw r0, #31
	cmp r1, r0
	bgt main$L253
main$L254:
	b main$L249
main$L253:
main$L257:
	movw r1, #33
	movw r0, #32
	cmp r1, r0
	bgt main$L258
main$L259:
	b main$L254
main$L258:
main$L262:
	movw r1, #34
	movw r0, #33
	cmp r1, r0
	bgt main$L263
main$L264:
	b main$L259
main$L263:
main$L267:
	movw r1, #35
	movw r0, #34
	cmp r1, r0
	bgt main$L268
main$L269:
	b main$L264
main$L268:
main$L272:
	movw r1, #36
	movw r0, #35
	cmp r1, r0
	bgt main$L273
main$L274:
	b main$L269
main$L273:
main$L277:
	movw r1, #37
	movw r0, #36
	cmp r1, r0
	bgt main$L278
main$L279:
	b main$L274
main$L278:
main$L282:
	movw r1, #38
	movw r0, #37
	cmp r1, r0
	bgt main$L283
main$L284:
	b main$L279
main$L283:
main$L287:
	movw r1, #39
	movw r0, #38
	cmp r1, r0
	bgt main$L288
main$L289:
	b main$L284
main$L288:
main$L292:
	movw r1, #40
	movw r0, #39
	cmp r1, r0
	bgt main$L293
main$L294:
	b main$L289
main$L293:
main$L297:
	movw r1, #41
	movw r0, #40
	cmp r1, r0
	bgt main$L298
main$L299:
	b main$L294
main$L298:
main$L302:
	movw r1, #42
	movw r0, #41
	cmp r1, r0
	bgt main$L303
main$L304:
	b main$L299
main$L303:
main$L307:
	movw r1, #43
	movw r0, #42
	cmp r1, r0
	bgt main$L308
main$L309:
	b main$L304
main$L308:
main$L312:
	movw r1, #44
	movw r0, #43
	cmp r1, r0
	bgt main$L313
main$L314:
	b main$L309
main$L313:
main$L317:
	movw r1, #45
	movw r0, #44
	cmp r1, r0
	bgt main$L318
main$L319:
	b main$L314
main$L318:
main$L322:
	movw r1, #46
	movw r0, #45
	cmp r1, r0
	bgt main$L323
main$L324:
	b main$L319
main$L323:
main$L327:
	movw r1, #47
	movw r0, #46
	cmp r1, r0
	bgt main$L328
main$L329:
	b main$L324
main$L328:
main$L332:
	movw r1, #48
	movw r0, #47
	cmp r1, r0
	bgt main$L333
main$L334:
	b main$L329
main$L333:
main$L337:
	movw r1, #49
	movw r0, #48
	cmp r1, r0
	bgt main$L338
main$L339:
	b main$L334
main$L338:
main$L342:
	movw r1, #50
	movw r0, #49
	cmp r1, r0
	bgt main$L343
main$L344:
	b main$L339
main$L343:
main$L347:
	movw r1, #51
	movw r0, #50
	cmp r1, r0
	bgt main$L348
main$L349:
	b main$L344
main$L348:
main$L352:
	movw r1, #52
	movw r0, #51
	cmp r1, r0
	bgt main$L353
main$L354:
	b main$L349
main$L353:
main$L357:
	movw r1, #53
	movw r0, #52
	cmp r1, r0
	bgt main$L358
main$L359:
	b main$L354
main$L358:
main$L362:
	movw r1, #54
	movw r0, #53
	cmp r1, r0
	bgt main$L363
main$L364:
	b main$L359
main$L363:
main$L367:
	movw r1, #55
	movw r0, #54
	cmp r1, r0
	bgt main$L368
main$L369:
	b main$L364
main$L368:
main$L372:
	movw r1, #56
	movw r0, #55
	cmp r1, r0
	bgt main$L373
main$L374:
	b main$L369
main$L373:
main$L377:
	movw r1, #57
	movw r0, #56
	cmp r1, r0
	bgt main$L378
main$L379:
	b main$L374
main$L378:
main$L382:
	movw r1, #58
	movw r0, #57
	cmp r1, r0
	bgt main$L383
main$L384:
	b main$L379
main$L383:
main$L387:
	movw r1, #59
	movw r0, #58
	cmp r1, r0
	bgt main$L388
main$L389:
	b main$L384
main$L388:
main$L392:
	movw r1, #60
	movw r0, #59
	cmp r1, r0
	bgt main$L393
main$L394:
	b main$L389
main$L393:
main$L397:
	movw r1, #61
	movw r0, #60
	cmp r1, r0
	bgt main$L398
main$L399:
	b main$L394
main$L398:
main$L402:
	movw r1, #62
	movw r0, #61
	cmp r1, r0
	bgt main$L403
main$L404:
	b main$L399
main$L403:
main$L407:
	movw r1, #63
	movw r0, #62
	cmp r1, r0
	bgt main$L408
main$L409:
	b main$L404
main$L408:
main$L412:
	movw r1, #64
	movw r0, #63
	cmp r1, r0
	bgt main$L413
main$L414:
	b main$L409
main$L413:
main$L417:
	movw r1, #65
	movw r0, #64
	cmp r1, r0
	bgt main$L418
main$L419:
	b main$L414
main$L418:
main$L422:
	movw r1, #66
	movw r0, #65
	cmp r1, r0
	bgt main$L423
main$L424:
	b main$L419
main$L423:
main$L427:
	movw r1, #67
	movw r0, #66
	cmp r1, r0
	bgt main$L428
main$L429:
	b main$L424
main$L428:
main$L432:
	movw r1, #68
	movw r0, #67
	cmp r1, r0
	bgt main$L433
main$L434:
	b main$L429
main$L433:
main$L437:
	movw r1, #69
	movw r0, #68
	cmp r1, r0
	bgt main$L438
main$L439:
	b main$L434
main$L438:
main$L442:
	movw r1, #70
	movw r0, #69
	cmp r1, r0
	bgt main$L443
main$L444:
	b main$L439
main$L443:
main$L447:
	movw r1, #71
	movw r0, #70
	cmp r1, r0
	bgt main$L448
main$L449:
	b main$L444
main$L448:
main$L452:
	movw r1, #72
	movw r0, #71
	cmp r1, r0
	bgt main$L453
main$L454:
	b main$L449
main$L453:
main$L457:
	movw r1, #73
	movw r0, #72
	cmp r1, r0
	bgt main$L458
main$L459:
	b main$L454
main$L458:
main$L462:
	movw r1, #74
	movw r0, #73
	cmp r1, r0
	bgt main$L463
main$L464:
	b main$L459
main$L463:
main$L467:
	movw r1, #75
	movw r0, #74
	cmp r1, r0
	bgt main$L468
main$L469:
	b main$L464
main$L468:
main$L472:
	movw r1, #76
	movw r0, #75
	cmp r1, r0
	bgt main$L473
main$L474:
	b main$L469
main$L473:
main$L477:
	movw r1, #77
	movw r0, #76
	cmp r1, r0
	bgt main$L478
main$L479:
	b main$L474
main$L478:
main$L482:
	movw r1, #78
	movw r0, #77
	cmp r1, r0
	bgt main$L483
main$L484:
	b main$L479
main$L483:
main$L487:
	movw r1, #79
	movw r0, #78
	cmp r1, r0
	bgt main$L488
main$L489:
	b main$L484
main$L488:
main$L492:
	movw r1, #80
	movw r0, #79
	cmp r1, r0
	bgt main$L493
main$L494:
	b main$L489
main$L493:
main$L497:
	movw r1, #81
	movw r0, #80
	cmp r1, r0
	bgt main$L498
main$L499:
	b main$L494
main$L498:
main$L502:
	movw r1, #82
	movw r0, #81
	cmp r1, r0
	bgt main$L503
main$L504:
	b main$L499
main$L503:
main$L507:
	movw r1, #83
	movw r0, #82
	cmp r1, r0
	bgt main$L508
main$L509:
	b main$L504
main$L508:
main$L512:
	movw r1, #84
	movw r0, #83
	cmp r1, r0
	bgt main$L513
main$L514:
	b main$L509
main$L513:
main$L517:
	movw r1, #85
	movw r0, #84
	cmp r1, r0
	bgt main$L518
main$L519:
	b main$L514
main$L518:
main$L522:
	movw r1, #86
	movw r0, #85
	cmp r1, r0
	bgt main$L523
main$L524:
	b main$L519
main$L523:
main$L527:
	movw r1, #87
	movw r0, #86
	cmp r1, r0
	bgt main$L528
main$L529:
	b main$L524
main$L528:
main$L532:
	movw r1, #88
	movw r0, #87
	cmp r1, r0
	bgt main$L533
main$L534:
	b main$L529
main$L533:
main$L537:
	movw r1, #89
	movw r0, #88
	cmp r1, r0
	bgt main$L538
main$L539:
	b main$L534
main$L538:
main$L542:
	movw r1, #90
	movw r0, #89
	cmp r1, r0
	bgt main$L543
main$L544:
	b main$L539
main$L543:
main$L547:
	movw r1, #91
	movw r0, #90
	cmp r1, r0
	bgt main$L548
main$L549:
	b main$L544
main$L548:
main$L552:
	movw r1, #92
	movw r0, #91
	cmp r1, r0
	bgt main$L553
main$L554:
	b main$L549
main$L553:
main$L557:
	movw r1, #93
	movw r0, #92
	cmp r1, r0
	bgt main$L558
main$L559:
	b main$L554
main$L558:
main$L562:
	movw r1, #94
	movw r0, #93
	cmp r1, r0
	bgt main$L563
main$L564:
	b main$L559
main$L563:
main$L567:
	movw r1, #95
	movw r0, #94
	cmp r1, r0
	bgt main$L568
main$L569:
	b main$L564
main$L568:
main$L572:
	movw r1, #96
	movw r0, #95
	cmp r1, r0
	bgt main$L573
main$L574:
	b main$L569
main$L573:
main$L577:
	movw r1, #97
	movw r0, #96
	cmp r1, r0
	bgt main$L578
main$L579:
	b main$L574
main$L578:
main$L582:
	movw r1, #98
	movw r0, #97
	cmp r1, r0
	bgt main$L583
main$L584:
	b main$L579
main$L583:
main$L587:
	movw r1, #99
	movw r0, #98
	cmp r1, r0
	bgt main$L588
main$L589:
	b main$L584
main$L588:
main$L592:
	movw r1, #100
	movw r0, #99
	cmp r1, r0
	bgt main$L593
main$L594:
	b main$L589
main$L593:
	b main$L594

.global malloc
.global getint
.global getch
.global getarray
.global putint
.global putch
.global putarray
.global starttime
.global stoptime
