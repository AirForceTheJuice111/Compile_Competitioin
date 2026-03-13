---
title: "HW2 实验报告"
author: "王思宇"
date: \today
using_title: true
using_table_of_content: true
---

# HW2 实验报告

## 参考资料

虎书第 4-5 章，语义分析与类型检查的理论基础，包括符号表设计和类型兼容性规则。

## 实现思路

### 构建NameMap

核心设计：在 `visit(Program*)` 中先预注册所有类名，再依次遍历各类声明。这解决了 FDMJ2026 允许父类声明在子类之后的问题。

- `MainMethod` 被视为特殊的 `__main__` 类中的 `main` 方法。
- 每个方法的形参列表末尾追加一个 `__return__` 伪形参来存储返回类型，便于后续统一处理。
- 变量注册区分类级别和方法级别：根据 `current_visiting_method` 是否为空来判断当前变量属于类还是方法。

### 2. 语义分析（`semantanlyzer.cc`）

#### 类型检查清单（按函数顺序）

以下按照 `semantanlyzer.cc` 中 visitor 函数的定义顺序，列出每个函数内部做了哪些检查和语义注解。

1. `visit(Program*)`：递归访问 `MainMethod` 和所有 `ClassDecl`，无自身检查。

2. `visit(MainMethod*)`：设置 `current_visiting_class = "__main__"`、`current_visiting_method = "main"`，递归访问变量声明和语句列表。

3. `visit(ClassDecl*)`：
   - 检查单层继承：父类不能再有父类（即不允许多层继承）
   - 检查循环继承：父类不能是自身
   - 递归访问类变量声明和方法声明

4. `visit(MethodDecl*)`：
   - 检查方法重写合法性（如果父类有同名方法）：
     - 参数个数必须完全匹配（含 `__return__`）
     - 参数类型必须完全匹配
     - 返回类型允许协变（子类方法的返回类型可以是父类方法返回类型的子类）
   - 递归访问方法内变量声明和语句列表

5. `visit(VarDecl*)`：
   - INT 初始化器只能用于 INT 类型变量
   - 数组初始化器只能用于 ARRAY 类型变量
   - CLASS 类型引用的类必须已声明存在

6. `visit(Formal*)` / `visit(Type*)`：无额外检查（形参在 name map 阶段已处理）。

7. `visit(Nested*)`：递归访问语句块中的所有语句。

8. `visit(If*)`：
   - 条件表达式必须有语义信息（nullptr 检查）
   - 条件表达式类型必须是 INT
   - 递归访问 then/else 分支

9. `visit(While*)`：
   - 条件表达式必须有语义信息（nullptr 检查）
   - 条件表达式类型必须是 INT
   - 维护 `in_a_while_loop` 计数器，递归访问循环体

10. `visit(Assign*)`：
    - 左右表达式必须都有语义信息（nullptr 防御检查）
    - 左侧必须是 lvalue
    - 左右两侧类型必须兼容（支持子类赋给父类变量）

11. `visit(CallStm*)`：
    - 对象必须是 CLASS 类型
    - 为方法名 IdExp 设置 `s_kind="MethodName"` 语义信息
    - 方法必须存在于对象类或其父类中
    - 参数数量必须与形参列表匹配
    - 每个参数的类型必须与对应形参类型兼容

12. `visit(Continue*)`：必须处于 while 循环内（`in_a_while_loop > 0`）。

13. `visit(Break*)`：必须处于 while 循环内（`in_a_while_loop > 0`）。

14. `visit(Return*)`：
    - 返回表达式必须有语义信息（nullptr 检查）
    - 返回类型必须与方法声明的返回类型兼容
    - 语义注解：为 Return 节点设置与返回表达式同类型（不含 type_par）、`lvalue=false`

15. `visit(PutInt*)`：
    - 表达式必须有语义信息（nullptr 检查）
    - 参数类型必须是 INT

16. `visit(PutCh*)`：
    - 表达式必须有语义信息（nullptr 检查）
    - 参数类型必须是 INT

17. `visit(PutArray*)`：
    - 两个参数都必须有语义信息（nullptr 检查）
    - 第一个参数（数量）类型必须是 INT
    - 第二个参数（数组）类型必须是 ARRAY

18. `visit(Starttime*)` / `visit(Stoptime*)`：无需语义检查。

19. `visit(BinaryOp*)`：
    - 左右操作数都必须有语义信息（nullptr 检查）
    - 左右操作数类型都必须是 INT（适用于 `+`, `-`, `*`, `/`, `<`, `>`, `<=`, `>=`, `==`, `!=`, `&&`, `||`）
    - 语义注解：结果类型 INT，`lvalue=false`

20. `visit(UnaryOp*)`：
    - 操作数必须有语义信息（nullptr 检查）
    - 操作数类型必须是 INT（适用于 `-`, `!`）
    - 语义注解：结果类型 INT，`lvalue=false`

21. `visit(ArrayExp*)`：
    - 基址表达式必须有语义信息（nullptr 检查）且类型必须是 ARRAY
    - 索引表达式必须有语义信息（nullptr 检查）且类型必须是 INT
    - 语义注解：结果类型 INT，`lvalue=true`

