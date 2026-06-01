---
title: "HW12 实验报告"
author: "王思宇"
date: \today
using_title: true
using_table_of_content: true
---

# HW12 实验报告

本次作业实现从 HW11 产生的虚拟寄存器汇编到 ARMv7-A 实寄存器汇编的寄存器分配。输入是 `.5-xml.asm`，输出是 `k2`、`k5`、`k9` 三种可用寄存器数量下的 `.colored.s`。

## 参考资料

1. 虎书第 11 章。主要参考了 liveness analysis、interference graph、simplify/spill/select 图着色寄存器分配，以及 spilled temp 的栈槽改写方法。
2. 课程 PPT 中关于 HW12 register allocation 的说明。主要参考了 `predataflowpass`、`buildIg`、`simcoafrespisel`、`asmprog2colored` 四个阶段的整体接口约定。
3. ARM Procedure Call Standard 的基本调用约定。实现中把 `r0` 到 `r3` 视为调用会破坏的 caller-saved register，并保留 `r9`、`r10` 作为 spill/reload scratch register。

## preDataFlowPass

`predataflowpass.cc` 中实现了 `preDataFlowPass()`。这个 pass 会遍历每个函数中的每条指令，识别 `bl`、`blx`、`I_CALL` 和 `I_EXTCALL`，并把 `r0`、`r1`、`r2`、`r3` 加入该 call 指令的 `dst`。

这样后续 liveness 和 interference graph 构造时，所有跨 call 仍然 live 的虚拟 temp 都会和 `r0` 到 `r3` 干涉，避免把跨调用活跃值错误分配到 caller-saved register。

## 干涉图构造

`buildIg.cc` 中实现了 `buildIg()` 和 `buildIgProg()`。`buildIgProg()` 为每个函数运行 `AsmDataFlowInfo::computeLiveness()`，再用数据流结果构造对应的 `InterferenceGraph`。

对每条指令：

- 把 `use`、`def`、`liveout` 中出现的 temp 都加入图节点。
- 对每个 `def` 和每个 `liveout` temp 加无向干涉边。
- 如果当前指令是单源单目标 `I_MOVE`，则记录 move pair，并在建边时跳过 move 的源和目标之间的直接干涉边，以保留后续 coalesce 的可能性。

## 图着色

`simcoafrespisel.cc` 中实现了 simplify、coalesce、freeze、spill 和 select。

- `simplify()` 删除度数小于 `k` 的非机器寄存器节点，并压入 `simplifiedNodes` 栈。
- `coalesce()` 采用课件中的 George 安全策略：如果要把 `removed` 合并到 `kept`，则 `removed` 的每个邻居要么已经和 `kept` 干涉，要么当前度数小于 `k`。满足条件时才合并节点、合并边，并把相关 move pair 重写到代表节点上。若 move pair 涉及机器寄存器，则保留机器寄存器作为代表；两个机器寄存器之间不会 coalesce，因为 precolored node 的颜色固定。
- `freeze()` 删除低度 move 节点相关的 move pair，使 simplify 能继续推进。
- `spill()` 在没有低度节点可删时选择当前度数最高的非机器寄存器节点作为 potential spill，压入栈中。
- `select()` 从栈中反向弹出节点，选择一个没有被已着色邻居使用的颜色。如果没有可用颜色，则把该 temp 放入 `spilled`。

机器寄存器编号小于 `100`，其颜色固定为自身编号。虚拟 temp 只使用 `0` 到 `k - 1` 这些颜色，因此在 `k = 2`、`5`、`9` 时分别只使用 `r0` 到 `r1`、`r0` 到 `r4`、`r0` 到 `r8` 作为可分配寄存器。

## 汇编改写

`asmprog2colored.cc` 中实现了 `asmprog2colored()`。该 pass 根据每个函数的 `Coloring` 结果把虚拟 temp 替换为实寄存器。

未 spill 的 temp 直接替换成对应颜色的 ARM register。机器寄存器 temp 固定替换为自身，例如 `t0` 替换为 `r0`，`t13` 替换为 `sp`，`t14` 替换为 `lr`。

