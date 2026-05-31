---
title: "HW11 实验报告"
author: "王思宇"
date: \today
using_title: true
using_table_of_content: true
---

# HW11 实验报告

**重要：请助教仔细看一下实验结果部分。我的实现与预期结果存在temp编号不同，指令调度顺序不同（不影响数据流）和函数顺序不同的差异。老师给的预期结果没有实现完备的 1. ptr_calc折叠  2. 常量复用。 实验结果部分中有详细说明。**

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

实现时维护 `lastTempDef`，把每个 use 连到最近一次定义该 temp 的节点；如果 temp 来自块外，则连到 entry 节点。每个 statement 也都连到 entry 节点，用来表示 block 入口对语句的基本控制约束。

对于 `LOAD`、`STORE`、`CALL`、`EXTCALL`、`MOVE_CALL` 和 `MOVE_EXTCALL`，额外维护一条 chain token。前一个有内存或调用副作用的语句定义 token，后一个语句使用 token，从而保证副作用语句之间不会被错误重排。

构造完一个基本块后，还会找到该块的最后语句。如果存在 `JUMP`、`CJUMP` 或 `RETURN`，则使用这个终结语句作为 last statement；否则使用块内最后一个普通语句。随后从每个其它 statement 向 last statement 加边，保证最后语句必须在块内其它语句之后被考虑。

## 指令选择

`selectInstr.cc` 中实现了 `selectInstructionsForBlock()`，现在主路径直接基于 advDFG 做 greedy tiling。每个块维护一个 `covered` 集合，entry 节点一开始被标记为 covered。之后反复按节点插入顺序扫描图，只有当一个节点的所有前驱都已经 covered 时，才允许对它做 tile selection 并发射指令。

对于普通语句，选择器调用 `selectStatement()` 发射对应 ARM 指令并把节点标记为 covered。对于 `PTR_CALC`，如果它定义的地址 temp 在本块内只有一次 use，且这个 use 是某个 `LOAD` 或 `STORE` 的地址操作数，则先不单独发射 `add`，而是把这个 `PTR_CALC` 记录为可折叠 tile。当后续对应的 memory 节点变为 ready 时，一次性发射 `ldr/str [base, #offset]` 或 `ldr/str [base, index]`，并同时覆盖 memory 节点。这样实现了 PPT 中“find-and-emit”式的 greedy tiling。

常量没有单独落成可发射节点，而是作为 tile 内的叶子处理。如果某个常量能进入 ARM immediate 字段，就直接生成 immediate 形式；否则通过 `movw` 和必要时的 `movt` 物化到临时寄存器。

主要翻译规则如下：

