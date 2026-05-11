---
title: "HW9 实验报告"
author: "王思宇"
date: \today
using_title: true
using_table_of_content: true
---

# HW9 实验报告

本次作业在 SSA Quad 和已有控制流信息的基础上实现简单循环优化，包括循环头发现和循环不变量外提。输入为 `.4-ssa-withflow-xml.quad` 文件，输出 loop header 信息和优化后的 `.4-ssa-loopopt.quad` 文件。

## 参考资料

虎书第 18 章 Optimizing for Memory Hierarchies / Loop Optimizations 相关内容。

## 循环头发现 (`findloopheader.cc`)

循环头通过 CFG 中的 back edge 识别。对每条 CFG 边 `tail -> header`，如果 `header` 支配 `tail`，说明控制流从循环体尾部回到了已经支配它的块，因此 `header` 是循环头。

识别到 back edge 后，使用自然循环的反向收集算法得到循环体：

1. 初始时将 `header` 和 `tail` 加入循环体。
2. 从 `tail` 开始沿 predecessor 反向遍历。
3. 新访问到的前驱块加入循环体，并继续向前扩展。
4. 遇到已经加入的块则跳过，直到工作栈为空。

同一个 header 可能有多条 back edge，因此实现中用 `map<int, set<int>>` 按 header 合并所有自然循环体，最后写入 `LoopHeaderMap`。

## 循环不变量判断 (`loophoistfunc.cc`)

本次优化采用保守策略，只考虑无副作用、不会读取内存的语句：

- `MOVE`
- `MOVE_BINOP`
- `PTR_CALC`

以下语句不会被外提：

- `LOAD`：假设每次内存读取都可能得到不同值。
- `STORE`：会修改内存。
- `CALL`、`MOVE_CALL`、`EXTCALL`、`MOVE_EXTCALL`：假设所有调用都可能有副作用。
- `PHI`、`CJUMP`、`JUMP`、`RETURN`：控制流或 SSA 结构语句不能直接移动。

对候选语句，判断它所有 use 的变量是否稳定：

- 如果 use 不在当前循环内定义，则它来自循环外，是循环不变量。
- 如果 use 在当前循环内定义，但该定义语句已经被识别为循环不变量，则它也是稳定的。
- 否则不能外提。

为了处理链式依赖，实现中在单个循环内做固定点迭代。例如 `t2 <- t1 + 1` 依赖 `t1`，只有当 `t1` 的定义也被确认可外提后，`t2` 才能继续被外提。

### Preheader 查找与语句移动

README 假设每个 loop header 已经有 preheader。实现中按以下规则查找 preheader：

1. preheader 不属于循环体。
2. preheader 的 `exit_labels` 中包含 loop header。

找到 preheader 后，将可外提语句插入到 preheader 的终结语句之前。终结语句通常是 `JUMP`、`CJUMP` 或 `RETURN`，如果插到终结语句之后，移动出的代码会不可达。

### 嵌套循环处理

循环优化先按循环体大小排序，优先处理较小的循环，通常对应内层循环。整体再做一层固定点迭代：

1. 先把内层循环的不变量移动到内层 preheader。
2. 如果该 preheader 仍在外层循环中，下一轮外层循环可能继续发现这些语句对外层也是不变量。
3. 重复直到没有新的语句被移动。

这样可以处理嵌套循环中不变量逐层向外提升的情况。

## 额外测试用例

除已有 10 个 opttest 外，额外编写了 3 个测试用例：

| 测试文件 | 覆盖点 | 说明 |
| --- | --- | --- |
| `extra_no_loop` | 无 back edge | CFG 中没有循环，测试不会误判 loop header，优化输出保持不变。 |
| `extra_ptrcalc_memory` | `PTR_CALC`、`LOAD`、`STORE` | `PTR_CALC` 可外提；`LOAD`、`STORE` 和依赖 load 的计算不会被外提。 |
| `extra_multi_backedge` | 多条 back edge 指向同一 header | 测试同一 loop header 的多个自然循环体合并，以及跨 block 的链式不变量外提。 |

## 遇到的问题

`xml2flow` 返回的是 `set<FuncFlowInfo*>`，按指针值排序，因此多函数测试中函数输出顺序可能受内存分配影响。`opttest6` 的优化结果中各函数内容正确，但与原参考文件相比可能出现函数先后顺序不同的问题。

## Git 提交记录

```
9e63a94 Add HW9 loop optimization edge tests
88e8766 HW9 done
162a689 Complete HW9 loop optimization
c0a11e3 Merge branch 'master' of gitee.com:fudanCompiler/fducompilerh2026
84074cb HW9 added
b78281d HW9 added
d6a036c hw 8complete.
da3310a complete? waiting for wxy's response on dead code's warning
4334186 fix: correct evalPhi edge executability and liveness-aware warning
db60dd4 half-complete: my revised ver
b4eceed fix: evalPhi taint approach - propagate NO_VALUE to fire warning at use site
6328394 refactor: simplify evalPhi NO_VALUE handling, remove deferred warning logic
```

## 测试结果

测试输出摘要如下：

