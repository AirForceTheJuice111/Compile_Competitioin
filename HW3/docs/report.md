---
title: "HW3 实验报告"
author: "王思宇"
date: \today
using_title: true
using_table_of_content: true
---

# HW3 实验报告

## 参考材料

- 虎书第7章 Translation to Intermediate Code，主要参考了 `Tr_ex`/`Tr_nx`/`Tr_cx` 以及 Patch List 回填机制。
- 课程PPT关于 Tiger IR+ 扩展设计的内容。

## Q1.1: treep.hh 中各 class 的作用

顶层：`Program` 包含函数列表；`FuncDecl` 表示一个函数，含函数名、参数、函数体、返回类型、最大temp/label编号。

语句（Stm）：

- `Seq`：语句序列，按顺序执行
- `LabelStm`：跳转目标标签
- `Jump`：无条件跳转
- `Cjump`：条件跳转，根据 relop 比较结果跳 true/false 标签
- `Move`：赋值，把 src 移入 dst
- `ExpStm`：执行表达式但忽略返回值，只保留副作用
- `Return`：返回语句（Tiger IR+ 扩展）

表达式（Exp）：

- `Binop`：二元运算（+、-、*、/、xor 等）
- `Mem`：内存访问，用于数组/对象成员存取
- `TempExp`：临时变量（类似无限寄存器）
- `Eseq`：先执行语句再返回表达式值
- `Name`：标签转地址
- `Const`：整型常量
- `Call`：类方法调用（带 obj 指针）
- `ExtCall`：外部函数调用（Tiger IR+ 扩展）

## Q1.2: Tiger IR+ 相对于 Tiger IR 的扩展

1. **`Return`**：Tiger 没有显式 return，靠特殊寄存器。FDMJ 以 stm 为主体，需要显式返回。
2. **`ExtCall`**：区分类方法调用（`Call`，带 obj）和外部函数调用（`ExtCall`，用于 `putint`/`getint` 等运行时函数）。
3. **`FuncDecl` 的 `last_temp_num`/`last_label_num`**：记录函数用到的最大编号，方便后续寄存器分配。
4. **`Exp` 带 `Type` 字段**：区分 `INT` 和 `PTR`，虎书不区分。
5. **`Seq` 用 `vector<Stm*>`**：虎书的 SEQ 是二叉节点，这里用列表更灵活。

## Q2: 不带 class 的翻译

翻译用 Visitor 模式遍历 AST。翻译结果以 `Tr_Exp` 形式传递：`Tr_ex`（有值表达式）、`Tr_nx`（语句）、`Tr_cx`（条件，附带待回填的 Patch_list）。三者可互转。

**变量分配**：进 MainMethod 时，从 NameMaps 拿到局部变量（set 天然字母序），依次分配 Temp，再给形参（含 `_^return^_main`）分配。

**算术运算**：左右 `unEx` 后构造 `Binop`。一元取负翻译为 `0 - exp`。

**比较运算**：翻译为 `Tr_cx`，构造 `Cjump`，分配两个标签放入 Patch_list，留给上层回填。

**逻辑运算（短路）**：`&&` 把左 true_list 回填到 middle 标签（左真则继续判右），false_list 合并左右的。`||` 对称地把左 false_list 回填到 middle。`!` 直接交换 true/false list。

**赋值**：`Move(dst.unEx, src.unEx)`。

**If**：先翻译条件并**立即 `unCx`**（关键：必须在翻译 body 前调用，否则 body 里的比较会抢占标签编号），再翻译 then/else body，最后分配 true/false/end 三个标签并回填。结构：`Seq(cond, Label(true), then, Jump(end), Label(false), else, Label(end))`。

**While**：翻译条件并 `unCx`，分配 start/body/done 标签，设置 continue/break 指向 start/done，翻译循环体，恢复外层 continue/break。结构：`Seq(Label(start), cond, Label(body), body, Jump(start), Label(done))`。

**Continue/Break**：直接 `Jump` 到对应标签。嵌套循环通过保存/恢复外层标签支持。

**外部函数调用**：`putint`/`putch` 等作为 `ExpStm(ExtCall(...))`（丢弃返回值），`getint`/`getch` 等作为 `ExtCall(...)`（保留返回值）。

**Return**：`tree::Return(exp.unEx)`。

## Git 提交记录

```
7e904d6 HW3: add extra test cases irtest9-12 (if-no-else, not-op, while-sum, nested-while)
a9e9a5c HW3: implement AST to IRP translation for main-only programs (all 8 tests pass)
c6e6b18 Merge branch 'master' of https://gitee.com/fudanCompiler/fducompilerh2026
67c6223 HW3 tests added
1c436d1 HW3 files
27f32a0 HW3 files
```

## 测试结果

