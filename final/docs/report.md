---
title: "Final Project 实验报告"
author: "王思宇"
date: \today
using_title: true
using_table_of_content: true
---

# Final Project 实验报告

本次 Final Project 的目标是把 HW2 到 HW12 中逐步实现的各个编译器阶段合并为一个完整的 FMJ 编译器，并在 `final/` 目录下提供统一构建、编译、链接、运行和回归测试入口。最终版本可以从 `.fmj` 源程序出发，经过前端、语义分析、IR 生成、Quad、SSA、优化、指令选择、调度和寄存器分配，生成可由 ARM 交叉工具链链接并在 qemu 下运行的 ARM 汇编。

除编译器本体外，本次还实现了一个解释器 `fmjinterp` 作为 oracle。所有收集到的 FMJ 测试均先由解释器和编译器分别检查；能通过检查的程序再比较解释执行结果和编译运行结果；带外部输入的程序通过随机输入 fuzz 比较一百万组输入下的输出 hash。

## 总体结构

`final/` 保留各次作业中 `include/`、`lib/`、`tools/` 的总体组织方式，把原先分散在 HW2-HW12 中的组件统一放入同一个 CMake 工程：

- `final/tools/fmjcc/main.cc`：完整编译器入口。
- `final/tools/fmjinterp/main.cc`：解释器和语义检查 oracle。
- `final/lib/frontend/` 和 `final/include/frontend/`：由 `PARSING/` 集成进来的 flex/bison parser 源码，用于把 FMJ 源文件转换为 XML AST。
- `final/scripts/collect_tests.sh`：收集全仓库 HW 中的 `.fmj` 文件。
- `final/scripts/compile_regression.sh`：只检查编译器是否接受或拒绝输入，以及是否生成空 label/block。
- `final/scripts/interpreter_regression.sh`：用解释器对照编译运行结果。
- `final/scripts/runtime_regression.sh`：对 HW10/HW12 运行时相关样例做快速链接运行检查。
- `final/scripts/fuzz_regression.sh`：对含输入程序做解释器 oracle fuzz。

构建入口仍然是 Makefile：

```text
make build
make compile-regression
make interpreter-regression
make runtime-regression
make fuzz-regression
```

同时可以直接编译运行指定任意 `.fmj` 路径：

```text
make run-one path/to/file.fmj
```

该目标会复制输入到临时目录，运行 `fmjcc` 生成 `.s`，再使用 HW12 的交叉编译参数链接并用 qemu 运行。

## 编译流水线

`fmjcc` 的默认流水线如下：

1. 调用 HW2 parser，把 `.fmj` 源文件解析为 XML AST。
2. `xml2ast` 读取 XML AST，恢复 C++ AST。
3. HW2 语义分析构建 NameMap 并进行类型检查、继承检查、变量与方法检查。
4. HW3 `ast2tree` 把 AST 翻译为 Tree IR。
5. HW6 规范化 Tree IR，生成 canonical form。
6. HW7 将 Tree IR 转换为 Quad，并做 basic block 划分和控制流分析。
7. HW8 转 SSA，并运行 SCCP。
8. HW9 运行 LICM。
9. HW10 运行 induction variable strength reduction 和无用归纳变量清理。
10. HW11 进行指令选择、advDFG 调度和汇编线性化。
11. HW12 进行寄存器分配和 spill 改写，输出 ARM 汇编。

默认编译启用 HW6-HW10 的优化；调试时可以用 `--no-opt` 跳过优化，便于定位后端错误。寄存器数量通过 `--k` 指定，默认回归测试使用 `k=9`。

## 前端与语法边界

Final 使用 `PARSING/` 中的 flex/bison parser 源码，而不是运行外部 parser binary。CMake 构建时会生成 `lexer.cc` 和 `parser.cc`，并把它们直接链接进 `fmjcc` 和 `fmjinterp`。编译器和解释器都复用同一个 parser，因此语法边界保持一致。

这里有一个需要特别说明的边界情况：`HW2/docs/FDMJ2026Specification.md` 中规定：

```text
VarDecl -> Type id [= ArrayInit] ;
ArrayInit -> { CONST (, CONST)* }
```

也就是说，变量声明只允许没有初始化，或者使用数组字面量初始化；`int i = 0;` 并不是合法 FMJ 语法。编译器和解释器会拒绝之。

## 运行期语义

为了让解释器和编译后的 ARM 程序具有一致的行为，Final 中补齐了若干运行期语义：

