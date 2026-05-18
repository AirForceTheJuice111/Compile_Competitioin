---
title: "HW10 实验报告"
author: "王思宇"
date: \today
using_title: true
using_table_of_content: true
---

# HW10 实验报告

本次作业在 SSA Quad 和已有控制流信息的基础上实现 induction variable 相关循环优化。

## 参考资料

1. 虎书第 18 章中关于循环优化、归纳变量和 strength reduction 的内容。
2. README。README 明确了本阶段只处理一阶 derived induction variable，形如 `j = a * i + b`，并说明了 basic induction variable 的更新形态可以是常量步长或 loop-invariant temp 步长。

## 循环识别

`loopheaderwithflow.cc` 中实现了基于 CFG 的自然循环识别：

1. 遍历每条 CFG 边 `tail -> header`。
2. 如果 `header` 支配 `tail`，则这条边是 back edge。
3. 从 `tail` 开始沿 predecessor 反向遍历，直到回到 `header`，收集自然循环体。
4. 同一个 header 可能有多条 back edge，因此用 `map<int, set<int>>` 合并循环体。

这样得到的 `LoopHeaderMap` 后续被 basic IV、derived IV 和 strength reduction 共同使用。

## Basic Induction Variable

`loopinductionbasic.cc` 识别 header block 中的 `PHI`：

- PHI 的一个输入来自循环外 preheader，作为初始值。
- 另一个输入来自循环体 backedge block，作为回边更新值。
- 回边更新语句必须形如 `i = i + c`、`i = i - c`、`i = i + t` 或 `i = i - t`。
- 如果步长是 temp，则要求该 temp 不在当前循环内定义，因此是 loop invariant。

实现中记录了 `phiTempNum`、`initTempNum`、`backedgeTempNum`、常量步长或 temp 步长、PHI 语句、update 语句以及 update 在函数内的线性顺序。这个顺序用于判断 derived IV 是在 basic IV 更新前还是更新后计算。

`classifyRelatedTemps()` 用 def-use 链判断相关 temp 是否只参与归纳变量自循环。如果一个 temp 只被 PHI/update 家族内部使用，就标记为 useless；如果被真实计算、条件或输出使用，就标记为 useful。

## Derived Induction Variable

`loopinductionderived.cc` 使用一个小的 affine 表达式传播器识别 `a * i + b`：

- basic IV 的 PHI temp 初始化为 `1 * i + 0`。
- backedge temp 也按同一个 basic IV 建模，但额外保留其 source temp，用来判断是否发生在 update 之后。
- 对 `+`、`-`、`*` 做保守传播：只接受常量与 affine 表达式相乘，或同一 basic IV 的 affine 表达式相加减。
- 中间乘法 temp 不直接作为 derived IV 输出，除非它被非 affine 链的语义语句使用。

例如 `t1 = 4 * i; t2 = t1 + 2; putint(t2)` 中，`t1` 只是中间量，真正的 derived IV 是 `t2 = 4 * i + 2`。

## Strength Reduction

`loopstrengthreduction.cc` 对每个 discovered derived IV 生成替换计划：

- 在 preheader 中插入新 IV 的初始值计算。
- 在 loop header 中插入新的 PHI。
- 在 backedge block 的跳转前插入递推更新。
- 将原 derived IV temp 的使用替换为新 PHI temp。
- 删除原 derived IV 定义语句，后续 cleanup 会继续删除无用的中间乘法链。

如果 derived IV 在 basic IV update 之后计算，则初始值需要先用 basic IV 的初始值模拟一次 update。例如 `k = k - 1; j = 4 * k + 2` 会先计算 `k0 - 1`，再计算 `4 * (k0 - 1) + 2`。

对于 loop-invariant temp 步长，例如 `i = i - step`、`j = 8 * i + 7`，实现会在 preheader 中提前准备 `8 * step`，回边更新时用 `j = j - 8 * step`，避免在循环体内反复乘法。

## 无用归纳变量清理

`loopinductionelimination.cc` 采用反向可达标记：

