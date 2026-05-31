---
title: "HW11 实验报告"
author: "王思宇"
date: \today
using_title: true
using_table_of_content: true
---

# HW11 实验报告

**重要：请助教仔细看一下实验结果部分。我的实现与预期结果存在temp编号不同，指令调度顺序不同（不影响数据流）和函数顺序不同的差异，并且在一个测试用例上还比预期输出选择指令选的更好。实验结果中有详细说明。**

本次作业实现从 SSA Quad 到 ARMv7-A 汇编的后端指令选择和线性化。输入是 `.4-ssa-withflow-xml.quad`，输出是使用无限量 temp 的 `.s` 文件。

## 参考资料

虎书第 9、10 章。主要参考了数据依赖图、指令选择和过程调用约定的基本思想。

## advDFG 构造

`buildAdvDFG.cc` 中为每个函数、每个基本块构造一个 `advDFGblock`。每个块首先建立 `EntryLabel` 节点，然后跳过 `LABEL` 和 `PHI`，把普通语句和终结语句建成图节点。

每个节点记录以下信息：

- `tempDefined`：语句定义的第一个 temp，没有定义则为 `-1`。
- `tempsUsed`：语句使用的 temp 编号集合。
- `chainDefined` 和 `chainUsed`：用于表示内存访问和调用的顺序依赖。
- `predecessors` 和 `successors`：由 def-use 依赖和 chain 依赖共同产生。

实现时维护 `lastTempDef`，把每个 use 连到最近一次定义该 temp 的节点；如果 temp 来自块外，则连到 entry 节点。对于 `LOAD`、`STORE`、`CALL`、`EXTCALL`、`MOVE_CALL`、`MOVE_EXTCALL` 和 `RETURN`，额外维护一条 chain，保证内存和调用副作用不会被错误重排。

## 指令选择

`selectInstr.cc` 中实现了 `selectInstructionsForBlock()`，按照 advDFG 节点的插入顺序处理非终结语句，生成 `preScheduleBlock::selectedInstructions`。

主要翻译规则如下：

- `MOVE` 翻译成 `mov`。如果源和目标 temp 编号相同，则省略。
- 常量通过 `movw` 载入；高 16 位非零时追加 `movt`。这样可以正确处理 `-1` 等 32 位立即数。
- `NAME` 翻译成 `adr`，用于函数表项等标签地址。
- `LOAD` 和 `STORE` 翻译成 `ldr` 和 `str`，地址先物化成 temp。
- `MOVE_BINOP` 翻译成 `add`、`sub`、`mul` 或 `sdiv`。
- `PTR_CALC` 翻译成 `add`，把 base 和 offset 相加。
- 外部函数调用和普通函数调用按 ARM 约定把参数放入 `r0`、`r1`、`r2`、`r3`，再生成 `bl`；带对象函数指针的调用使用 `blx`。
- 带返回值的调用把 `r0` 搬到目标 temp。

指令选择阶段不处理 `JUMP`、`CJUMP`、`RETURN` 和 `PHI`，这些语句留给 schedule 阶段统一处理。

## 指令线性化

`schedule.cc` 中实现 `scheduleProg()`，把每个函数的 `preScheduleBlock` 线性化成最终 `ScheduleFunc`。

函数开头统一插入：

```text
push {r4-r10, fp, lr}
sub sp, sp, #4
add fp, sp, #36
```

如果函数有参数，则把 `r0` 到 `r3` 搬到对应参数 temp。函数返回时把返回表达式物化到 temp，再搬到 `r0`，最后插入统一 epilogue：

```text
sub sp, fp, #36
add sp, sp, #4
pop {r4-r10, fp, lr}
bx lr
```

线性化采用从 block list 开始的递归 DFS。每个 block 只输出一次标签和本块已选择指令。对于 `JUMP`，在跳转边上插入目标块 PHI 对应的拷贝；如果目标块尚未输出，则直接继续输出目标块，否则生成显式 `b`。

对于 `CJUMP`，先生成 `cmp` 和条件跳转。默认把 false 分支作为 fall-through，true 分支作为条件跳转目标。如果 true 目标块上有来自当前块的 PHI 拷贝，则新增一个边标签：条件跳转先跳到边标签，在边标签处执行 PHI 拷贝，再进入 true 目标块。这样避免条件跳转直接绕过 PHI 拷贝。

## Git 提交记录

```text
30c8af3 Implement HW11 instruction scheduling
65946c7 Merge branches 'master' and 'master' of gitee.com:fudanCompiler/fducompilerh2026
818df73 HW11 added
ca19001 revised report
f186162 Keep HW10 plan alignment within allowed sources
4bdfad4 Align HW10 IV reports with reference comments
2957eba Merge branch 'master' of gitee.com:fudanCompiler/fducompilerh2026
cead473 HW10 fin
```

## 测试结果

独立构建通过：