- 局部变量和字段默认初始化：`int` 为 `0`，数组和对象引用为空。
- 函数执行到末尾没有显式 `return` 时默认返回 `0`。
- 数组布局使用首 word 记录长度，数组字面量和 `new int[n]` 使用同一布局。
- 对空对象、空数组、数组越界、负长度数组和除零进行运行期检查。
- 运行期错误通过 `exit(-1)` 结束，因此 shell 观察到的退出码为 `255`。
- 对象创建时初始化祖先类字段、本类字段和方法指针。

之前出现的空 block / 空 label 问题来自 IR 翻译和后续线性化阶段对某些控制流边界处理不完整。修复后，回归脚本会扫描生成的 Quad，检查 `Entry Label:` 空值、`LABEL ;` 和空跳转目标，当前结果为 `empty_label_hits=0`。

## HW6-HW10 优化集成

优化集成时比较重要的一点是优化之间共享 label/temp 编号空间。每个 pass 后都需要维护 `last_label_num` 和 `last_temp_num`，否则后端会遇到重复临时变量或跳转目标不一致的问题。

## 解释器 oracle

`fmjinterp` 使用与编译器相同的 parser 和语义分析，保证“是否接受输入”的判断基于同一套语法和类型规则。解释器直接执行 AST，覆盖：

- `int`、数组、对象、继承和方法调用。
- `if`、`while`、`break`、`continue`、`return`。
- `getint`、`getch`、`getarray`、`putint`、`putch`、`putarray`。
- 32 位整数加减乘 wraparound 语义。
- 空引用、越界、负数组长度、除零等运行期错误。

解释器提供两种用途：

1. `--check` 只做 parser 和 semantic check，用于确认 reject 是否合理。
2. 普通执行模式输出 stdout 和退出码，用于和 qemu 运行的编译产物比较。

fuzz 时解释器还提供 hash 模式，和 ARM harness 使用相同的随机输入生成器以及相同的输出 hash 规则。这样一百万组输入不需要保存完整输出流，只比较最终 hash 和输入调用次数。

## 测试收集与 reject 确认

`collect_tests.sh` 会遍历整个仓库，排除 `final/` 自身，收集各 HW 和 `PARSING/test` 中的 `.fmj` 文件到 `final/test/all`。当前一共收集到 280 个测试。

对这 280 个输入，`compile-regression` 的结果是：

```text
total=280 pass=171 reject=109 crash=0 timeout=0 empty_label_hits=0 workdir=/tmp/final_compile_regression
```

这里 `reject=109` 再用解释器的 `--check` 模式确认。`interpreter-regression` 会分别运行解释器检查和编译器检查：

- 两者都拒绝，记为 `reject_match`。
- 只有编译器拒绝，记为 `compile_only_reject`。
- 只有解释器拒绝，记为 `interp_only_reject`。

当前结果如下：

```text
total=280 run_match=167 reject_match=109 timeout_match=4 mismatch=0 compile_only_reject=0 interp_only_reject=0 link_fail=0 run_fail=0 timeout=4 workdir=/tmp/final_interpreter_regression
```

因此 109 个 reject 均由解释器语义/语法检查确认，没有出现编译器单方面拒绝的情况。4 个 timeout 是解释器和编译产物都超时的非终止程序，作为行为一致处理。

## Fuzz 测试

对于包含外部输入的程序，`fuzz_regression.sh` 使用解释器作为 oracle 进行差分 fuzz。测试方式是：

1. 收集含 `getint`、`getch` 或 `getarray` 的 FMJ 程序。
2. 先用解释器和编译器分别检查语法/语义。
3. 对可接受程序，编译成 ARM 汇编，并把 `main` 改名为 `test_main`。
4. ARM 侧链接一个 harness，在同一个 qemu 进程中循环调用 `test_main()`。
5. harness 和解释器使用相同 seed、相同输入生成策略、相同输出 hash。
6. 每个程序执行 `ITERS=1000000` 组随机输入。

最近一次 fuzz 结果：

```text
fuzz_pass=41 fuzz_fail=0 fuzz_skip=3 iterations=1000000 kset="9" workdir=/tmp/final_fuzz_regression
```

其中 `fuzz_skip=3` 是三个 spec-invalid 的 HW2 原始测试：

```text
HW2/test/fibonacci.fmj
HW2/test/test_comprehensive.fmj
HW2/test/test_io.fmj
```

这些文件包含 `int i = 0;` 或类似声明初始化，因此 parser 按 FDMJ2026 specification 拒绝。输入类测试的分类结果为：

```text
input_total=46 accepted=42 rejected=4
```