对于 spilled temp，每个函数分配独立栈槽，从 `[fp, #-40]` 开始向下增长。使用 spilled temp 前插入：

```text
ldr r9/r10, [fp, #offset]
```

定义 spilled temp 后插入：

```text
str r10, [fp, #offset]
```

目的操作数统一使用 `r10`，这与课件中 spill destination 的处理方式一致。

同时根据 spill 栈槽数量修改函数栈帧。原始 HW11 prologue 中的：

```text
sub sp, sp, #4
add fp, sp, #36
```

会被改成：

```text
sub sp, sp, #(4 + 4 * spill_count)
add fp, sp, #(32 + 4 + 4 * spill_count)
```

epilogue 中对应的 `sub sp, fp, #36` 和 `add sp, sp, #4` 也同步调整。这样 spilled temp 的栈槽不会覆盖保存的 callee-saved register。

## Git 提交记录

```text
cbf6f32 Document HW12 George coalescing
ec51b06 Implement George coalescing for HW12
62b31f8 Add HW12 getint fuzz results
cbf6b97 Update HW12 run-assem results
63bd4af Add HW12 report
b229e2d Implement HW12 register allocation
e99f08e revised HW11 makefile
c48677d revised HW11 report
bcc2e71 HW11 fin
32f4213 Ignore HW11 build directory
a54c0ed Document advDFG instruction selection rewrite
e9a259a Use advDFG traversal for HW11 instruction selection
6824695 Refresh HW11 test result report
d3494d9 Merge branch 'master' of gitee.com:fudanCompiler/fducompilerh2026
4182231 Detail HW11 assembly diffs by test
0514985 HW12 added
980620d Document HW11 assembly output differences
```

## 测试结果

构建通过：

```text
make -C HW12 build
```

重新生成官方输入的 `k2`、`k5`、`k9` 三组 `.colored.s`：

```text
make -C HW12 run
```

对所有生成的 colored assembly 做占位符检查，没有残留虚拟 temp、占位符或非法寄存器名：

