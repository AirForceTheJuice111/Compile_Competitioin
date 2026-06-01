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

`simcoafrespisel.cc` 中实现了简化版的 simplify、freeze、spill 和 select。

- `simplify()` 删除度数小于 `k` 的非机器寄存器节点，并压入 `simplifiedNodes` 栈。
- `coalesce()` 采用保守策略，当前不主动合并节点，以降低错误 coalesce 破坏干涉关系的风险。
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
str r9/r10, [fp, #offset]
```

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

独立构建通过：

```text
cmake -S HW12 -B /tmp/hw12build -G Ninja
ninja -C /tmp/hw12build
```

隔离运行 13 个官方输入，均成功生成 `k2`、`k5`、`k9` 三组 `.colored.s`：

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
DONE
```

对所有生成的 colored assembly 做占位符检查，没有残留虚拟 temp、占位符或非法寄存器名：

```text
rg -n '`[dsj][0-9]|r100|r10000|t[0-9]' /tmp/hw12-current/k2 /tmp/hw12-current/k5 /tmp/hw12-current/k9 -S
```

没有输出。

使用 `arm-linux-gnueabihf-as` 对全部 39 个 `.colored.s` 做汇编器检查，全部通过：

```text
Assembling only k2/bubblesort
Assembling only k2/fibonacci
Assembling only k2/insttest0
Assembling only k2/insttest1
Assembling only k2/insttest2
Assembling only k2/insttest3
Assembling only k2/insttest4
Assembling only k2/optloopivtest1
Assembling only k2/optloopivtest2
Assembling only k2/optloopivtest3
Assembling only k2/optloopivtest4
Assembling only k2/optloopivtest5
Assembling only k2/optloopivtest6
Assembling only k5/bubblesort
Assembling only k5/fibonacci
Assembling only k5/insttest0
Assembling only k5/insttest1
Assembling only k5/insttest2
Assembling only k5/insttest3
Assembling only k5/insttest4
Assembling only k5/optloopivtest1
Assembling only k5/optloopivtest2
Assembling only k5/optloopivtest3
Assembling only k5/optloopivtest4
Assembling only k5/optloopivtest5
Assembling only k5/optloopivtest6
Assembling only k9/bubblesort
Assembling only k9/fibonacci
Assembling only k9/insttest0
Assembling only k9/insttest1
Assembling only k9/insttest2
Assembling only k9/insttest3
Assembling only k9/insttest4
Assembling only k9/optloopivtest1
Assembling only k9/optloopivtest2
Assembling only k9/optloopivtest3
Assembling only k9/optloopivtest4
Assembling only k9/optloopivtest5
Assembling only k9/optloopivtest6
All generated colored.s accepted by assembler
```

当前环境没有安装 `qemu-arm`：

```text
command -v qemu-arm qemu-arm-static qemu-aarch64
```

没有输出。因此本地无法执行 `make run-assem` 的最终 ARM user-mode 运行验证。尝试使用 `arm-linux-gnueabihf-gcc` 链接时，当前环境的交叉 GCC 还缺少 `cc1`，并且自带 `libsysy32.s` 与该 assembler 的默认 `-march=armv8-a+crc` 选项不兼容。因此本次本地测试以生成成功、占位符清除、interference coloring 无冲突报错、全部 `.colored.s` 被 ARM assembler 接受作为证明。

生成输出与仓库参考 `.colored.s` 不完全一致，原因是图着色中 simplify/spill 的选择顺序、是否 coalesce、spill 栈槽分配顺序和可用颜色选择策略都允许不同。寄存器分配不要求文本完全一致，只要最终汇编满足干涉约束、调用约定和 spill/reload 语义即可。