除上述三个语法无效文件外，还有一个语义无效用例 `semant_test30_getarray_non_lvalue` 被正确拒绝。可执行输入测试在一百万组随机输入下没有发现解释器和编译产物输出不一致。

实际进入 ARM harness 和解释器 hash 对照的 41 个输入程序全部通过；被跳过的程序都是前置 parser/semantic check 已确认应当拒绝的程序。

部分 fuzz 输出如下：

```text
OK interpreter k=9 HW10/test/optloopivextra2.fmj ab4e2331539ea869 1000000
OK interpreter k=9 HW11/test/fibonacci.fmj b424d2ae97575e98 1000000
OK interpreter k=9 HW9/test/opttest9.fmj d4e17d72c5b25503 1000000
```

## 运行时回归测试

为了快速检查后端链接和运行，另外保留了 `runtime-regression`，只跑 HW10 和 HW12 中更接近运行端到端的测试。结果如下：

```text
total=24 pass=24 compile_fail=0 link_fail=0 run_fail=0 workdir=/tmp/final_runtime_regression
```

## 遇到的问题

### 空 label / 空 block

早期集成后，部分程序在中间 IR 中出现空入口 label 或空跳转目标。这个问题会一路传播到 Quad 和汇编，最终导致后端生成不可用控制流。修复方式是在 `ast2tree` 和控制流生成阶段保证每个 block 都有明确 label，所有条件跳转和无条件跳转都指向有效 label。`compile-regression` 中加入了专门扫描，当前 `empty_label_hits=0`。

### 运行期默认值

原作业中的部分阶段默认测试较短，没有完整覆盖未显式初始化变量、字段和函数 fallthrough return。解释器 oracle 对这些行为很敏感，因此 final 中统一了默认初始化和默认返回值，否则解释器和编译运行容易在无输入程序上出现差异。

### 数组布局

数组字面量和运行时 `new int[n]` 必须使用同一布局。最终采用首 word 存长度、后续 word 存元素的布局，并让 `GetArray` 返回 `int` 长度。这样 `putarray`、越界检查和解释器数组模型可以对齐。

### 后端立即数范围

ARM 的 load/store offset 立即数有范围限制。指令选择阶段若把任意常量都折叠进 `[base, #imm]`，大数组或深字段偏移时会产生非法汇编。修复后只在合法范围内折叠，其他情况显式计算地址。

### 函数 prologue 位置

调度阶段曾把入口 label 放在 prologue 之前。这样普通顺序执行没有问题，但循环回边如果跳回入口 label，会重复压栈和调整 `sp/fp`。最终把 prologue 固定在线性化结果的入口 label 之前，保证回边不会重复执行函数入口代码。

## 使用方式

构建：

```text
make build
```

编译单个文件：

```text
final/build/fmjcc --k 9 path/to/file.fmj
```

解释执行单个文件：

```text
final/build/fmjinterp path/to/file.fmj
```

只检查语法和语义：

```text
final/build/fmjinterp --check path/to/file.fmj
```

编译、链接并运行单个文件：

```text
make run-one path/to/file.fmj
```

端到端回归：

```text
make compile-regression
make interpreter-regression
make runtime-regression
make fuzz-regression
```

## 测试结果汇总

```text
make compile-regression
total=280 pass=171 reject=109 crash=0 timeout=0 empty_label_hits=0 workdir=/tmp/final_compile_regression
```

```text
make interpreter-regression
total=280 run_match=167 reject_match=109 timeout_match=4 mismatch=0 compile_only_reject=0 interp_only_reject=0 link_fail=0 run_fail=0 timeout=4 workdir=/tmp/final_interpreter_regression
```

```text
make runtime-regression
total=24 pass=24 compile_fail=0 link_fail=0 run_fail=0 workdir=/tmp/final_runtime_regression
```

```text
make fuzz-regression
fuzz_pass=41 fuzz_fail=0 fuzz_skip=3 iterations=1000000 kset="9" workdir=/tmp/final_fuzz_regression
```

这些结果说明：所有 280 个收集到的 FMJ 文件都被编译器和解释器一致地接受或拒绝；被接受且终止的程序 stdout 和退出码完全一致；含外部输入程序在一百万组随机输入下与解释器 oracle 一致；所有不合法输入均被正确拒绝，没有编译器 crash。

## 参考资料

1. `HW2/docs/FDMJ2026Specification.md`，FMJ 语法和语言边界。
2. HW2-HW12 已完成代码与报告。
3. `HW12/Makefile` 和 `HW12/vendor/libsysy`，ARM 链接和 qemu 运行方式。
