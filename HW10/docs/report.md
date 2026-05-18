---
title: "HW10 实验报告"
author: "王思宇"
date: \today
using_title: true
using_table_of_content: true
---

# HW10 实验报告

本次作业在 SSA Quad 和已有控制流信息的基础上实现 induction variable 相关循环优化。输入为 `.4-ssa-withflow-xml.quad` 文件，输出 basic induction variable、derived induction variable、strength reduction plan 的报告，以及优化后的 `.4-ssa-loopivopt.quad` 文件。

## 参考资料

1. 《现代编译原理：C 语言描述》（虎书）第 18 章中关于循环优化、归纳变量和 strength reduction 的内容。该部分用于确认 basic induction variable、derived induction variable 与强度削弱的基本定义。

2. 课程提供的 `HW10/README.md`。README 明确了本阶段只处理一阶 derived induction variable，形如 `j = a * i + b`，并说明了 basic induction variable 的更新形态可以是常量步长或 loop-invariant temp 步长。

3. 课程提供的 `.github/FDMJSLPGrammar.md` 和 `.github/FDMJSLPClassHierarchy.md`。这两个文件用于确认新增 `.fmj` 测试程序的语法范围。

4. HW9 实现中的 loop header 识别思路。HW10 的 `loopheaderwithflow.cc` 复用了 HW9 中“back edge + dominator + 反向 predecessor 收集自然循环”的方法，只是数据来源改为 `ControlFlowInfo`。

5. LLVM Loop Strength Reduction 文档：https://llvm.org/docs/Passes.html#loop-reduce-loop-strength-reduction 。该资料用于辅助理解 strength reduction 的工程化目标，但本次实现严格按 README 中限定的一阶 affine 形式完成。

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

1. `HW10/lib/quadflow` 目录下原文件名是 `CmakeLists.txt`，但上层 CMake 使用 `add_subdirectory` 查找 `CMakeLists.txt`。在 Linux 大小写敏感文件系统上会导致构建失败，因此将文件名修正为 `CMakeLists.txt`。

2. derived IV 识别一开始会把中间乘法 temp 也当成 derived IV，导致 strength reduction plan 生成多余替换。后来改为只把被非 affine 链语句消费的 affine temp 作为真正 derived IV。

3. 清理旧 IV 时，普通“无 use 删除”不能删除 PHI/update 自循环。最终改用从副作用、条件和返回语句反向标记 useful definition 的方式，才能删除不再影响程序结果的自循环。

4. `run_quad_text.sh` 需要本机安装 `llvm-link-14`。当前环境没有该命令，所以无法做优化前后 LLVM 执行对比。本次验证以 `make build`、`make run`、官方 6 个输出文件与参考输出无 diff 为准。

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
Optimized function __$main__^main
Writing optimized Quad to file: optloopivextra1.4-ssa-loopivopt.quad
-----Done---
Reading optloopivextra2.4-ssa-withflow-xml.quad
Reading Quad (SSA) from xml: optloopivextra2.4-ssa-withflow-xml.quad
Successfully read SSA with flow information
Optimized function __$main__^main
Writing optimized Quad to file: optloopivextra2.4-ssa-loopivopt.quad
-----Done---
Reading optloopivextra3.4-ssa-withflow-xml.quad
Reading Quad (SSA) from xml: optloopivextra3.4-ssa-withflow-xml.quad
Successfully read SSA with flow information
Optimized function __$main__^main
Writing optimized Quad to file: optloopivextra3.4-ssa-loopivopt.quad
-----Done---
Reading optloopivtest1.4-ssa-withflow-xml.quad
Reading Quad (SSA) from xml: optloopivtest1.4-ssa-withflow-xml.quad
Successfully read SSA with flow information
Optimized function __$main__^main
Writing optimized Quad to file: optloopivtest1.4-ssa-loopivopt.quad
-----Done---
Reading optloopivtest2.4-ssa-withflow-xml.quad
Reading Quad (SSA) from xml: optloopivtest2.4-ssa-withflow-xml.quad
Successfully read SSA with flow information
Optimized function __$main__^main
Writing optimized Quad to file: optloopivtest2.4-ssa-loopivopt.quad
-----Done---
Reading optloopivtest3.4-ssa-withflow-xml.quad
Reading Quad (SSA) from xml: optloopivtest3.4-ssa-withflow-xml.quad
Successfully read SSA with flow information
Optimized function __$main__^main
Writing optimized Quad to file: optloopivtest3.4-ssa-loopivopt.quad
-----Done---
Reading optloopivtest4.4-ssa-withflow-xml.quad
Reading Quad (SSA) from xml: optloopivtest4.4-ssa-withflow-xml.quad
Successfully read SSA with flow information
Optimized function __$main__^main
Writing optimized Quad to file: optloopivtest4.4-ssa-loopivopt.quad
-----Done---
Reading optloopivtest5.4-ssa-withflow-xml.quad
Reading Quad (SSA) from xml: optloopivtest5.4-ssa-withflow-xml.quad
Successfully read SSA with flow information
Optimized function __$main__^main
Writing optimized Quad to file: optloopivtest5.4-ssa-loopivopt.quad
-----Done---
Reading optloopivtest6.4-ssa-withflow-xml.quad
Reading Quad (SSA) from xml: optloopivtest6.4-ssa-withflow-xml.quad
Successfully read SSA with flow information
Optimized function __$main__^main
Writing optimized Quad to file: optloopivtest6.4-ssa-loopivopt.quad
-----Done---
```

官方 6 个输出文件与仓库参考输出完全一致：

```text
git diff -- HW10/test/optloopivtest1.4-ssa-loopivopt.quad HW10/test/optloopivtest2.4-ssa-loopivopt.quad HW10/test/optloopivtest3.4-ssa-loopivopt.quad HW10/test/optloopivtest4.4-ssa-loopivopt.quad HW10/test/optloopivtest5.4-ssa-loopivopt.quad HW10/test/optloopivtest6.4-ssa-loopivopt.quad

<no output>
```