```text
rg -n '`[dsj][0-9]|r10000|t[0-9]+' HW12/test/k2 HW12/test/k5 HW12/test/k9 -S
```

没有输出。

配置系统版交叉编译与运行环境后，使用 Makefile 已实现的 `run-assem` 实际链接并运行所有官方用例。由于部分测试程序调用 `getint()`，直接无输入运行时参考程序也会读到未定义值并长时间输出，因此实际验证时通过管道为所有 `getint()` 提供输入 `4`：

```text
yes 4 | timeout 120 make -C HW12 run-assem
```

运行结果如下，`k2`、`k5`、`k9` 三种寄存器数量下输出一致：

```text
Running bubblesort.colored.s .........
Running the final assembly program with k=2.........
0 1 2 3 5 6 9 
Running the final assembly program with k=5.........
0 1 2 3 5 6 9 
Running the final assembly program with k=9.........
0 1 2 3 5 6 9 
Running fibonacci.colored.s .........
Running the final assembly program with k=2.........
Enter the number of term:0 1 1 2 
Running the final assembly program with k=5.........
Enter the number of term:0 1 1 2 
Running the final assembly program with k=9.........
Enter the number of term:0 1 1 2 
Running insttest0.colored.s .........
Running the final assembly program with k=2.........
200Running the final assembly program with k=5.........
200Running the final assembly program with k=9.........
200Running insttest1.colored.s .........
Running the final assembly program with k=2.........
4 3 2 1 
Running the final assembly program with k=5.........
4 3 2 1 
Running the final assembly program with k=9.........
4 3 2 1 
Running insttest2.colored.s .........
Running the final assembly program with k=2.........
3 2 1 Running the final assembly program with k=5.........
3 2 1 Running the final assembly program with k=9.........
3 2 1 Running insttest3.colored.s .........
Running the final assembly program with k=2.........
3 2 1 Running the final assembly program with k=5.........
3 2 1 Running the final assembly program with k=9.........
3 2 1 Running insttest4.colored.s .........
Running the final assembly program with k=2.........
4 3 2 1 Running the final assembly program with k=5.........
4 3 2 1 Running the final assembly program with k=9.........
4 3 2 1 Running optloopivtest1.colored.s .........
Running the final assembly program with k=2.........
14 10 6 2 
Running the final assembly program with k=5.........
14 10 6 2 
Running the final assembly program with k=9.........
14 10 6 2 
Running optloopivtest2.colored.s .........
Running the final assembly program with k=2.........
18 10 0
Running the final assembly program with k=5.........
18 10 0
Running the final assembly program with k=9.........
18 10 0
Running optloopivtest3.colored.s .........
Running the final assembly program with k=2.........
7 
Running the final assembly program with k=5.........
7 
Running the final assembly program with k=9.........
7 
Running optloopivtest4.colored.s .........
Running the final assembly program with k=2.........
34 30 26 22 18 14 10 6 2 -2 0
Running the final assembly program with k=5.........
34 30 26 22 18 14 10 6 2 -2 0
Running the final assembly program with k=9.........
34 30 26 22 18 14 10 6 2 -2 0
Running optloopivtest5.colored.s .........
Running the final assembly program with k=2.........
32 29 26 23 20 17 14 11 8 5 5
Running the final assembly program with k=5.........
32 29 26 23 20 17 14 11 8 5 5
Running the final assembly program with k=9.........
32 29 26 23 20 17 14 11 8 5 5
Running optloopivtest6.colored.s .........
Running the final assembly program with k=2.........
32 29 26 23 20 17 14 11 8 5 5
Running the final assembly program with k=5.........
32 29 26 23 20 17 14 11 8 5 5
Running the final assembly program with k=9.........
32 29 26 23 20 17 14 11 8 5 5
```

另外，我将仓库中参考 `.colored.s` 导出到临时目录，分别编译参考版本和当前生成版本，并对每个二进制单独提供相同输入 `4 4 4 4`。全部 39 组程序的 stdout 完全一致，没有发现语义差异。

针对所有包含 `getint()` 调用的官方程序，我又做了随机输入差分 fuzz。测试方式是把参考 `.colored.s` 和当前生成 `.colored.s` 的 `main` 临时改名为 `test_main`，分别链接同一个 ARM harness。harness 在同一个 qemu 进程内循环调用 `test_main()` 一百万次，随机生成输入，并对 `putint`、`putch` 输出流和返回值计算 hash。这样避免了每组输入都重新启动 qemu 的开销。

输入范围如下：

- `fibonacci`：随机生成 `[-5, 12]`，覆盖负数、零和正常递归输入，同时避免过大的递归运行时间。
- `insttest4`：随机生成 `[-32, 32]`，覆盖选择两个不同数组分支的情况。该用例源码最后会触发 `exit(-1)`，harness 将 exit code 纳入 hash 后继续下一组。
- `optloopivtest1` 和 `optloopivtest2`：随机生成 `[-20, 80]`，覆盖不进循环、小循环和较长循环。
- `optloopivtest3`：第一项随机生成 `[-20, 80]`，第二项随机生成 `[1, 20]`，覆盖不进循环和不同步长，同时保证循环终止。
- `optloopivtest5` 和 `optloopivtest6`：随机生成 `[-32, 32]`，覆盖 `i > 0` 和 `i <= 0` 两条路径。

fuzz 结果如下：

```text
OK k2/fibonacci 55df99f7044bd2a5 1000000
OK k2/insttest4 daf53c3291bcbf2b 1000000
OK k2/optloopivtest1 aaa3658d7a8e2c5f 1000000
OK k2/optloopivtest2 d2aaa2f8ae3f0c91 1000000
OK k2/optloopivtest3 ed40332c76b45d99 2000000
OK k2/optloopivtest5 8b80070115de348b 1000000
OK k2/optloopivtest6 8b80070115de348b 1000000
OK k5/fibonacci 55df99f7044bd2a5 1000000
OK k5/insttest4 daf53c3291bcbf2b 1000000
OK k5/optloopivtest1 aaa3658d7a8e2c5f 1000000
OK k5/optloopivtest2 d2aaa2f8ae3f0c91 1000000
OK k5/optloopivtest3 ed40332c76b45d99 2000000
OK k5/optloopivtest5 8b80070115de348b 1000000
OK k5/optloopivtest6 8b80070115de348b 1000000
OK k9/fibonacci 55df99f7044bd2a5 1000000
OK k9/insttest4 daf53c3291bcbf2b 1000000
OK k9/optloopivtest1 aaa3658d7a8e2c5f 1000000
OK k9/optloopivtest2 d2aaa2f8ae3f0c91 1000000
OK k9/optloopivtest3 ed40332c76b45d99 2000000
OK k9/optloopivtest5 8b80070115de348b 1000000
OK k9/optloopivtest6 8b80070115de348b 1000000
ALL_FUZZ_OK
```

其中最后一列是实际调用 `getint()` 的次数。`optloopivtest3` 每组输入调用两次 `getint()`，所以一百万组随机数据对应两百万次读取。在加入 George coalesce 并调整 spill destination 使用 `r10` 后，重新执行了上述 fuzz。所有 fuzz hash 仍与参考版本一致，没有发现随机输入下的输出或返回值差异。

生成输出与仓库参考 `.colored.s` 不完全一致。逐测试用例差异如下：

1. `bubblesort`：`k2`、`k5`、`k9` 均存在寄存器颜色选择、spill/reload 栈槽和少量冗余 move 差异；`k2`、`k5` 还因为 spill 数量不同导致栈帧大小不同。运行输出一致。
2. `fibonacci`：三种 `k` 下均有寄存器选择和 spill/reload 差异；`k5` 中部分比较和跳转附近的实寄存器不同，但控制流结构不变。运行输出一致。
3. `insttest0`：主要是全局地址加载附近使用的实寄存器不同；`k2` 还多出少量 spill/reload 和栈帧调整。运行输出一致。
4. `insttest1`：三种 `k` 下均有寄存器选择、spill/reload 和 move 消除差异；`k2`、`k5` 的栈帧大小与参考不同。运行输出一致。
5. `insttest2`：三种 `k` 下主要是寄存器选择、spill/reload 插入位置和 move 消除差异；`k5`、`k9` 中比较指令使用的实寄存器不同。运行输出一致。
6. `insttest3`：三种 `k` 下均有寄存器选择与 spill/reload 差异；函数调用相关的 `blx` 位置保持语义顺序，差异只来自参数和临时值所在寄存器。运行输出一致。
7. `insttest4`：三种 `k` 下均有寄存器选择、spill/reload 和栈槽差异；`k9` 比参考少若干可被消除的 move。运行输出一致。
8. `optloopivtest1`：三种 `k` 下差异集中在循环体内归纳变量的 reload、store 和 scratch register 选择，例如参考用 `r10` 保存中间结果，当前版本可复用 `r9`。运行输出一致。
9. `optloopivtest2`：三种 `k` 下差异集中在循环体内 `mul` 前后的 reload/store 和 move 消除；语义等价。运行输出一致。
10. `optloopivtest3`：三种 `k` 下均有两个输入值和循环变量的栈槽、reload/store 差异；`k2` 因 spill 更多，文本差异最大。运行输出一致。
11. `optloopivtest4`：三种 `k` 下差异集中在循环变量更新时的 scratch register 选择和 spill slot 访问。运行输出一致。
12. `optloopivtest5`：三种 `k` 下主要是循环变量、边界值和中间乘法结果的寄存器选择差异；`k9` spill 更少，因此差异主要表现为 move 和实寄存器替换。运行输出一致。
13. `optloopivtest6`：三种 `k` 下主要是循环变量和中间值的 spill/reload 位置、栈槽和 move 消除差异。运行输出一致。

这些差异没有暴露出正确性问题。它们来自图着色中 simplify/spill 顺序、George coalesce 合并选择、spill 栈槽编号和可用颜色选择策略不同。所有差异都已通过实际 ARM 链接运行、与参考版本同输入 stdout 对比，以及一百万组随机输入 fuzz 确认。