1. 先把有副作用或控制语义的语句标记为 useful，例如 `STORE`、`CALL`、`EXTCALL`、`CJUMP`、`RETURN`。
2. 从这些语句使用的 temp 反查定义语句，把定义语句也标记为 useful。
3. 重复直到没有新的 useful 定义。
4. 删除没有被标记 useful 的纯定义语句，包括 `MOVE`、`MOVE_BINOP`、`PTR_CALC` 和 `PHI`。

这样可以删除只在 PHI/update 自循环中存在、但不再影响条件、输出或返回值的旧 basic IV。

## 额外测试用例

除了课程提供的 `optloopivtest1` 到 `optloopivtest6`，我额外添加了 3 个 `.fmj` 测试：

| 测试文件 | 覆盖点 | 说明 |
| --- | --- | --- |
| `optloopivextra1.fmj` | 正步长 basic IV | `i = i + 1`，derived IV 为 `2 * i + 3`，验证正向递推更新。 |
| `optloopivextra2.fmj` | 负常量偏移 | `j = 5 * k - 7`，`k = k - 3`，验证负 offset 和 `-15` 更新步长。 |
| `optloopivextra3.fmj` | loop-invariant temp 步长 | `i = i - step`，`j = 6 * i + 1`，验证 temp 步长的缩放和 preheader 计算。 |

新增测试的 `.4-ssa-withflow-xml.quad` 和 `.4-ssa.quad` 使用作业提供的 `vendor/genAndrunQuad/linux-ubuntu-amd64/genQuad` 生成。生成文件被仓库 ignore，因此报告中说明测试源文件和运行方式。

## 遇到的问题

derived IV 识别一开始会把中间乘法 temp 也当成 derived IV，导致 strength reduction plan 生成多余替换。后来改为只把被非 affine 链语句消费的 affine temp 作为真正 derived IV。

## Git 提交记录

```text
79099be Add HW10 induction optimization tests
5816844 Implement HW10 induction variable optimization
3602dda Merge branch 'master' of gitee.com:fudanCompiler/fducompilerh2026
efb30d1 HW10/vendor/genAndrunQuad/README: typo correction
f0a162c HW10: simplified. README changed
7caa712 HW10: simplified
e76c672 HW10: simpler test caseswith reports added into fmj files
d85741a HW10: simplified
```

## 测试结果

`make build` 通过：

```text
-- Configuring done (0.0s)
-- Generating done (0.0s)
-- Build files have been written to: /home/wsy/fducompilerh2026/HW10/build
[1/2] Building CXX object lib/opt/CMakeFiles/opt.dir/loopstrengthreduction.cc.o
[2/2] Linking CXX executable tools/main/main
```

`make run` 通过，覆盖 3 个新增测试和 6 个官方测试：