22. `visit(CallExp*)`：
    - 对象必须是 CLASS 类型
    - 为方法名 IdExp 设置 `s_kind="MethodName"` 语义信息
    - 方法必须存在于对象类或其父类中
    - 参数数量与类型必须匹配
    - 语义注解：结果类型为方法声明的返回类型，`lvalue=false`

23. `visit(ClassVar*)`：
    - 对象必须是 CLASS 类型
    - 字段必须存在于对象类或其父类中
    - 语义注解：结果类型为字段的类型，`lvalue=true`；同时为字段名 IdExp 设置相同类型信息

24. `visit(This*)`：
    - 不能在 `__main__` 中使用
    - 语义注解：类型为当前类（CLASS），`lvalue=false`

25. `visit(Length*)`：
    - 参数必须有语义信息（nullptr 检查）
    - 参数类型必须是 ARRAY
    - 语义注解：结果类型 INT，`lvalue=false`

26. `visit(NewArray*)`：
    - size 表达式必须有语义信息（nullptr 检查）
    - size 表达式类型必须是 INT
    - 语义注解：结果类型 ARRAY（arity=0），`lvalue=false`

27. `visit(NewObject*)`：
    - 类名必须是已声明的类
    - 语义注解：结果类型 CLASS，`lvalue=false`

28. `visit(GetInt*)`：无需检查。语义注解：结果类型 INT，`lvalue=false`。

29. `visit(GetCh*)`：无需检查。语义注解：结果类型 INT，`lvalue=false`。

30. `visit(GetArray*)`：
    - 参数必须有语义信息（nullptr 检查）
    - 参数类型必须是 ARRAY
    - 语义注解：结果类型 INT，`lvalue=false`

31. `visit(IdExp*)`：
    - 按优先级查找标识符：方法局部变量 → 方法形参 → 当前类变量 → 父类变量
    - 找不到则报 "Undeclared identifier" 错误
    - 语义注解：类型与查到的声明一致，`lvalue=true`

32. `visit(IntExp*)`：无需检查。语义注解：结果类型 INT，`lvalue=false`。

33. `visit(OpExp*)`：无需语义分析（运算符标记节点）。

#### 名称查找优先级

`IdExp` 的名称解析遵循以下优先级：

1. 方法局部变量
2. 方法形参
3. 当前类的类变量
4. 父类的类变量

#### 方法查找

当调用 `obj.method()` 时，先在对象类型对应的类中查找方法，找不到则在父类中查找。

#### 类型兼容性

`type_compatible()` 辅助函数处理类型兼容性判断：

- INT 对 INT：直接兼容
- ARRAY 对 ARRAY：直接兼容
- CLASS 对 CLASS：右侧类型必须是左侧类型的子类（或相同类）

### 3. 语义信息注解

语义分析器不仅进行类型检查，还为 AST 节点添加语义注解信息（`AST_Semant`），以便后续阶段使用。具体包括：

- 表达式节点：设置 `s_kind="Value"`，附带 `typeKind`、`lvalue`、以及类型参数（CLASS 的 `cid`、ARRAY 的 `arity`）。
- Return 节点：设置与返回表达式相同的类型信息（不含 type_par 细节），`lvalue=false`。
- ClassVar 字段名 IdExp：设置字段的类型信息，`lvalue=true`。
- CallExp/CallStm 方法名 IdExp：设置 `s_kind="MethodName"`。

### 4. 遇到的坑

1. 类变量访问：最初在 `IdExp` 的名称解析中没有考虑类变量，导致 `bubblesort.fmj` 中 `temp`（一个类变量）无法解析。实际上虽然规范说类变量只能通过 `obj.id` 访问，但测试用例中类方法可以直接引用自己的类变量（相当于隐式 `this`）。

2. 前向引用：父类可以声明在子类之后，需要在 `visit(Program*)` 中预先注册所有类名。

3. 返回类型存储：方法的返回类型是作为形参列表的最后一个元素 `__return__` 存储的，需要在构建 formal list 时正确处理。

4. ARRAY 类型 arity：返回类型为 ARRAY 时，如果原始声明没有 arity，需要补上 `arity=0`，否则创建 Formal 时会报错。

5. Return 节点语义：Return 节点本身也需要设置语义信息（类型与返回表达式一致），但不应包含 `type_par`（如 ARRAY 的 arity），否则 ast2xml 会输出多余的属性。

6. ClassVar 和方法名的语义：`ClassVar` 的字段名 IdExp 和 `CallExp`/`CallStm` 的方法名 IdExp 都需要设置语义信息，因为 `ast2xml` 会对这些子节点调用 `set_position_and_semant`。

7. If/While 条件类型：条件表达式必须是 INT 类型，需要在遍历条件后显式检查。

## Git 提交记录

```
922474c feat: add defensive nullptr checks to all visitors + 15 extra test cases
27caa7e docs(hw2): update report with new fixes and git log
b574258 fix(hw2): add Return/ClassVar.id/MethodName semantic info, add If/While condition type checks
edf1588 merge: pull upstream changes with new tests and tools
09ff324 docs(hw2): add experiment report
17406c9 test(hw2): add additional test cases for inheritance, IO, errors, and class hierarchy
5b8ee79 feat(hw2): implement name maps and semantic analyzer - basic type checking
b69912f make format fixed: hw2
5dc4dee make format: hw2
0daa9e5 chore: ignored .ast
```

## 测试结果