- `MOVE` 翻译成 `mov`。如果源和目标 temp 编号相同，则省略。
- 常量通过 `movw` 载入；高 16 位非零时追加 `movt`。这样可以正确处理 `-1` 等 32 位立即数。
- `NAME` 翻译成 `adr`，用于函数表项等标签地址。
- `LOAD` 和 `STORE` 翻译成 `ldr` 和 `str`，地址先物化成 temp。
- `MOVE_BINOP` 翻译成 `add`、`sub`、`mul` 或 `sdiv`。
- `PTR_CALC` 翻译成 `add`，把 base 和 offset 相加。如果可以不翻译就不翻。（直接填入`str`，`ldr`的offset）
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
e9a259a Use advDFG traversal for HW11 instruction selection
6824695 Refresh HW11 test result report
d3494d9 Merge branch 'master' of gitee.com:fudanCompiler/fducompilerh2026
4182231 Detail HW11 assembly diffs by test
0514985 HW12 added
980620d Document HW11 assembly output differences
801cb2d HW11: test updated
fe13725 HW11: test updated
c970a23 HW11: test files updated
748bae7 Merge branch 'master' of https://forgejo.dywsy21.cn:18080/dywsy21/Compiler-H
```

## 测试结果

运行 13 个测试均通过：

```text
Running bubblesort
Running fibonacci
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
13
```

随后将生成的 `.s` 与仓库中的参考 `.s` 逐个比较。完全一致的文件如下：

```text
MATCH optloopivtest1.s
MATCH optloopivtest3.s
MATCH optloopivtest4.s
MATCH optloopivtest6.s
```

仍存在文本差异但语义等价的文件如下：

```text
DIFF bubblesort.s
DIFF fibonacci.s
DIFF insttest0.s
DIFF insttest1.s
DIFF insttest2.s
DIFF insttest3.s
DIFF insttest4.s
DIFF optloopivtest2.s
DIFF optloopivtest5.s
```

这些差异主要分为五类：

1. 函数输出顺序不同。例如 `bubblesort.s` 中参考输出先输出 `__$main__^main`，当前输出先输出 `b1^bubbleSort`。函数之间没有顺序依赖，因此不影响汇编语义。
2. 独立指令调度顺序不同。例如 `optloopivtest2.s` 和 `optloopivtest5.s` 中，参考输出会先计算回边更新 temp，再执行 `putint` 和 `putch`；当前输出先执行输出调用，再在回边 PHI 拷贝前计算该 temp。该 temp 只在回边处使用，因此语义一致。
3. temp 编号和部分中间地址 temp 不同。当前实现会把 `PTR_CALC` 后唯一用于 `LOAD` 或 `STORE` 的地址进一步折叠到 ARM 寻址模式中，例如生成 `str t16300, [t12600, t16000]` 或 `str t148, [t10000, #24]`，而参考输出有时保留显式地址 temp。这个差异属于指令选择更充分使用 ARM addressing mode，不改变语义。
4. 标签地址物化方式不同。pull 后部分参考 `.s` 使用 `ldr t, =label` 伪指令，当前实现仍使用 `adr t, label`。两者都用于把标签地址放入 temp，在当前测试范围内语义一致。
5. 预期输出中，没有做常量复用。

逐测试用例差异说明如下：

- `bubblesort.s`：如果忽略函数输出顺序，两个函数内部仍有若干调度差异。`__$main__^main` 中参考输出先生成 `mov t10100, #0` 和 `mov t10200, #0`，当前实现把这两个初始化放在数组 store 之后。参考使用 `ldr t151, =b1^bubbleSort` 物化函数标签地址，当前实现使用 `adr t151, b1^bubbleSort`。循环体中有独立语句调度差异，例如 `add t10202, t10201, #1` 被放在输出调用之后、若干 `mul` 和 `ldr` 的先后顺序不同。当前实现仍会使用 `str t148, [t10000, #24]`、`str t149, [t10000, #28]` 和 `str t16300, [t12600, t16000]` 这样的 offset/index addressing。

- `fibonacci.s`：`__$main__^main` 中参考先分配对象再初始化 `t10200`、`t10000`，当前实现先初始化这两个 temp；参考使用 `ldr t172, =fib^f`，当前实现使用 `adr t172, fib^f`。打印提示字符串时，当前实现会复用已经装载过的字符常量 temp，例如空格、`t`、`e`、`r`、`n`，所以比参考少若干 `movw`，但输出字符序列一致。`fib^f` 中递归调用前后存在独立 move/load 的调度顺序差异，例如 `mov t10600, t10300` 和第二次递归调用参数准备的先后顺序不同，调用目标、参数和返回值组合保持一致。

- `insttest0.s`：`C^max` 函数完全一致。`__$main__^main` 只有标签地址物化方式不同：参考为 `ldr t113, =C^max`，当前实现为 `adr t113, C^max`，后续都将该地址写入对象方法表并通过 `blx` 调用，语义一致。

- `insttest1.s`：指令形态和控制流一致，差异集中在常量 temp 编号以及常量复用。pull 后参考会重新生成 `movw t129, #4` 再 store 数组长度末项，当前实现复用前面已经保存 `4` 的 temp `t125`。后续比较用 `0`、换行 `10`、返回值 `2`、`exit(-1)`、乘法常量 `4` 和空格 `32` 的 temp 编号不同，语义不变。

- `insttest2.s`：初始化阶段的独立语句顺序不同。参考较早生成 `mov t10100, #3`、`mov t12100, t10300` 和 `mov t10000, t10300`，当前实现先完成部分数组内容 store，再生成这些 move。当前实现也复用保存 `4` 的 temp `t130` 来写数组长度。循环部分差异仍主要是常量 temp 编号不同。

- `insttest3.s`：函数 `C^m` 完全一致。`__$main__^main` 中参考保留 `add t12500, t10400, #4` 后执行 `str t142, [t12500]`，当前实现直接执行 `str t142, [t10400, #4]`；这是更充分使用 offset addressing 的差异。参考使用 `ldr t142, =C^m`，当前实现使用 `adr t142, C^m`。参考还会更早生成 `mov t10200, #3` 和 `mov t10600, t10400`，当前实现稍后生成。循环部分主要是常量 temp 编号不同。

- `insttest4.s`：函数 `C^m` 完全一致。`__$main__^main` 中参考提前生成 `t13600 = t10300 + 4`、`t13700 = t10300 + 8`、`t13800 = t10600 + 8` 等地址 temp，再通过这些 temp 做 store/load；当前实现直接生成 `str t10500, [t10300, #4]`、`str t162, [t10300, #8]` 和 `ldr t11200, [t10600, #8]`。参考使用 `ldr t162, =C^m`，当前实现使用 `adr t162, C^m`。后续循环部分仍是常量 temp 编号不同。

- `optloopivtest2.s`：只有回边更新语句位置不同。参考在输出 derived IV 结果前计算 `sub t10002, t10001, #2`，当前实现把这条语句放在 `putint` 和 `putch` 之后、回边 PHI 拷贝之前。`t10002` 只在回边更新 `t10001` 时使用，因此语义一致。

- `optloopivtest5.s`：和 `optloopivtest2.s` 类似。参考先生成 `sub t10202, t10201, #1`，再计算并输出 `3 * t10201 + 2`；当前实现先计算输出值并调用 `putint`、`putch`，再生成 `sub t10202, t10201, #1` 并进行回边 PHI 拷贝。该 temp 只用于回边更新，因此语义一致。

额外测试也通过：

```text
Running extra_phi_true_edge
Running extra_relops
All extra HW11 tests passed.
```
