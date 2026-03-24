---
title: "HW3+HW4 实验报告"
author: "王思宇"
date: \today
using_title: true
using_table_of_content: true
---

# HW3+HW4 实验报告

本次代码是经ai指导下完成（询问了大量设计上的问题，让其给出了一些示例代码），完成后，让ai为代码增添了大量注释，便于日后快速回想起来。

## 参考材料

- 虎书第7章 Translation to Intermediate Code。
- 课程PPT关于 Tiger IR+ 扩展设计的内容。

## Q1.1: `treep.hh`中有许多tigerirp的class，他们分别起到了什么作用？

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

## Q1.2: 相对于虎书中的Tiger IR，我们的Tiger IR+多了哪些内容，为什么需要多的这些内容？

1. **`Return`**：Tiger 没有显式 return，靠特殊寄存器。FDMJ 以 stm 为主体，需要显式返回。
2. **`ExtCall`**：区分类方法调用（`Call`，带 obj）和外部函数调用（`ExtCall`，用于 `putint`/`getint` 等运行时函数）。
3. **`FuncDecl` 的 `last_temp_num`/`last_label_num`**：记录函数用到的最大编号，方便后续寄存器分配。
4. **`Exp` 带 `Type` 字段**：区分 `INT` 和 `PTR`，虎书不区分。
5. **`Seq` 用 `vector<Stm*>`**：虎书的 SEQ 是二叉节点，这里用列表更灵活。

## Q2: 在不带class的翻译情况下，你是如何完成各成分的翻译的？

翻译用 Visitor 模式遍历 AST。翻译结果以 `Tr_Exp` 形式传递：`Tr_ex`（有值表达式）、`Tr_nx`（语句）、`Tr_cx`（条件，附带待回填的 Patch_list）。三者可互转。

### 变量分配

进 MainMethod 时，从 NameMaps 拿到局部变量（set 天然字母序），依次分配 Temp，再给形参（含 `_^return^_main`）分配。

### 算术运算

左右 `unEx` 后构造 `Binop`。一元取负翻译为 `0 - exp`。

### 比较运算

翻译为 `Tr_cx`，构造 `Cjump`，分配两个标签放入 Patch_list，留给上层回填。

### 逻辑运算（短路求值）

`&&` 把左 true_list 回填到 middle 标签（左真则继续判右），false_list 合并左右的。`||` 对称地把左 false_list 回填到 middle。`!` 直接交换 true/false list。

### 赋值

`Move(dst.unEx, src.unEx)`。

### If 条件

先翻译条件并立即 `unCx`，再翻译 then/else body，最后分配 true/false/end 三个标签并回填。结构：`Seq(cond, Label(true), then, Jump(end), Label(false), else, Label(end))`。

### While 循环

翻译条件并 `unCx`，分配 start/body/done 标签，设置 continue/break 指向 start/done，翻译循环体，恢复外层 continue/break。结构：`Seq(Label(start), cond, Label(body), body, Jump(start), Label(done))`。

### Continue / Break

直接 `Jump` 到对应标签。嵌套循环通过保存/恢复外层标签支持。

### 外部函数调用

`putint`/`putch` 等作为 `ExpStm(ExtCall(...))`（丢弃返回值），`getint`/`getch` 等作为 `ExtCall(...)`（保留返回值）。

### Return

`tree::Return(exp.unEx)`。

## Q3: 你是如何重命名method的？你是如何处理this的？你是如何记录不同class的变量和方法的？你是如何处理多态的？你是如何翻译有关class的操作的？

### 如何重命名method

每个类方法重命名为 `ClassName^MethodName`（如 `fib^f`），main 方法重命名为 `__$main__^main`。这样不同类中的同名方法会生成不同的函数，避免命名冲突。

### main method和class method的参数列表有何不同？如何处理class method中的this？

main method 的参数列表只有 `_^return^_main`（类型为 INT）；class method 的参数列表中，`_^return^_method` 形参类型改为 PTR，它就是 `this` 指针，作为 FuncDecl.args 的唯一元素。

在方法体中，`this` 翻译为 `TempExp(this_temp)`，其中 `this_temp` 是 `_^return^_method` 对应的 Temp。方法的其他形参和局部变量分别从 NameMaps 获取并分配 Temp。额外分配一个 scratch temp（使 last_temp 符合约定）。

### 如何记录不同class的变量和方法（Unified Object Record）

所有类共享同一个 object layout。变量 key 为 `ClassName^VarName`（支持字段隐藏），方法 key 为方法名（同名方法共享偏移，支持多态）。`generate_class_table` 跳过 `__$main__`，收集所有类的字段和方法名，按字典序排列（变量在前，方法在后），每个条目占 `address_length=4` 字节。

### 如何处理多态

多态通过两个机制实现：

