---
title: "HW6 实验报告"
author: "王思宇"
date: \today
using_title: true
using_table_of_content: true
---

# HW6 实验报告

本次作业在 Quad 中间表示上实现了控制流分析和数据流分析，包括基本块的前驱/后继关系、支配树、支配边界以及活跃变量分析。

## 参考资料

1. 虎书第 17-19 章：数据流分析基础，包括活跃变量分析的迭代算法。
2. 课程课件。

## 关键技术实现

### 控制流分析 (`controlflowinfo.cc`)

#### 不可达块检测与消除

从入口块开始做 BFS，沿 `exit_labels` 扩展可达集合，未被访问到的块即为不可达块。消除时直接从 `func->quadblocklist` 中移除，并清空所有已计算的映射以便重新计算。

#### 前驱/后继计算

遍历每个块的 `exit_labels`：

- 后继：块 B 的 `exit_labels` 中的每个标号对应一个后继块。
- 前驱：后继关系的逆，即若 A → B，则 B 的前驱包含 A。

#### 支配关系（迭代算法）

初始化 `dom(entry) = {entry}`，其余块 `dom(n) = allBlocks`。反复迭代：

$$dom(n) = \{n\} \cup \bigcap_{p \in pred(n)} dom(p)$$

直到不动点。

#### 直接支配者

对每个块 b 的严格支配者集合 `dom(b)\{b}`，找到唯一的 d 使得 `dom(b)\{b}` 中的所有其他元素都支配 d。这个 d 就是 b 的直接支配者（idom）。

#### 支配树

从 idom 关系直接构建：每个块 b 成为 `idom(b)` 的子节点。

#### 支配边界

对每条 CFG 边 (a, b)，从 a 沿 idom 链向上走到 `idom(b)` 为止，途径的每个节点 runner 都将 b 加入其 DF 集合：

```
for each CFG edge (a, b):
    runner = a
    while runner != idom(b):
        DF[runner] ∪= {b}
        runner = idom(runner)
```

### 数据流分析 (`dataflowinfo.cc`)

#### 变量收集

遍历所有块的所有语句，收集 `def` 和 `use` 集合中的临时变量编号到 `allVars`。同时构建 `defs` 和 `uses` 映射（变量 → 定义/使用该变量的 (块, 语句) 对集合）。函数参数也被加入 `allVars`。

#### 活跃变量分析（迭代反向数据流）

标准的反向迭代数据流分析：

- 块内：语句按逆序处理，`live_out(s_i) = live_in(s_{i+1})`。
- 块间：最后一条语句的 `live_out` 等于所有后继块首条语句的 `live_in` 的并集。
- 传递函数：$live\_in(s) = use(s) \cup (live\_out(s) - def(s))$

反复迭代直到不动点。块按逆序处理以加速收敛。

### 一些问题

`main.cc` 中使用 `set<DataFlowInfo*>` 和 `set<FuncFlowInfo*>` 存储分析结果，按指针值排序，导致输出的函数顺序依赖于内存分配器的行为，可能与参考输出顺序不同。但各函数的分析结果内容是正确的。

## Git 提交记录

```
886c8e6 HW6: add 3 extra test cases (diamond CFG, self-loop, nested loops)
75b407e HW6: implement control flow and data flow analysis
89992d9 HW6 README minior edit
ee78ffb HW6 updated
81494a4 HW6 updated
f11bdf7 HW6 test files
3eb7fca HW6 remove build folder
9958d73 HW6 added
8742b14 HW6 added
```

## 额外测试用例

编写了 3 个额外测试用例：

- extratest1：钻石形 CFG（if-then-else 两路汇合），测试支配边界在汇合点的正确性。
- extratest2：自循环（while 循环体只有一条语句），测试块自身出现在自己的支配边界中。
- extratest3：嵌套循环，测试内外层循环的支配关系和支配边界传播。

## 测试结果

```
Reading extratest1.4-xml.quad
Reading Quad from xml: extratest1.4-xml.quad
Done control flow information computing
Reading extratest2.4-xml.quad
Reading Quad from xml: extratest2.4-xml.quad
Done control flow information computing
Reading extratest3.4-xml.quad
Reading Quad from xml: extratest3.4-xml.quad
Done control flow information computing
Reading quadtest1.4-xml.quad
Reading Quad from xml: quadtest1.4-xml.quad
Done control flow information computing
Reading quadtest2.4-xml.quad
Reading Quad from xml: quadtest2.4-xml.quad
Done control flow information computing
Reading quadtest3.4-xml.quad
Reading Quad from xml: quadtest3.4-xml.quad
Done control flow information computing
Reading quadtest4.4-xml.quad
Reading Quad from xml: quadtest4.4-xml.quad
Done control flow information computing
Reading quadtest5.4-xml.quad
Reading Quad from xml: quadtest5.4-xml.quad
Done control flow information computing
Reading quadtest6.4-xml.quad
Reading Quad from xml: quadtest6.4-xml.quad
Done control flow information computing
Reading quadtest7.4-xml.quad
Reading Quad from xml: quadtest7.4-xml.quad
Done control flow information computing
Reading quadtest8.4-xml.quad
Reading Quad from xml: quadtest8.4-xml.quad
Done control flow information computing
Reading quadtest9.4-xml.quad
Reading Quad from xml: quadtest9.4-xml.quad
Done control flow information computing
Reading simplecall1.4-xml.quad
Reading Quad from xml: simplecall1.4-xml.quad
Done control flow information computing
Reading simplecall2.4-xml.quad
Reading Quad from xml: simplecall2.4-xml.quad
Done control flow information computing
```

所有 14 个测试用例（9 个提供的 + 2 个 simplecall + 3 个自编额外测试）均运行成功，输出的控制流和数据流分析结果与参考输出语义一致。
