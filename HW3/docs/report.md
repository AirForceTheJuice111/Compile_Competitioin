---
title: "HW3 实验报告"
author: "王思宇"
date: \today
using_title: true
using_table_of_content: true
---

# HW3 实验报告

## 参考材料

1. **虎书 (Modern Compiler Implementation in C/Java)**，Andrew W. Appel。第7章 Translation to Intermediate Code 是本次作业的核心参考，详细介绍了 `Tr_ex`/`Tr_nx`/`Tr_cx` 三种翻译表达式类型、Patch List 的回填机制以及条件表达式的短路求值翻译。
2. **课程PPT**：Tiger IR+ 的扩展设计，包括 `ExtCall`、`Return`、`ExpStm` 等新增节点。
3. **tinyxml2 文档**：https://github.com/leethomason/tinyxml2 ，用于 XML 格式的 AST 输入解析与 IRP 输出序列化。

## Q1.1: Tiger IR+ 中各 class 的作用

`treep.hh` 中定义了 Tiger IR+ 的中间表示节点，分为语句（Stm）和表达式（Exp）两大类，以及顶层结构。

### 顶层结构

- `tree::Program`：整个程序的根节点，包含一个函数声明列表（`funcdecllist`）。翻译后的程序由一系列函数组成，第一个是 main 函数。
- `tree::FuncDecl`：函数声明节点，包含函数名（`name`，格式为 `类名^方法名`）、参数列表（`args`，一组 `Temp`）、函数体语句（`stm`）、返回类型（`return_type`）、以及该函数使用的最大临时变量编号（`last_temp_num`）和最大标签编号（`last_label_num`）。

### 语句节点（Stm）

- `tree::Seq`：语句序列，包含一个语句列表（`sl`），按顺序执行。
- `tree::LabelStm`：标签语句，作为程序跳转的目的地，类似于 goto 标签的作用。
- `tree::Jump`：无条件跳转，跳转到指定标签。
- `tree::Cjump`：条件跳转，比较两个表达式（`left`、`right`），根据关系运算符（`relop`）的结果跳转到 `t`（真）或 `f`（假）标签。
- `tree::Move`：赋值语句，将 `src` 表达式的值移动到 `dst`（可以是 `TempExp` 或 `Mem`）。
- `tree::ExpStm`：表达式语句，执行一个表达式但忽略其返回值，只保留副作用（如函数调用）。
- `tree::Return`：返回语句，返回一个表达式的值。这是 Tiger IR+ 相对于原始 Tiger IR 的扩展。

### 表达式节点（Exp）

- `tree::Binop`：二元运算表达式，支持 `+`、`-`、`*`、`/`、`&&`、`||`、`xor` 等操作。
- `tree::Mem`：内存访问表达式，对一个地址表达式取内存值，用于数组存取和对象成员访问。
- `tree::TempExp`：临时变量表达式，将一个 `Temp`（类似无限寄存器）转换为表达式。每个源程序变量会被分配一个唯一的 `Temp`。
- `tree::Eseq`：表达式-语句序列，先执行一个语句（`stm`），然后返回一个表达式（`exp`）的值。用于在表达式中嵌入副作用。
- `tree::Name`：标签名表达式，将标签转换为地址（指针类型），可用于函数指针等场景。
- `tree::Const`：整型常量表达式，包含一个整数值。
- `tree::Call`：类方法调用表达式，包含方法名（`id`）、对象指针（`obj`）和参数列表（`args`）。
- `tree::ExtCall`：外部函数调用表达式，包含外部函数名（`extfun`）和参数列表（`args`）。这是 Tiger IR+ 的扩展，用于调用 `putint`、`getint` 等运行时库函数。

## Q1.2: Tiger IR+ 相对于 Tiger IR 的扩展

相对于虎书中的标准 Tiger IR，我们的 Tiger IR+ 做了以下扩展：

1. **`tree::Return` 语句**：标准 Tiger IR 中没有显式的 return 语句，函数返回值通过特殊寄存器（如 RV）隐式处理。Tiger IR+ 添加了 `Return` 节点，使函数返回值的表达更加直接和清晰。这是因为 FDMJ 以语句（stm）为主体而非 Tiger 以表达式（exp）为主体，需要一种显式的返回机制。

