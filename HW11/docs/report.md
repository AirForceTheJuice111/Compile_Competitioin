---
title: "HW11 实验报告"
author: "王思宇"
date: \today
using_title: true
using_table_of_content: true
---

# HW11 实验报告

本次作业实现从 SSA Quad 到 ARMv7-A 汇编的后端指令选择和线性化。输入是 `.4-ssa-withflow-xml.quad`，输出是使用无限量 temp 的 `.s` 文件。

## 参考资料

1. README。HW11 的 README 明确了三个阶段：构造 advDFG、在 advDFG 上做 ARM 指令选择、在线性化阶段处理 Jump/CJump、Phi、函数入口、函数调用和 return。
2. 虎书第 9、10 章。主要参考了数据依赖图、指令选择和过程调用约定的基本思想。
3. ARM Architecture Reference Manual 中 ARMv7-A 常用指令格式说明。实现中使用了 `movw`、`movt`、`add`、`sub`、`mul`、`ldr`、`str`、`cmp`、条件跳转、`bl` 和 `blx`。
4. 课程仓库中 HW11 的头文件和已有框架，包括 `advDFG.hh`、`assem.hh`、`preSchedule.hh`、`schedule.hh` 和 `quad.hh`。这些文件定义了本次实现必须填充的数据结构边界。
5. 前几次作业的报告和实现风格，主要参考 `HW10/docs/report.md` 的报告组织方式和测试结果记录方式。

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

## 遇到的问题

第一个问题是 CMake 大小写。仓库中 `HW11/lib/quadflow` 原本只有 `CmakeLists.txt`，但父级 `CMakeLists.txt` 使用 `add_subdirectory(quadflow)` 时在 Linux 干净构建环境需要 `CMakeLists.txt`。因此我补充了同内容的 `HW11/lib/quadflow/CMakeLists.txt`，不改变原文件。

第二个问题是测试会覆盖仓库中已有的参考 `.s`。为避免把生成文件混入提交，我后续使用 `/tmp/hw11build` 作为独立构建目录，并把 `HW11/test` 复制到 `/tmp/hw11test` 中运行测试。

第三个问题是 PHI 的边拷贝不能只在普通 `JUMP` 上处理。条件跳转的 true 分支如果直接跳到含 PHI 的目标块，就会绕过拷贝。因此 schedule 阶段为这种边插入临时标签，把边拷贝显式放在线性化路径上。

## 额外测试

README 要求不要改变除指定代码外的文件，因此我没有向 `HW11/test` 中新增长期保留的测试文件。额外验证采用隔离测试方式：

- 使用 `/tmp/hw11build` 做干净构建，避免复用仓库中已有 `build/` 缓存。
- 将官方 11 个输入复制到 `/tmp/hw11test`。
- 对每个 `.4-ssa-withflow-xml.quad` 运行新生成的 `main`。
- 检查 11 个 `.s` 都能成功生成。

这组测试覆盖了对象方法调用、外部函数调用、数组读写、常量和负数物化、条件跳转、循环、PHI 回边拷贝、函数参数搬运和 return。

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

隔离运行 11 个官方输入均通过：

```text
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
11
```