1. **UOR 中方法偏移共享**：同名方法在所有类中占据相同的 UOR 偏移，因此通过对象指针获取方法指针时，无论实际类型如何都能找到正确位置。
2. **NewObject 中存储实际实现**：创建对象时，通过 `resolve_method_class` 沿继承链向上查找方法的实际实现类，将 `Name(sname="ImplClass^method")` 存入对应偏移。子类如果 override 了方法，就存子类的函数指针；否则存父类的。

调用时通过 `Mem[obj + method_pos]` 间接获取函数指针（虚分派），运行时自动跳到正确的实现。

### 如何翻译有关class的操作（初始化、访问类变量、访问类方法……）

#### NewObject（对象创建）

`new ClassName()` 翻译为 ESeq：先 `malloc(UOR总大小)` 得到对象指针，然后把该类能解析到的每个方法的函数指针（`Name(sname="ImplClass^method")`）存到对应偏移位置。

#### ClassVar（字段访问）

`obj.field` 翻译为 `Mem[obj + offset]`，其中 offset 通过 `resolve_var_class` 沿继承链查找字段声明类，再从 class_table 获取。对链式访问（如 `this.c.j`），通过 `class_var_class_name` 传递中间对象的类型。

#### CallExp / CallStm（虚方法调用）

`obj.method(args)` 翻译为 `Call(id="method", obj=Mem[obj + method_offset], args=[obj, ...])`。函数指针通过 `Mem[obj + method_pos]` 间接获取，实现虚分派。CallStm 包装在 ExpStm 中丢弃返回值。

#### NewArray（数组创建）

`new int[size]` 翻译为 ESeq：`malloc((size+1)*4)`，在 `[0]` 存长度，返回指针。

#### 数组字面量初始化

`int[] a = {1,2,3}` 翻译为一系列 Move：`malloc(16)`，`Mem[a]=3`（长度），`Mem[a+4]=1`, `Mem[a+8]=2`, `Mem[a+12]=3`。VarDecl 产生的 Seq 会被 flatten 到方法体的 Seq 中，避免嵌套。

#### ArrayExp（下标访问+越界检查）

`arr[idx]` 翻译为 ESeq：先将复杂的 arr/idx 表达式materialize成临时变量，然后做越界检查：

1. `len = Mem[arr]`（读长度）
2. `CJump(idx >= 0, ok, exit)`
3. `CJump(idx >= len, exit, done)`
4. exit: `ExtCall("exit", {-1})`

最后返回 `Mem[arr + (idx+1)*4]`。

#### Length（数组长度）

`arr.length` 翻译为 `Mem[arr]`（长度存在数组首地址）。

## Git 提交记录

```
aac9f92 Merge remote-tracking branch 'origin/master' into hw3
c5d04d5 HW4: add Q3 section to report
57825c2 HW4: array/class AST->IRP translation (irtest9-20 pass)
840c1df before hw4
76a9bd5 Merge HW4 test cases from origin
a1131f1 HW3: rm int init, inline Seq/args, simplify report
7e904d6 HW3: add extra test cases irtest9-12
a9e9a5c HW3: implement AST to IRP translation for main-only programs (all 8 tests pass)
67c6223 HW3 tests added
1c436d1 HW3 files
```

## 测试结果

irtest1–20 与参考答案完全一致，irtest21–22 在 temp 编号分配顺序上存在差异但 IR 语义显然等价。

`make run` 输出（截取部分）：

```
Reading irtest1
------Reading AST from : irtest1.2-semant.ast------------
Saving AST (XML) to: irtest1.2-semant-debug.ast
Converting AST to IR
Compiler Configuration:: address_length: 4; memory_alignment: 4; int_length: 4; float_length: 4; double_length: 8
======Class Table:
======End of Class Table
Saving IR (XML) to: irtest1.3.irp
-----Done---
...
Reading irtest14
------Reading AST from : irtest14.2-semant.ast------------
Saving AST (XML) to: irtest14.2-semant-debug.ast
Converting AST to IR
Compiler Configuration:: address_length: 4; memory_alignment: 4; int_length: 4; float_length: 4; double_length: 8
======Class Table:
var A^i has pos 0
method f has pos 4
======End of Class Table
Saving IR (XML) to: irtest14.3.irp
-----Done---
...
Reading irtest22
------Reading AST from : irtest22.2-semant.ast------------
Saving AST (XML) to: irtest22.2-semant-debug.ast
Converting AST to IR
Compiler Configuration:: address_length: 4; memory_alignment: 4; int_length: 4; float_length: 4; double_length: 8
======Class Table:
method f has pos 0
======End of Class Table
Saving IR (XML) to: irtest22.3.irp
-----Done---
...
Reading irtest9
------Reading AST from : irtest9.2-semant.ast------------
Saving AST (XML) to: irtest9.2-semant-debug.ast
Converting AST to IR
Compiler Configuration:: address_length: 4; memory_alignment: 4; int_length: 4; float_length: 4; double_length: 8
======Class Table:
======End of Class Table
Saving IR (XML) to: irtest9.3.irp
-----Done---
```