2. **`tree::ExpStm` 语句**：将一个表达式包装为语句，忽略其返回值。虎书中通过 `EXP` 语句实现类似功能，Tiger IR+ 中的 `ExpStm` 更加明确地表达了"执行表达式但丢弃结果"的语义，方便处理如外部函数调用等只需要副作用的情况。

3. **`tree::ExtCall` 表达式**：标准 Tiger IR 只有 `CALL` 节点来调用函数。Tiger IR+ 区分了类方法调用（`Call`，带有对象指针 `obj`）和外部函数调用（`ExtCall`，不带对象指针）。这是因为 FDMJ 需要调用运行时库函数（如 `putint`、`getint`、`putch`、`getch`、`putarray`、`getarray`、`starttime`、`stoptime`），这些函数不属于任何类，调用方式与类方法不同。

4. **`tree::FuncDecl` 的 `last_temp_num` 和 `last_label_num`**：虎书中不需要在函数声明中记录临时变量和标签的使用范围。Tiger IR+ 额外记录了每个函数使用的最大临时变量编号和标签编号，这便于后续阶段（如寄存器分配）了解函数的资源使用情况。

5. **表达式的 `Type` 属性**：每个 `tree::Exp` 都有 `type` 字段（`INT` 或 `PTR`），标准 Tiger IR 中不区分表达式类型。这是因为 FDMJ 需要区分整数类型和指针类型（数组、对象），以便在后续翻译阶段正确处理不同宽度的数据。

6. **`tree::Seq` 使用 `vector<Stm*>`**：虎书中的 `SEQ` 是一个二叉节点（只有两个子语句），Tiger IR+ 使用了一个语句列表，更灵活地表示任意长度的语句序列，避免了深度嵌套的二叉结构。

## Q2: 不带 class 的翻译实现

### 整体翻译框架

翻译采用 Visitor 模式，`ASTToTreeVisitor` 继承 `fdmj::AST_Visitor`，遍历 AST 节点，将每个节点翻译为对应的 Tiger IR+ 节点。

翻译结果通过两个成员变量传递：

- `visit_tree_result`：用于传递顶层 IR 树节点（如 `FuncDecl`、`Program`）
- `visit_exp_result`：用于传递 `Tr_Exp*`（`Tr_ex`/`Tr_nx`/`Tr_cx` 三种形式之一）

`Tr_Exp` 系统（参见虎书第7章）是翻译过程的核心：

- `Tr_ex`：包含一个 `tree::Exp*`，表示有返回值的表达式
- `Tr_nx`：包含一个 `tree::Stm*`，表示无返回值的语句
- `Tr_cx`：包含一个条件跳转语句和两个 `Patch_list`（true_list 和 false_list），表示条件表达式，其跳转目标待回填

三者之间可以自由转换（`unEx`/`unNx`/`unCx`），转换时会自动分配必要的临时变量和标签。

### 变量分配

进入 `MainMethod` 翻译时，首先通过 `generate_method_var_table()` 为所有变量分配临时变量（`Temp`）：

1. 先从 `NameMaps` 获取局部变量集合（`set<string>`，按字母序排列），依次分配 `Temp`
2. 再为形式参数分配 `Temp`（包括 `_^return^_main` 返回值变量）

这样每个变量都有唯一的 `Temp` 编号，后续通过 `method_var_table` 查找变量名到 `Temp` 的映射。

### 变量声明（含初始化）

对于 `int x = val;` 形式的声明，翻译为 `Move(TempExp(x的temp), Const(val))`。无初始化值的声明不产生 IR 语句。

### 算术运算

对于 `+`、`-`、`*`、`/` 运算，翻译时分别对左右子表达式调用 `unEx` 获取 `tree::Exp*`，然后构造 `tree::Binop` 节点，返回 `Tr_ex`。

对于一元 `-`（取负），翻译为 `0 - exp`，即 `Binop(INT, "-", Const(0), exp)`。

### 比较运算

对于 `<`、`>`、`<=`、`>=`、`==`、`!=`，翻译结果为 `Tr_cx`：