```text
Reading optloopivextra1.4-ssa-withflow-xml.quad
Reading Quad (SSA) from xml: optloopivextra1.4-ssa-withflow-xml.quad
Successfully read SSA with flow information

=== Processing function: __$main__^main ===
=== BASIC INDUCTION VARIABLES ===
Loop at header L102:
  t10001 (PHI at L102)
    Initial value: t10000
    Backedge: t10002
    Update order: 11
    Step: 1 (constant)
    Related temps: t10000 t10001 t10002 
    Useless (cyclic only): t10000 t10002 

=== DERIVED INDUCTION VARIABLES ===
Loop at header L102:
  t10200 (derived IV)
    Depends on: t10600
    Source order: 8
    Expression: 2*t10001 + 3
    Definition: MOVE_BINOP t10200:int <- (+, t10600:int, Const:3); 


=== STRENGTH REDUCTION PLAN ===
Total replacements: 1
Replacement 1:
  Original IV: t10200
  New PHI temp: t10201
  New backedge temp: t10203
  Loop header: L102
  Source order: 8
  Init adjusted for update order: no
  Init expr: 2*t10001 + 3
  Step expr: 2

Optimized function __$main__^main
Writing optimized Quad to file: optloopivextra1.4-ssa-loopivopt.quad
-----Done---
Reading optloopivextra2.4-ssa-withflow-xml.quad
Reading Quad (SSA) from xml: optloopivextra2.4-ssa-withflow-xml.quad
Successfully read SSA with flow information

=== Processing function: __$main__^main ===
=== BASIC INDUCTION VARIABLES ===
Loop at header L102:
  t10101 (PHI at L102)
    Initial value: t10100
    Backedge: t10102
    Update order: 11
    Step: -3 (constant)
    Related temps: t10100 t10101 t10102 
    Useless (cyclic only): t10100 t10102 

=== DERIVED INDUCTION VARIABLES ===
Loop at header L102:
  t10000 (derived IV)
    Depends on: t10800
    Source order: 8
    Expression: 5*t10101 - 7
    Definition: MOVE_BINOP t10000:int <- (-, t10800:int, Const:7); 


=== STRENGTH REDUCTION PLAN ===
Total replacements: 1
Replacement 1:
  Original IV: t10000
  New PHI temp: t10001
  New backedge temp: t10003
  Loop header: L102
  Source order: 8
  Init adjusted for update order: no
  Init expr: 5*t10101 - 7
  Step expr: -15

Optimized function __$main__^main
Writing optimized Quad to file: optloopivextra2.4-ssa-loopivopt.quad
-----Done---
Reading optloopivextra3.4-ssa-withflow-xml.quad
Reading Quad (SSA) from xml: optloopivextra3.4-ssa-withflow-xml.quad
Successfully read SSA with flow information

=== Processing function: __$main__^main ===
=== BASIC INDUCTION VARIABLES ===
Loop at header L102:
  t10001 (PHI at L102)
    Initial value: t10000
    Backedge: t10002
    Update order: 8
    Step: -t10100 (loop-invariant)
    Related temps: t10000 t10001 t10002 t10100 
    Useless (cyclic only): t10000 

=== DERIVED INDUCTION VARIABLES ===
Loop at header L102:
  t10500 (derived IV)
    Depends on: t11100
    Source order: 10
    Expression: 6*t10001 + 1
    Definition: MOVE_BINOP t10500:int <- (+, t11100:int, Const:1); 


=== STRENGTH REDUCTION PLAN ===
Total replacements: 1
Replacement 1:
  Original IV: t10500
  New PHI temp: t10501
  New backedge temp: t10503
  Loop header: L102
  Source order: 10
  Init adjusted for update order: yes
  Init expr: 6*t10001 + 1
  Step expr: -t10506

Optimized function __$main__^main
Writing optimized Quad to file: optloopivextra3.4-ssa-loopivopt.quad
-----Done---
Reading optloopivextra4.4-ssa-withflow-xml.quad
Reading Quad (SSA) from xml: optloopivextra4.4-ssa-withflow-xml.quad
Successfully read SSA with flow information

=== Processing function: __$main__^main ===
=== BASIC INDUCTION VARIABLES ===
Loop at header L102:
  t10101 (PHI at L102)
    Initial value: t10100
    Backedge: t10102
    Update order: 13
    Step: -t10200 (loop-invariant)
    Related temps: t10100 t10101 t10102 t10200 
    Useless (cyclic only): t10100 t10102 

=== DERIVED INDUCTION VARIABLES ===
Loop at header L102:
  t10600 (derived IV)
    Depends on: t11200
    Source order: 10
    Expression: 3*t10101 + 2
    Definition: MOVE_BINOP t10600:int <- (+, t11200:int, Const:2); 


=== STRENGTH REDUCTION PLAN ===
Total replacements: 1
Replacement 1:
  Original IV: t10600
  New PHI temp: t10601
  New backedge temp: t10603
  Loop header: L102
  Source order: 10
  Init adjusted for update order: no
  Init expr: 3*t10101 + 2
  Step expr: -t10605

Optimized function __$main__^main
Writing optimized Quad to file: optloopivextra4.4-ssa-loopivopt.quad
-----Done---
Reading optloopivextra5.4-ssa-withflow-xml.quad
Reading Quad (SSA) from xml: optloopivextra5.4-ssa-withflow-xml.quad
Successfully read SSA with flow information

=== Processing function: __$main__^main ===
=== BASIC INDUCTION VARIABLES ===
Loop at header L102:
  t10101 (PHI at L102)
    Initial value: t10100
    Backedge: t10102
    Update order: 9
    Step: -t10200 (loop-invariant)
    Related temps: t10100 t10101 t10102 t10200 
    Useless (cyclic only): t10100 

=== DERIVED INDUCTION VARIABLES ===
Loop at header L102:
  t10600 (derived IV)
    Depends on: t11200
    Source order: 11
    Expression: 3*t10101 + 2
    Definition: MOVE_BINOP t10600:int <- (+, t11200:int, Const:2); 


=== STRENGTH REDUCTION PLAN ===
Total replacements: 1
Replacement 1:
  Original IV: t10600
  New PHI temp: t10601
  New backedge temp: t10603
  Loop header: L102
  Source order: 11
  Init adjusted for update order: yes
  Init expr: 3*t10101 + 2
  Step expr: -t10606

Optimized function __$main__^main
Writing optimized Quad to file: optloopivextra5.4-ssa-loopivopt.quad
-----Done---
Reading optloopivtest1.4-ssa-withflow-xml.quad
Reading Quad (SSA) from xml: optloopivtest1.4-ssa-withflow-xml.quad
Successfully read SSA with flow information

=== Processing function: __$main__^main ===
=== BASIC INDUCTION VARIABLES ===
Loop at header L102:
  t10001 (PHI at L102)
    Initial value: t10000
    Backedge: t10002
    Update order: 7
    Step: -1 (constant)
    Related temps: t10000 t10001 t10002 
    Useless (cyclic only): t10000 

=== DERIVED INDUCTION VARIABLES ===
Loop at header L102:
  t10300 (derived IV)
    Depends on: t10800
    Source order: 9
    Expression: 4*t10001 + 2
    Definition: MOVE_BINOP t10300:int <- (+, t10800:int, Const:2); 


=== STRENGTH REDUCTION PLAN ===
Total replacements: 1
Replacement 1:
  Original IV: t10300
  New PHI temp: t10301
  New backedge temp: t10303
  Loop header: L102
  Source order: 9
  Init adjusted for update order: yes
  Init expr: 4*t10001 + 2
  Step expr: -4

Optimized function __$main__^main
Writing optimized Quad to file: optloopivtest1.4-ssa-loopivopt.quad
-----Done---
Reading optloopivtest2.4-ssa-withflow-xml.quad
Reading Quad (SSA) from xml: optloopivtest2.4-ssa-withflow-xml.quad
Successfully read SSA with flow information

=== Processing function: __$main__^main ===
=== BASIC INDUCTION VARIABLES ===
Loop at header L102:
  t10001 (PHI at L102)
    Initial value: t10000
    Backedge: t10002
    Update order: 11
    Step: -2 (constant)
    Related temps: t10000 t10001 t10002 
    Useless (cyclic only): t10000 t10002 

=== DERIVED INDUCTION VARIABLES ===
Loop at header L102:
  t10300 (derived IV)
    Depends on: t10900
    Source order: 8
    Expression: 4*t10001 + 2
    Definition: MOVE_BINOP t10300:int <- (+, t10900:int, Const:2); 


=== STRENGTH REDUCTION PLAN ===
Total replacements: 1
Replacement 1:
  Original IV: t10300
  New PHI temp: t10301
  New backedge temp: t10303
  Loop header: L102
  Source order: 8
  Init adjusted for update order: no
  Init expr: 4*t10001 + 2
  Step expr: -8

Optimized function __$main__^main
Writing optimized Quad to file: optloopivtest2.4-ssa-loopivopt.quad
-----Done---
Reading optloopivtest3.4-ssa-withflow-xml.quad
Reading Quad (SSA) from xml: optloopivtest3.4-ssa-withflow-xml.quad
Successfully read SSA with flow information

=== Processing function: __$main__^main ===
=== BASIC INDUCTION VARIABLES ===
Loop at header L102:
  t10301 (PHI at L102)
    Initial value: t10300
    Backedge: t10302
    Update order: 10
    Step: -t10400 (loop-invariant)
    Related temps: t10300 t10301 t10302 t10400 
    Useless (cyclic only): t10300 

=== DERIVED INDUCTION VARIABLES ===
Loop at header L102:
  t10202 (derived IV)
    Depends on: t11300
    Source order: 12
    Expression: 8*t10301 + 7
    Definition: MOVE_BINOP t10202:int <- (+, t11300:int, Const:7); 


=== STRENGTH REDUCTION PLAN ===
Total replacements: 1
Replacement 1:
  Original IV: t10202
  New PHI temp: t10203
  New backedge temp: t10205
  Loop header: L102
  Source order: 12
  Init adjusted for update order: yes
  Init expr: 8*t10301 + 7
  Step expr: -t10208

Optimized function __$main__^main
Writing optimized Quad to file: optloopivtest3.4-ssa-loopivopt.quad
-----Done---
Reading optloopivtest4.4-ssa-withflow-xml.quad
Reading Quad (SSA) from xml: optloopivtest4.4-ssa-withflow-xml.quad
Successfully read SSA with flow information

=== Processing function: __$main__^main ===
=== BASIC INDUCTION VARIABLES ===
Loop at header L102:
  t10101 (PHI at L102)
    Initial value: t10100
    Backedge: t10102
    Update order: 7
    Step: -1 (constant)
    Related temps: t10100 t10101 t10102 
    Useless (cyclic only): t10100 

=== DERIVED INDUCTION VARIABLES ===
Loop at header L102:
  t10000 (derived IV)
    Depends on: t10700
    Source order: 9
    Expression: 4*t10101 - 2
    Definition: MOVE_BINOP t10000:int <- (-, t10700:int, Const:2); 


=== STRENGTH REDUCTION PLAN ===
Total replacements: 1
Replacement 1:
  Original IV: t10000
  New PHI temp: t10001
  New backedge temp: t10003
  Loop header: L102
  Source order: 9
  Init adjusted for update order: yes
  Init expr: 4*t10101 - 2
  Step expr: -4

Optimized function __$main__^main
Writing optimized Quad to file: optloopivtest4.4-ssa-loopivopt.quad
-----Done---
Reading optloopivtest5.4-ssa-withflow-xml.quad
Reading Quad (SSA) from xml: optloopivtest5.4-ssa-withflow-xml.quad
Successfully read SSA with flow information

=== Processing function: __$main__^main ===
=== BASIC INDUCTION VARIABLES ===
Loop at header L104:
  t10201 (PHI at L104)
    Initial value: t10200
    Backedge: t10202
    Update order: 16
    Step: -1 (constant)
    Related temps: t10200 t10201 t10202 
    Useless (cyclic only): t10200 t10202 

=== DERIVED INDUCTION VARIABLES ===
Loop at header L104:
  t10102 (derived IV)
    Depends on: t11000
    Source order: 13
    Expression: 3*t10201 + 2
    Definition: MOVE_BINOP t10102:int <- (+, t11000:int, Const:2); 


=== STRENGTH REDUCTION PLAN ===
Total replacements: 1
Replacement 1:
  Original IV: t10102
  New PHI temp: t10105
  New backedge temp: t10107
  Loop header: L104
  Source order: 13
  Init adjusted for update order: no
  Init expr: 3*t10201 + 2
  Step expr: -3

Optimized function __$main__^main
Writing optimized Quad to file: optloopivtest5.4-ssa-loopivopt.quad
-----Done---
Reading optloopivtest6.4-ssa-withflow-xml.quad
Reading Quad (SSA) from xml: optloopivtest6.4-ssa-withflow-xml.quad
Successfully read SSA with flow information

=== Processing function: __$main__^main ===
=== BASIC INDUCTION VARIABLES ===
Loop at header L102:
  t10201 (PHI at L102)
    Initial value: t10200
    Backedge: t10202
    Update order: 22
    Step: -1 (constant)
    Related temps: t10200 t10201 t10202 
    Useless (cyclic only): t10200 t10202 

=== DERIVED INDUCTION VARIABLES ===
Loop at header L102:
  t10102 (derived IV)
    Depends on: t11000
    Source order: 13
    Expression: 3*t10201 + 2
    Definition: MOVE_BINOP t10102:int <- (+, t11000:int, Const:2); 


=== STRENGTH REDUCTION PLAN ===
Total replacements: 1
Replacement 1:
  Original IV: t10102
  New PHI temp: t10105
  New backedge temp: t10107
  Loop header: L102
  Source order: 13
  Init adjusted for update order: no
  Init expr: 3*t10201 + 2
  Step expr: -3

Optimized function __$main__^main
Writing optimized Quad to file: optloopivtest6.4-ssa-loopivopt.quad
-----Done---
```

6 个输出文件与参考输出完全一致。