```
Reading extra_multi_backedge.4-ssa-withflow-xml.quad
Reading Quad (SSA) with flow info from xml: extra_multi_backedge.4-ssa-withflow-xml.quad
Loop headers for function __$main__^extra_multi_backedge: Header Label: 601, Body Blocks: {601 602 603 } 
Optimized function __$main__^extra_multi_backedge
Writing optimized Quad to file: extra_multi_backedge.4-ssa-loopopt.quad
-----Done---
Reading extra_no_loop.4-ssa-withflow-xml.quad
Reading Quad (SSA) with flow info from xml: extra_no_loop.4-ssa-withflow-xml.quad
Loop headers for function __$main__^extra_no_loop: 
Optimized function __$main__^extra_no_loop
Writing optimized Quad to file: extra_no_loop.4-ssa-loopopt.quad
-----Done---
Reading extra_ptrcalc_memory.4-ssa-withflow-xml.quad
Reading Quad (SSA) with flow info from xml: extra_ptrcalc_memory.4-ssa-withflow-xml.quad
Loop headers for function __$main__^extra_ptrcalc_memory: Header Label: 402, Body Blocks: {402 403 } 
Optimized function __$main__^extra_ptrcalc_memory
Writing optimized Quad to file: extra_ptrcalc_memory.4-ssa-loopopt.quad
-----Done---
Reading opttest1.4-ssa-withflow-xml.quad
Reading Quad (SSA) with flow info from xml: opttest1.4-ssa-withflow-xml.quad
Loop headers for function __$main__^main: Header Label: 102, Body Blocks: {102 103 } 
Optimized function __$main__^main
Writing optimized Quad to file: opttest1.4-ssa-loopopt.quad
-----Done---
Reading opttest10.4-ssa-withflow-xml.quad
Reading Quad (SSA) with flow info from xml: opttest10.4-ssa-withflow-xml.quad
Loop headers for function __$main__^main: Header Label: 102, Body Blocks: {102 103 107 108 109 } Header Label: 107, Body Blocks: {107 108 } 
Optimized function __$main__^main
Writing optimized Quad to file: opttest10.4-ssa-loopopt.quad
-----Done---
Reading opttest2.4-ssa-withflow-xml.quad
Reading Quad (SSA) with flow info from xml: opttest2.4-ssa-withflow-xml.quad
Loop headers for function __$main__^main: Header Label: 102, Body Blocks: {102 103 107 108 109 } Header Label: 107, Body Blocks: {107 108 } 
Optimized function __$main__^main
Writing optimized Quad to file: opttest2.4-ssa-loopopt.quad
-----Done---
Reading opttest3.4-ssa-withflow-xml.quad
Reading Quad (SSA) with flow info from xml: opttest3.4-ssa-withflow-xml.quad
Loop headers for function __$main__^main: Header Label: 104, Body Blocks: {104 105 } 
Optimized function __$main__^main
Writing optimized Quad to file: opttest3.4-ssa-loopopt.quad
-----Done---
Reading opttest4.4-ssa-withflow-xml.quad
Reading Quad (SSA) with flow info from xml: opttest4.4-ssa-withflow-xml.quad
Loop headers for function __$main__^main: Header Label: 102, Body Blocks: {102 103 107 108 109 } Header Label: 107, Body Blocks: {107 108 } 
Optimized function __$main__^main
Writing optimized Quad to file: opttest4.4-ssa-loopopt.quad
-----Done---
Reading opttest5.4-ssa-withflow-xml.quad
Reading Quad (SSA) with flow info from xml: opttest5.4-ssa-withflow-xml.quad
Loop headers for function __$main__^main: Header Label: 102, Body Blocks: {102 103 107 108 109 112 113 114 } Header Label: 107, Body Blocks: {107 108 112 113 114 } 
Optimized function __$main__^main
Writing optimized Quad to file: opttest5.4-ssa-loopopt.quad
-----Done---
Reading opttest6.4-ssa-withflow-xml.quad
Reading Quad (SSA) with flow info from xml: opttest6.4-ssa-withflow-xml.quad
Loop headers for function C^m: Header Label: 102, Body Blocks: {102 103 } 
Optimized function C^m
Loop headers for function __$main__^main: Header Label: 102, Body Blocks: {102 103 107 108 109 112 113 114 } Header Label: 107, Body Blocks: {107 108 112 113 114 } 
Optimized function __$main__^main
Writing optimized Quad to file: opttest6.4-ssa-loopopt.quad
-----Done---
Reading opttest7.4-ssa-withflow-xml.quad
Reading Quad (SSA) with flow info from xml: opttest7.4-ssa-withflow-xml.quad
Loop headers for function __$main__^main: Header Label: 102, Body Blocks: {102 103 108 109 } 
Optimized function __$main__^main
Writing optimized Quad to file: opttest7.4-ssa-loopopt.quad
-----Done---
Reading opttest8.4-ssa-withflow-xml.quad
Reading Quad (SSA) with flow info from xml: opttest8.4-ssa-withflow-xml.quad
Loop headers for function __$main__^main: Header Label: 102, Body Blocks: {102 103 107 108 109 } 
Optimized function __$main__^main
Writing optimized Quad to file: opttest8.4-ssa-loopopt.quad
-----Done---
Reading opttest9.4-ssa-withflow-xml.quad
Reading Quad (SSA) with flow info from xml: opttest9.4-ssa-withflow-xml.quad
Loop headers for function __$main__^main: Header Label: 102, Body Blocks: {102 103 107 108 109 } Header Label: 107, Body Blocks: {107 108 } 
Optimized function __$main__^main
Writing optimized Quad to file: opttest9.4-ssa-loopopt.quad
-----Done---
```