1. 分别翻译左右操作数并调用 `unEx` 得到 `tree::Exp*`
2. 分配两个新标签作为 true 和 false 目标
3. 构造 `Cjump(relop, left, right, true_label, false_label)`
4. 创建相应的 `Patch_list`，将 true_label 和 false_label 分别加入
5. 返回 `Tr_cx(true_list, false_list, cjump)`

这些标签目标会在上层语句（if/while）翻译时通过 `Patch_list::patch()` 统一回填。

### 逻辑运算（短路求值）

**`&&` 运算**：

1. 翻译左操作数并调用 `unCx` 得到 `Tr_cx`
2. 翻译右操作数并调用 `unCx` 得到 `Tr_cx`
3. 分配一个 middle 标签
4. 将左操作数的 true_list 回填到 middle（左为真时继续判断右边）
5. 合并语句为 `Seq(left_stm, Label(middle), right_stm)`
6. 结果的 true_list = 右操作数的 true_list，false_list = 左操作数的 false_list + 右操作数的 false_list

**`||` 运算**：

1. 翻译方式类似，但将左操作数的 false_list 回填到 middle（左为假时继续判断右边）
2. 结果的 true_list = 左操作数的 true_list + 右操作数的 true_list，false_list = 右操作数的 false_list

**`!` 运算**：

翻译操作数后调用 `unCx`，然后交换 true_list 和 false_list，即 `Tr_cx(false_list, true_list, stm)`。

### 赋值

翻译为 `Move(dst, src)`：对左侧变量调用 `unEx` 获取目标 `TempExp`，对右侧表达式调用 `unEx` 获取源 `Exp`，构造 `tree::Move`，包装为 `Tr_nx`。

### 条件语句（If）

If 语句的翻译关键在于**标签分配顺序**：

1. 翻译条件表达式，**立即**调用 `unCx` 获取 `Tr_cx`（此时分配条件相关的标签）
2. 翻译 then-body 和 else-body
3. 分配三个标签：true_label、false_label、end_label
4. 回填条件的 true_list 到 true_label，false_list 到 false_label
5. 组装: `Seq(cond_stm, Label(true), then_body, Jump(end), Label(false), else_body, Label(end))`

**重要的坑**：条件的 `unCx()` 必须在翻译 then/else body 之前调用，否则 body 中如果有比较运算也会分配标签，导致标签编号顺序不匹配预期输出。

### 循环语句（While）

1. 翻译条件表达式，调用 `unCx`
2. 分配三个标签：start_label、body_label、done_label
3. 设置 `continue_label = start_label`，`break_label = done_label`
4. 翻译循环体
5. 恢复外层的 continue/break 标签
6. 组装: `Seq(Label(start), cond_stm, Label(body), body_stm, Jump(start), Label(done))`

回填方式：条件的 true_list 回填到 body_label，false_list 回填到 done_label。

### Continue 和 Break

- `Continue`：翻译为 `Jump(continue_label)`，跳回 while 循环的开始位置
- `Break`：翻译为 `Jump(break_label)`，跳到 while 循环结束位置

支持嵌套循环：进入内层 while 时保存外层的 continue/break 标签，退出时恢复。

### 外部函数调用

所有外部函数调用翻译为 `ExtCall` 节点：

- `putint(exp)` → `ExpStm(ExtCall("putint", {exp}))`（作为语句，丢弃返回值）
- `putch(exp)` → `ExpStm(ExtCall("putch", {exp}))`
- `getint()` → `ExtCall("getint", {})`（作为表达式，保留返回值）
- `getch()` → `ExtCall("getch", {})`
- `putarray(n, arr)` → `ExpStm(ExtCall("putarray", {n, arr}))`
- `getarray(exp)` → `ExtCall("getarray", {exp})`
- `starttime()` → `ExpStm(ExtCall("starttime", {}))`
- `stoptime()` → `ExpStm(ExtCall("stoptime", {}))`

其中 `putint`/`putch`/`putarray`/`starttime`/`stoptime` 作为语句处理（`Tr_nx`），`getint`/`getch`/`getarray` 作为表达式处理（`Tr_ex`）。

### Return

翻译为 `tree::Return(exp)`，对返回表达式调用 `unEx` 获取值。

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