```text
ninja: Entering directory `/tmp/hw11build'
[1/3] Building CXX object lib/instr/CMakeFiles/instr.dir/buildAdvDFG.cc.o
[2/3] Building CXX object lib/instr/CMakeFiles/instr.dir/selectInstr.cc.o
[3/3] Linking CXX executable tools/main/main
```

隔离运行 12 个官方输入均通过：

```text
Running bubblesort
Running insttest0
Running insttest1
Running insttest2
Running insttest3
Running insttest4
Running optloopivtest1
Running optloopivtest2
Running optloopivtest3
Running optloopivtest4
Running optloopivtest5
Running optloopivtest6
Generated assembly files:
12
```

随后将生成的 `.s` 与仓库中的参考 `.s` 逐个比较。完全一致的文件如下：

```text
MATCH insttest0.s
MATCH optloopivtest1.s
MATCH optloopivtest3.s
MATCH optloopivtest4.s
MATCH optloopivtest6.s
```

仍存在文本差异但语义等价的文件如下：

```text
DIFF bubblesort.s
DIFF insttest1.s
DIFF insttest2.s
DIFF insttest3.s
DIFF insttest4.s
DIFF optloopivtest2.s
DIFF optloopivtest5.s
```

这些差异主要分为三类：

1. 函数输出顺序不同。例如 `bubblesort.s` 中参考输出先输出 `__$main__^main`，当前输出先输出 `b1^bubbleSort`。函数之间没有顺序依赖，因此不影响汇编语义。
2. 独立指令调度顺序不同。例如 `optloopivtest2.s` 和 `optloopivtest5.s` 中，参考输出会先计算回边更新 temp，再执行 `putint` 和 `putch`；当前输出先执行输出调用，再在回边 PHI 拷贝前计算该 temp。该 temp 只在回边处使用，因此语义一致。
3. temp 编号和部分中间地址 temp 不同。当前实现会把 `PTR_CALC` 后唯一用于 `LOAD` 或 `STORE` 的地址进一步折叠到 ARM 寻址模式中，例如生成 `str t16300, [t12600, t16000]` 或 `str t148, [t10000, #24]`，而参考输出有时保留显式地址 temp。这个差异属于指令选择更充分使用 ARM addressing mode，不改变语义。

因此，当前实现已经覆盖 `str/ldr [base, #offset]`、`str/ldr [base, index]`、`add/sub #imm` 和小常量 `mov #imm` 等常见 ARM 指令选择形式；剩余 diff 不是由于没有使用 offset/index 寻址造成的。

逐测试用例差异说明如下：

- `bubblesort.s`：如果忽略函数输出顺序，两个函数内部仍有若干调度差异。`__$main__^main` 中参考输出先计算 `t12800 = t10000 + 24` 和 `t12900 = t10000 + 28`，再用 `str [t12800]` 和 `str [t12900]`；当前实现直接生成 `str t148, [t10000, #24]` 和 `str t149, [t10000, #28]`，实际上比参考输出指令选择做的好。`b1^bubbleSort` 中也有类似差异，例如参考先保留地址中间 temp，当前实现生成 `str t16300, [t12600, t16000]` 这样的 indexed addressing。其余差异主要是独立语句调度顺序不同，例如 `mov t13100, t12700`、`mov t12200, t102`、若干 `mul` 和 `ldr` 的先后顺序不同，以及由此引起的常量 temp 编号不同。

- `insttest1.s`：指令形态和控制流一致，差异集中在常量 temp 编号。例如参考使用 `t132` 保存 `0`、`t131` 保存 `10`、`t133` 保存 `2`，当前实现对应 temp 为 `t134`、`t133`、`t135`。`exit(-1)`、乘法常量 `4`、输出空格 `32` 的 temp 编号也不同。语义不变。

- `insttest2.s`：初始化阶段的独立语句顺序不同。参考较早生成 `mov t10100, #3`、`mov t12100, t10300` 和 `mov t10000, t10300`，当前实现先完成数组内容 store，再生成这些 move。后续循环部分的差异仍主要是常量 temp 编号不同，例如比较用 `0`、换行 `10`、返回值 `2`、`exit(-1)`、乘法常量 `4` 和空格 `32` 的 temp 编号不同。

- `insttest3.s`：函数 `C^m` 完全一致。`__$main__^main` 中参考保留 `add t12500, t10400, #4` 后执行 `str t141, [t12500]`，当前实现直接执行 `str t142, [t10400, #4]`；这是更充分使用 offset addressing 的差异。参考还会更早生成 `mov t10200, #3` 和 `mov t10600, t10400`，当前实现稍后生成。循环部分主要是常量 temp 编号不同。

- `insttest4.s`：函数 `C^m` 完全一致。`__$main__^main` 中参考提前生成 `t13600 = t10300 + 4`、`t13700 = t10300 + 8`、`t13800 = t10600 + 8` 等地址 temp，再通过这些 temp 做 store/load；当前实现直接生成 `str t10500, [t10300, #4]`、`str t162, [t10300, #8]` 和 `ldr t11200, [t10600, #8]`。这是 offset addressing 折叠造成的文本差异。后续循环部分仍是常量 temp 编号不同。

- `optloopivtest2.s`：只有回边更新语句位置不同。参考在输出 derived IV 结果前计算 `sub t10002, t10001, #2`，当前实现把这条语句放在 `putint` 和 `putch` 之后、回边 PHI 拷贝之前。`t10002` 只在回边更新 `t10001` 时使用，因此语义一致。

- `optloopivtest5.s`：和 `optloopivtest2.s` 类似。参考先生成 `sub t10202, t10201, #1`，再计算并输出 `3 * t10201 + 2`；当前实现先计算输出值并调用 `putint`、`putch`，再生成 `sub t10202, t10201, #1` 并进行回边 PHI 拷贝。该 temp 只用于回边更新，因此语义一致。

额外测试也通过：

```text
Running extra_phi_true_edge
Running extra_relops
All extra HW11 tests passed.
```
