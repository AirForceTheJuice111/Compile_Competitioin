---
title: "HW2 实验报告"
author: "王思宇"
date: \today
using_title: true
using_table_of_content: true
---

# HW2 实验报告

## Quiz 报告

分别有两个检查：

### namemap注册类名时，判断是否有子父类immutability不一致性

首先定义辅助函数用以检查immutability：

```cpp
bool check_immutability_by_name(string class_name) {
    return class_name.length() >= 10 && class_name.substr(class_name.length() - 10) == "_Immutable";
}
```

然后，visit(ClassDecl *node)时，

1. 设置immutability：

```cpp
   if(check_immutability_by_name(class_name)) {
        name_maps->set_class_immutable(class_name, true);
    }
```

2. 检查子父类是否有不一致性：

```cpp
if (!check_immutability_by_name(class_name) && check_immutability_by_name(parent_name)) {
    cerr << "Error: at position " << node->eid->get_pos()->to_str() << endl;
    cerr << "Error: Class " << class_name << " extends immutable class " << parent_name << " but is not marked as immutable. Immutability is hereditary." << endl;
}
if (check_immutability_by_name(class_name) && !check_immutability_by_name(parent_name)) {
    cerr << "Error: at position " << node->eid->get_pos()->to_str() << endl;
    cerr << "Error: Class " << class_name << " extends mutable class " << parent_name << " but is marked as immutable. Immutability is hereditary." << endl;
}
```

如上所示，有不一致则报错。

之所以没有使用 name_maps->is_class_immutable(parent_name) 来判断父类是否immutable，是因为在子类定义时父类可能还没被定义。

编写额外的测试用例验证了两种子父类不匹配的情况都覆盖到了：

```
public class A {
    int a;
}
public class C_Immutable extends A {
    int c;
}
```

结果：

```
Error: at position Position(sline: 21, scolumn: 34, eline: 21, ecolumn: 34)
Error: Class C_Immutable extends mutable class A but is marked as immutable. Immutability is hereditary.
```

```
public class B extends TestClass_Immutable {
    int i;
}
```

结果：

```
Error: at position Position(sline: 14, scolumn: 24, eline: 14, ecolumn: 42)
Error: Class B extends immutable class TestClass_Immutable but is not marked as immutable. Immutability is hereditary.
```

### 是否有Assign node修改了immutable class的ClassVar

显然只有Assign node能够修改一个变量的值。因此，只需在visit(Assign* node)的时候判断一下：

1. node->left是否为ClassVar
2. 如果是，其obj的class_name是否在name_maps中被注册为immutable，若是则报错

相关代码：

```cpp
    // get the class of the left node if it's ClassVar, and check immutability
    if (node->left->getASTKind() == ASTKind::ClassVar) {
        auto cast_left = dynamic_cast<ClassVar*>(node->left);
        AST_Semant *obj_sem = (cast_left->obj != nullptr) ? semant_map->getSemant(cast_left->obj) : nullptr;
        string class_name = get<string>(obj_sem->get_type_par());
        if (name_maps->is_class_immutable(class_name)) {
            cerr << "Error: at position " << node->get_pos()->to_str() << endl;
            cerr << "Cannot assign to a member of an immutable object of type " << class_name << endl;
        }
    }
```

这并没有过度检查（没有检查成深不可变类），因为只有A.a这样的形式的变量才为A的ClassVar，像：

1. A.a[0]，其最外层为ArrayExp，因此不是。
2. A.a.b，其最外层也为ClassVar，但检查的是A.a这个class是否immutable，不是检查的A，也被我们的检查覆盖到了。

immutabletest1输出结果：

```
Error: at position Position(sline: 3, scolumn: 5, eline: 3, ecolumn: 21)
Cannot assign to a member of an immutable object of type A_Immutable
Error: at position Position(sline: 4, scolumn: 5, eline: 4, ecolumn: 36)
Cannot assign to a member of an immutable object of type A_Immutable
```

在其基础上，编写了额外的测试用例确认没检测成深不可变类：

```
public int main() {
    class A_Immutable a;
    a.b[2] = 1;
}

public class TestClass_Immutable {
    int a;
    int[] b = {1,2,3};

    public class TestClass_Immutable test1(class TestClass_Immutable a) {
        int[] a;
    }
}

public class A_Immutable extends TestClass_Immutable {
   int a;
   class TestClass_Immutable o;
}
```

a.b[2] = 1并未报错，因此无误。

## 参考资料

虎书第 4-5 章，语义分析与类型检查的理论基础，包括符号表设计和类型兼容性规则。

## 构建NameMap

核心是在 `visit(Program*)` 中先预注册所有类名，再依次遍历各类声明。这解决了 FDMJ2026 允许父类声明在子类之后的问题。

一些比较特殊的点如下：（写在这也防止我自己忘了）

- `MainMethod` 被视为特殊的 `__main__` 类中的 `main` 方法。
- 每个方法的形参列表末尾追加一个 `__return__` 伪形参来存储返回类型，便于后续统一处理。
- 变量注册区分类级别和方法级别，根据 `current_visiting_method` 是否为空来判断当前变量属于类还是方法。

## 语义分析

### 实现了的语义注解和类型检查

- `visit(ClassDecl*)`：
   - 检查单层继承：父类不能再有父类
   - 检查循环继承：父类不能是自身

- `visit(MethodDecl*)`：
   - 检查方法override合法性：
     - 参数个数必须完全匹配（含 `__return__`）
     - 参数类型必须完全匹配
     - 返回类型允许协变

- `visit(VarDecl*)`：
   - INT 初始化器只能用于 INT 类型变量
   - 数组初始化器只能用于 ARRAY 类型变量
   - CLASS 类型引用的类必须已声明存在

- `visit(If*)`：
   - 条件表达式必须有语义信息（nullptr 检查）
   - 条件表达式类型必须是 INT
   - 递归访问 then/else 分支

- `visit(While*)`：
   - 条件表达式必须有语义信息（nullptr 检查）
   - 条件表达式类型必须是 INT

- `visit(Assign*)`：
    - 左右表达式必须都有语义信息（nullptr 检查）
    - 左侧必须是 lvalue
    - 左右两侧类型必须兼容（支持子类赋给父类变量）

- `visit(CallStm*)`：
    - 对象必须是 CLASS 类型
    - 方法必须存在于对象类或其父类中
    - 参数数量必须与形参列表匹配
    - 每个参数的类型必须与对应形参类型兼容

- `visit(Continue*)`：必须处于 while 循环内（`in_a_while_loop > 0`）。

- `visit(Break*)`：必须处于 while 循环内（`in_a_while_loop > 0`）。

- `visit(Return*)`：
    - 返回表达式必须有语义信息（nullptr 检查）
    - 返回类型必须与方法声明的返回类型兼容

- `visit(PutInt*)`：
    - 表达式必须有语义信息（nullptr 检查）
    - 参数类型必须是 INT

- `visit(PutCh*)`：
    - 表达式必须有语义信息（nullptr 检查）
    - 参数类型必须是 INT

- `visit(PutArray*)`：
    - 两个参数都必须有语义信息（nullptr 检查）
    - 第一个参数（数量）类型必须是 INT
    - 第二个参数（数组）类型必须是 ARRAY

- `visit(BinaryOp*)`：
    - 左右操作数都必须有语义信息（nullptr 检查）
    - 左右操作数类型都必须是 INT
    - 语义注解：结果类型 INT，`lvalue=false`

- `visit(UnaryOp*)`：
    - 操作数必须有语义信息（nullptr 检查）
    - 操作数类型必须是 INT
    - 语义注解：结果类型 INT，`lvalue=false`

- `visit(ArrayExp*)`：
    - 基址表达式必须有语义信息（nullptr 检查）且类型必须是 ARRAY
    - 索引表达式必须有语义信息（nullptr 检查）且类型必须是 INT
    - 语义注解：结果类型 INT，`lvalue=true`

- `visit(CallExp*)`：
    - 对象必须是 CLASS 类型
    - 为方法名 IdExp 设置 `s_kind="MethodName"` 语义信息
    - 方法必须存在于对象类或其父类中
    - 参数数量与类型必须匹配
    - 语义注解：结果类型为方法声明的返回类型，`lvalue=false`

- `visit(ClassVar*)`：
    - 对象必须是 CLASS 类型
    - 字段必须存在于对象类或其父类中

- `visit(This*)`：
    - 检查：不能在 `__main__` 中使用

- `visit(Length*)`：
    - 参数必须有语义信息（nullptr 检查）
    - 参数类型必须是 ARRAY

- `visit(NewArray*)`：
    - size 表达式必须有语义信息（nullptr 检查）
    - size 表达式类型必须是 INT

- `visit(NewObject*)`：
    - 类名必须是已声明的类
    - 语义注解：结果类型 CLASS，`lvalue=false`

- `visit(GetArray*)`：
    - 参数必须有语义信息（nullptr 检查）
    - 参数类型必须是 ARRAY

- `visit(IdExp*)`：
    - 按优先级查找标识符：方法局部变量 → 方法形参 → 当前类变量 → 父类变量
    - 找不到则报 "Undeclared identifier" 错误

### 名称查找优先级

`IdExp` 的名称解析遵循以下优先级：

1. 方法局部变量
2. 方法形参
3. 当前类的类变量
4. 父类的类变量

### 方法查找优先级

当调用 `obj.method()` 时，先在对象类型对应的类中查找方法，找不到则在父类中查找。

### 类型兼容性

`type_compatible()` 辅助函数处理类型兼容性判断：

- INT 对 INT：直接兼容
- ARRAY 对 ARRAY：直接兼容
- CLASS 对 CLASS：右侧类型必须是左侧类型的子类（或相同类）

### 语义信息注解

语义分析器也为 AST 节点添加语义注解信息（`AST_Semant`），以便后续阶段使用。具体包括：

- 表达式节点：设置 `s_kind="Value"`，附带 `typeKind`、`lvalue`、以及类型参数（CLASS 的 `cid`、ARRAY 的 `arity`）。
- Return 节点：设置与返回表达式相同的类型信息（不含 type_par 细节），`lvalue=false`。
- ClassVar 字段名 IdExp：设置字段的类型信息，`lvalue=true`。
- CallExp/CallStm 方法名 IdExp：设置 `s_kind="MethodName"`。

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

测试结果文本如下，其中还有一些我新增的测试用例：

```
 wsy@21:17:53  ~/fducompilerh2026/HW2   hw2 ±  make run                       
cd /home/wsy/fducompilerh2026/HW2/test && \
for file in $(ls .); do \
        if [ "${file##*.}" = "fmj" ]; then \
            echo "Parsing ${file%%.*}"; \
                /home/wsy/fducompilerh2026/HW2/vendor/parser/parser "${file%%.*}"; \
                echo "Checking ${file%%.*}"; \
                /home/wsy/fducompilerh2026/HW2/build/tools/main/main "${file%%.*}"; \
        fi \
done; \
cd .. > /dev/null 2>&1 
Parsing bubblesort
------Parsing fmj source file: bubblesort.fmj------------
Error at lines [2:11-2:11]: syntax error, unexpected NONNEGATIVEINT, expecting '{'
Error: parsing failed
AST is not valid!
Checking bubblesort
Read AST from file: bubblesort.2.ast
Converting XML to AST...
Semantic analysis...
--Making Name Maps...
--Analyzing Semantics...
Convert AST to XML with Semantic Info...
Parsing example
------Parsing fmj source file: example.fmj------------
Error at lines [3:11-3:11]: syntax error, unexpected NONNEGATIVEINT, expecting '{'
Error: parsing failed
AST is not valid!
Checking example
Read AST from file: example.2.ast
Converting XML to AST...
Semantic analysis...
--Making Name Maps...
--Analyzing Semantics...
Error: at position Position(sline: 16, scolumn: 24, eline: 16, ecolumn: 24)
Error: Undeclared identifier: j
Parsing extratest1
------Parsing fmj source file: extratest1.fmj------------
Convert AST  to XML...
Writing AST to file: extratest1.2.ast
Checking extratest1
Read AST from file: extratest1.2.ast
Converting XML to AST...
Semantic analysis...
--Making Name Maps...
--Analyzing Semantics...
Error: at position Position(sline: 4, scolumn: 5, eline: 4, ecolumn: 16)
Error: Left-hand side of assignment is not an lvalue
Parsing extratest10
------Parsing fmj source file: extratest10.fmj------------
Convert AST  to XML...
Writing AST to file: extratest10.2.ast
Checking extratest10
Read AST from file: extratest10.2.ast
Converting XML to AST...
Semantic analysis...
--Making Name Maps...
--Analyzing Semantics...
Error: at position Position(sline: 5, scolumn: 5, eline: 9, ecolumn: 5)
Error: If condition must be of integer type
Parsing extratest11
------Parsing fmj source file: extratest11.fmj------------
Convert AST  to XML...
Writing AST to file: extratest11.2.ast
Checking extratest11
Read AST from file: extratest11.2.ast
Converting XML to AST...
Semantic analysis...
--Making Name Maps...
--Analyzing Semantics...
Error: at position Position(sline: 4, scolumn: 5, eline: 6, ecolumn: 5)
Error: While condition must be of integer type
Parsing extratest12
------Parsing fmj source file: extratest12.fmj------------
Convert AST  to XML...
Writing AST to file: extratest12.2.ast
Checking extratest12
Read AST from file: extratest12.2.ast
Converting XML to AST...
Semantic analysis...
--Making Name Maps...
--Analyzing Semantics...
Error: at position Position(sline: 3, scolumn: 12, eline: 3, ecolumn: 15)
Error: 'this' used outside of a class method
Parsing extratest13
------Parsing fmj source file: extratest13.fmj------------
Convert AST  to XML...
Writing AST to file: extratest13.2.ast
Checking extratest13
Read AST from file: extratest13.2.ast
Converting XML to AST...
Semantic analysis...
--Making Name Maps...
--Analyzing Semantics...
Error: at position Position(sline: 5, scolumn: 9, eline: 5, ecolumn: 18)
Error: new int[] size must be int
Parsing extratest14
------Parsing fmj source file: extratest14.fmj------------
Convert AST  to XML...
Writing AST to file: extratest14.2.ast
Checking extratest14
Read AST from file: extratest14.2.ast
Converting XML to AST...
Semantic analysis...
--Making Name Maps...
--Analyzing Semantics...
Error: at position Position(sline: 6, scolumn: 9, eline: 6, ecolumn: 22)
Error: Method add expects 2 arguments but got 3
Parsing extratest15
------Parsing fmj source file: extratest15.fmj------------
Convert AST  to XML...
Writing AST to file: extratest15.2.ast
Checking extratest15
Read AST from file: extratest15.2.ast
Converting XML to AST...
Semantic analysis...
--Making Name Maps...
--Analyzing Semantics...
Error: at position Position(sline: 3, scolumn: 5, eline: 3, ecolumn: 10)
Error: break statement outside of a while loop
Parsing extratest2
------Parsing fmj source file: extratest2.fmj------------
Convert AST  to XML...
Writing AST to file: extratest2.2.ast
Checking extratest2
Read AST from file: extratest2.2.ast
Converting XML to AST...
Semantic analysis...
--Making Name Maps...
--Analyzing Semantics...
Error: at position Position(sline: 5, scolumn: 5, eline: 5, ecolumn: 10)
Error: Assign node has a different type between left and right
Parsing extratest3
------Parsing fmj source file: extratest3.fmj------------
Convert AST  to XML...
Writing AST to file: extratest3.2.ast
Checking extratest3
Read AST from file: extratest3.2.ast
Converting XML to AST...
Semantic analysis...
--Making Name Maps...
--Analyzing Semantics...
Error: at position Position(sline: 9, scolumn: 9, eline: 9, ecolumn: 17)
Error: Return type mismatch in method Foo.bar
Parsing extratest4
------Parsing fmj source file: extratest4.fmj------------
Convert AST  to XML...
Writing AST to file: extratest4.2.ast
Checking extratest4
Read AST from file: extratest4.2.ast
Converting XML to AST...
Semantic analysis...
--Making Name Maps...
--Analyzing Semantics...
Error: at position Position(sline: 5, scolumn: 9, eline: 5, ecolumn: 13)
Error: Left operand of '+' must be int
Parsing extratest5
------Parsing fmj source file: extratest5.fmj------------
Convert AST  to XML...
Writing AST to file: extratest5.2.ast
Checking extratest5
Read AST from file: extratest5.2.ast
Converting XML to AST...
Semantic analysis...
--Making Name Maps...
--Analyzing Semantics...
Error: at position Position(sline: 5, scolumn: 9, eline: 5, ecolumn: 10)
Error: Operand of unary '!' must be int
Parsing extratest6
------Parsing fmj source file: extratest6.fmj------------
Convert AST  to XML...
Writing AST to file: extratest6.2.ast
Checking extratest6
Read AST from file: extratest6.2.ast
Converting XML to AST...
Semantic analysis...
--Making Name Maps...
--Analyzing Semantics...
Error: at position Position(sline: 5, scolumn: 5, eline: 5, ecolumn: 14)
Error: putint() argument must be int
Parsing extratest7
------Parsing fmj source file: extratest7.fmj------------
Convert AST  to XML...
Writing AST to file: extratest7.2.ast
Checking extratest7
Read AST from file: extratest7.2.ast
Converting XML to AST...
Semantic analysis...
--Making Name Maps...
--Analyzing Semantics...
Error: at position Position(sline: 6, scolumn: 9, eline: 6, ecolumn: 17)
Error: length() argument must be an array
Parsing extratest8
------Parsing fmj source file: extratest8.fmj------------
Convert AST  to XML...
Writing AST to file: extratest8.2.ast
Checking extratest8
Read AST from file: extratest8.2.ast
Converting XML to AST...
Semantic analysis...
--Making Name Maps...
--Analyzing Semantics...
Convert AST to XML with Semantic Info...
Parsing extratest9
------Parsing fmj source file: extratest9.fmj------------
Convert AST  to XML...
Writing AST to file: extratest9.2.ast
Checking extratest9
Read AST from file: extratest9.2.ast
Converting XML to AST...
Semantic analysis...
--Making Name Maps...
--Analyzing Semantics...
Error: at position Position(sline: 6, scolumn: 5, eline: 6, ecolumn: 10)
Error: Assign node has a different type between left and right
Parsing fibonacci
------Parsing fmj source file: fibonacci.fmj------------
Error at lines [4:11-4:11]: syntax error, unexpected NONNEGATIVEINT, expecting '{'
Error: parsing failed
AST is not valid!
Checking fibonacci
Read AST from file: fibonacci.2.ast
Converting XML to AST...
Semantic analysis...
--Making Name Maps...
--Analyzing Semantics...
Convert AST to XML with Semantic Info...
Parsing newtest1
------Parsing fmj source file: newtest1.fmj------------
Convert AST  to XML...
Writing AST to file: newtest1.2.ast
Checking newtest1
Read AST from file: newtest1.2.ast
Converting XML to AST...
Semantic analysis...
--Making Name Maps...
--Analyzing Semantics...
Error: at position Position(sline: 5, scolumn: 5, eline: 5, ecolumn: 20)
Error: Assign node has a different type between left and right
Parsing newtest10
------Parsing fmj source file: newtest10.fmj------------
Convert AST  to XML...
Writing AST to file: newtest10.2.ast
Checking newtest10
Read AST from file: newtest10.2.ast
Converting XML to AST...
Semantic analysis...
--Making Name Maps...
--Analyzing Semantics...
Convert AST to XML with Semantic Info...
Parsing newtest11
------Parsing fmj source file: newtest11.fmj------------
Convert AST  to XML...
Writing AST to file: newtest11.2.ast
Checking newtest11
Read AST from file: newtest11.2.ast
Converting XML to AST...
Semantic analysis...
--Making Name Maps...
--Analyzing Semantics...
Convert AST to XML with Semantic Info...
Parsing newtest12
------Parsing fmj source file: newtest12.fmj------------
Convert AST  to XML...
Writing AST to file: newtest12.2.ast
Checking newtest12
Read AST from file: newtest12.2.ast
Converting XML to AST...
Semantic analysis...
--Making Name Maps...
--Analyzing Semantics...
Convert AST to XML with Semantic Info...
Parsing newtest13
------Parsing fmj source file: newtest13.fmj------------
Convert AST  to XML...
Writing AST to file: newtest13.2.ast
Checking newtest13
Read AST from file: newtest13.2.ast
Converting XML to AST...
Semantic analysis...
--Making Name Maps...
--Analyzing Semantics...
Convert AST to XML with Semantic Info...
Parsing newtest14
------Parsing fmj source file: newtest14.fmj------------
Convert AST  to XML...
Writing AST to file: newtest14.2.ast
Checking newtest14
Read AST from file: newtest14.2.ast
Converting XML to AST...
Semantic analysis...
--Making Name Maps...
--Analyzing Semantics...
Error: at position Position(sline: 5, scolumn: 5, eline: 5, ecolumn: 10)
Error: ArrayExp node has a non-array value expression
Parsing newtest15
------Parsing fmj source file: newtest15.fmj------------
Convert AST  to XML...
Writing AST to file: newtest15.2.ast
Checking newtest15
Read AST from file: newtest15.2.ast
Converting XML to AST...
Semantic analysis...
--Making Name Maps...
--Analyzing Semantics...
Error: at position Position(sline: 6, scolumn: 5, eline: 6, ecolumn: 12)
Error: Assign node has a different type between left and right
Parsing newtest16
------Parsing fmj source file: newtest16.fmj------------
Convert AST  to XML...
Writing AST to file: newtest16.2.ast
Checking newtest16
Read AST from file: newtest16.2.ast
Converting XML to AST...
Semantic analysis...
--Making Name Maps...
--Analyzing Semantics...
Convert AST to XML with Semantic Info...
Parsing newtest17
------Parsing fmj source file: newtest17.fmj------------
Convert AST  to XML...
Writing AST to file: newtest17.2.ast
Checking newtest17
Read AST from file: newtest17.2.ast
Converting XML to AST...
Semantic analysis...
--Making Name Maps...
--Analyzing Semantics...
Convert AST to XML with Semantic Info...
Parsing newtest18
------Parsing fmj source file: newtest18.fmj------------
Convert AST  to XML...
Writing AST to file: newtest18.2.ast
Checking newtest18
Read AST from file: newtest18.2.ast
Converting XML to AST...
Semantic analysis...
--Making Name Maps...
--Analyzing Semantics...
Error: at position Position(sline: 6, scolumn: 5, eline: 6, ecolumn: 12)
Error: Assign node has a different type between left and right
Parsing newtest2
------Parsing fmj source file: newtest2.fmj------------
Convert AST  to XML...
Writing AST to file: newtest2.2.ast
Checking newtest2
Read AST from file: newtest2.2.ast
Converting XML to AST...
Semantic analysis...
--Making Name Maps...
Error: at position Position(sline: 3, scolumn: 5, eline: 3, ecolumn: 22)
Error: Duplicate method variable: a in method __main__.main
--Analyzing Semantics...
Error: at position Position(sline: 3, scolumn: 5, eline: 3, ecolumn: 22)
Error: Duplicate method variable: a in method __main__.main
Error: at position Position(sline: 7, scolumn: 5, eline: 7, ecolumn: 13)
Error: Return type mismatch in method __main__.main
Parsing newtest3
------Parsing fmj source file: newtest3.fmj------------
Convert AST  to XML...
Writing AST to file: newtest3.2.ast
Checking newtest3
Read AST from file: newtest3.2.ast
Converting XML to AST...
Semantic analysis...
--Making Name Maps...
--Analyzing Semantics...
Convert AST to XML with Semantic Info...
Parsing newtest4
------Parsing fmj source file: newtest4.fmj------------
Convert AST  to XML...
Writing AST to file: newtest4.2.ast
Checking newtest4
Read AST from file: newtest4.2.ast
Converting XML to AST...
Semantic analysis...
--Making Name Maps...
--Analyzing Semantics...
Error: at position Position(sline: 16, scolumn: 9, eline: 16, ecolumn: 20)
Error: While condition must be of integer type
Parsing newtest5
------Parsing fmj source file: newtest5.fmj------------
Convert AST  to XML...
Writing AST to file: newtest5.2.ast
Checking newtest5
Read AST from file: newtest5.2.ast
Converting XML to AST...
Semantic analysis...
--Making Name Maps...
--Analyzing Semantics...
Error: at position Position(sline: 21, scolumn: 9, eline: 21, ecolumn: 25)
Error: If condition must be of integer type
Parsing newtest6
------Parsing fmj source file: newtest6.fmj------------
Convert AST  to XML...
Writing AST to file: newtest6.2.ast
Checking newtest6
Read AST from file: newtest6.2.ast
Converting XML to AST...
Semantic analysis...
--Making Name Maps...
--Analyzing Semantics...
Convert AST to XML with Semantic Info...
Parsing newtest7
------Parsing fmj source file: newtest7.fmj------------
Convert AST  to XML...
Writing AST to file: newtest7.2.ast
Checking newtest7
Read AST from file: newtest7.2.ast
Converting XML to AST...
Semantic analysis...
--Making Name Maps...
--Analyzing Semantics...
Error: at position Position(sline: 28, scolumn: 2, eline: 28, ecolumn: 13)
Error: Return type mismatch in method TestClass1.test0
Parsing newtest8
------Parsing fmj source file: newtest8.fmj------------
Convert AST  to XML...
Writing AST to file: newtest8.2.ast
Checking newtest8
Read AST from file: newtest8.2.ast
Converting XML to AST...
Semantic analysis...
--Making Name Maps...
--Analyzing Semantics...
Error: at position Position(sline: 13, scolumn: 1, eline: 13, ecolumn: 46)
Error: Multi-level inheritance not allowed. TestClass2 extends TestClass1 which extends TestClass
Parsing newtest9
------Parsing fmj source file: newtest9.fmj------------
Convert AST  to XML...
Writing AST to file: newtest9.2.ast
Checking newtest9
Read AST from file: newtest9.2.ast
Converting XML to AST...
Semantic analysis...
--Making Name Maps...
--Analyzing Semantics...
Error: at position Position(sline: 7, scolumn: 4, eline: 7, ecolumn: 12)
Error: continue statement outside of a while loop
Parsing test1
------Parsing fmj source file: test1.fmj------------
Error at lines [2:10-2:10]: syntax error, unexpected NONNEGATIVEINT, expecting '{'
Error: parsing failed
AST is not valid!
Checking test1
Read AST from file: test1.2.ast
Converting XML to AST...
Semantic analysis...
--Making Name Maps...
Error: at position Position(sline: 3, scolumn: 2, eline: 3, ecolumn: 31)
Error: Duplicate method variable: x in method __main__.main
Error: at position Position(sline: 18, scolumn: 42, eline: 18, ecolumn: 42)
Error: Duplicate formal parameter: x in method B.m1
--Analyzing Semantics...
Error: at position Position(sline: 3, scolumn: 2, eline: 3, ecolumn: 31)
Error: Duplicate method variable: x in method __main__.main
Error: at position Position(sline: 18, scolumn: 42, eline: 18, ecolumn: 42)
Error: Duplicate formal parameter: x in method B.m1
Error: at position Position(sline: 24, scolumn: 2, eline: 29, ecolumn: 2)
Error: Method override return type not covariant: A.m
Parsing test2
------Parsing fmj source file: test2.fmj------------
Convert AST  to XML...
Writing AST to file: test2.2.ast
Checking test2
Read AST from file: test2.2.ast
Converting XML to AST...
Semantic analysis...
--Making Name Maps...
--Analyzing Semantics...
Convert AST to XML with Semantic Info...
Parsing test_class
------Parsing fmj source file: test_class.fmj------------
Error at lines [29:16-29:16]: syntax error, unexpected NONNEGATIVEINT, expecting '{'
Error: parsing failed
AST is not valid!
Checking test_class
Read AST from file: test_class.2.ast
Converting XML to AST...
Semantic analysis...
--Making Name Maps...
--Analyzing Semantics...
Convert AST to XML with Semantic Info...
Parsing test_comprehensive
------Parsing fmj source file: test_comprehensive.fmj------------
Error at lines [36:15-36:17]: syntax error, unexpected NONNEGATIVEINT, expecting '{'
Error: parsing failed
AST is not valid!
Checking test_comprehensive
Read AST from file: test_comprehensive.2.ast
Converting XML to AST...
Semantic analysis...
--Making Name Maps...
--Analyzing Semantics...
Convert AST to XML with Semantic Info...
Parsing test_err_assign
------Parsing fmj source file: test_err_assign.fmj------------
Convert AST  to XML...
Writing AST to file: test_err_assign.2.ast
Checking test_err_assign
Read AST from file: test_err_assign.2.ast
Converting XML to AST...
Semantic analysis...
--Making Name Maps...
--Analyzing Semantics...
Error: at position Position(sline: 4, scolumn: 5, eline: 4, ecolumn: 12)
Error: Assign node has a different type between left and right
Parsing test_err_break
------Parsing fmj source file: test_err_break.fmj------------
Convert AST  to XML...
Writing AST to file: test_err_break.2.ast
Checking test_err_break
Read AST from file: test_err_break.2.ast
Converting XML to AST...
Semantic analysis...
--Making Name Maps...
--Analyzing Semantics...
Error: at position Position(sline: 4, scolumn: 5, eline: 4, ecolumn: 10)
Error: break statement outside of a while loop
Parsing test_forward
------Parsing fmj source file: test_forward.fmj------------
Error at lines [16:13-16:13]: syntax error, unexpected NONNEGATIVEINT, expecting '{'
Error: parsing failed
AST is not valid!
Checking test_forward
Read AST from file: test_forward.2.ast
Converting XML to AST...
Semantic analysis...
--Making Name Maps...
--Analyzing Semantics...
Convert AST to XML with Semantic Info...
Parsing test_inherit
------Parsing fmj source file: test_inherit.fmj------------
Error at lines [26:17-26:17]: syntax error, unexpected NONNEGATIVEINT, expecting '{'
Error: parsing failed
AST is not valid!
Checking test_inherit
Read AST from file: test_inherit.2.ast
Converting XML to AST...
Semantic analysis...
--Making Name Maps...
--Analyzing Semantics...
Convert AST to XML with Semantic Info...
Parsing test_io
------Parsing fmj source file: test_io.fmj------------
Error at lines [4:13-4:13]: syntax error, unexpected NONNEGATIVEINT, expecting '{'
Error: parsing failed
AST is not valid!
Checking test_io
Read AST from file: test_io.2.ast
Converting XML to AST...
Semantic analysis...
--Making Name Maps...
--Analyzing Semantics...
Convert AST to XML with Semantic Info...
```

新增的测试用例示例：

```java
/* 测试: Assign 左侧非 lvalue (函数调用结果赋值) */
public int main() {
    class Foo f;
    f.bar() = 1;
    return 0;
}

public class Foo {
    int x;
    public int bar() {
        return x;
    }
}

/*
Expected error: Left-hand side of assignment is not an lvalue
*/
```

semant.ast示例：(bubblesort)

```xml
<?xml version="1.0" encoding="UTF-8"?>
<Program bline="1" bpos="1" eline="37" epos="1">
    <MainMethod bline="1" bpos="1" eline="15" epos="1">
        <VarDeclList>
            <VarDecl bline="2" bpos="3" eline="2" epos="12">
                <Type typeKind="INT" bline="2" bpos="3" eline="2" epos="5"/>
                <IdExp id="i" bline="2" bpos="7" eline="2" epos="7"/>
            </VarDecl>
            <VarDecl bline="3" bpos="3" eline="3" epos="34">
                <Type typeKind="ARRAY" bline="3" bpos="3" eline="3" epos="7">
                    <Arity val="0" bline="3" bpos="6" eline="3" epos="6"/>
                </Type>
                <IdExp id="a" bline="3" bpos="9" eline="3" epos="9"/>
                <IntInitList>
                    <IntExp val="6" bline="3" bpos="14" eline="3" epos="14"/>
                    <IntExp val="3" bline="3" bpos="17" eline="3" epos="17"/>
                    <IntExp val="0" bline="3" bpos="20" eline="3" epos="20"/>
                    <IntExp val="5" bline="3" bpos="23" eline="3" epos="23"/>
                    <IntExp val="9" bline="3" bpos="26" eline="3" epos="26"/>
                    <IntExp val="1" bline="3" bpos="29" eline="3" epos="29"/>
                    <IntExp val="2" bline="3" bpos="32" eline="3" epos="32"/>
                </IntInitList>
            </VarDecl>
            <VarDecl bline="4" bpos="3" eline="4" epos="14">
                <Type typeKind="CLASS" bline="4" bpos="3" eline="4" epos="10">
                    <IdExp id="b1" bline="4" bpos="9" eline="4" epos="10"/>
                </Type>
                <IdExp id="bo" bline="4" bpos="12" eline="4" epos="13"/>
            </VarDecl>
        </VarDeclList>
        <StmList>
            <Assign bline="6" bpos="3" eline="6" epos="16">
                <IdExp bline="6" bpos="3" eline="6" epos="4" s_kind="Value" typeKind="Class" lvalue="true" cid="b1" id="bo"/>
                <NewObject bline="6" bpos="8" eline="6" epos="15" s_kind="Value" typeKind="Class" lvalue="false" cid="b1">
                    <Id id="b1"/>
                </NewObject>
            </Assign>
            <CallStm bline="7" bpos="3" eline="7" epos="30">
                <IdExp bline="7" bpos="3" eline="7" epos="4" s_kind="Value" typeKind="Class" lvalue="true" cid="b1" id="bo"/>
                <IdExp id="bubbleSort" bline="7" bpos="6" eline="7" epos="15" s_kind="MethodName"/>
                <ParList>
                    <IdExp bline="7" bpos="17" eline="7" epos="17" s_kind="Value" typeKind="IntArray" lvalue="true" arity="0" id="a"/>
                    <Length bline="7" bpos="20" eline="7" epos="28" s_kind="Value" typeKind="Int" lvalue="false">
                        <IdExp bline="7" bpos="27" eline="7" epos="27" s_kind="Value" typeKind="IntArray" lvalue="true" arity="0" id="a"/>
                    </Length>
                </ParList>
            </CallStm>
            <While bline="8" bpos="3" eline="12" epos="3">
                <BinaryOp bline="8" bpos="10" eline="8" epos="22" s_kind="Value" typeKind="Int" lvalue="false">
                    <IdExp bline="8" bpos="10" eline="8" epos="10" s_kind="Value" typeKind="Int" lvalue="true" id="i"/>
                    <OpExp op="&lt;" bline="8" bpos="12" eline="8" epos="12"/>
                    <Length bline="8" bpos="14" eline="8" epos="22" s_kind="Value" typeKind="Int" lvalue="false">
                        <IdExp bline="8" bpos="21" eline="8" epos="21" s_kind="Value" typeKind="IntArray" lvalue="true" arity="0" id="a"/>
                    </Length>
                </BinaryOp>
                <Nested bline="8" bpos="25" eline="12" epos="3">
                    <StmList>
                        <PutInt bline="9" bpos="5" eline="9" epos="17">
                            <ArrayExp bline="9" bpos="12" eline="9" epos="15" s_kind="Value" typeKind="Int" lvalue="true">
                                <IdExp bline="9" bpos="12" eline="9" epos="12" s_kind="Value" typeKind="IntArray" lvalue="true" arity="0" id="a"/>
                                <IdExp bline="9" bpos="14" eline="9" epos="14" s_kind="Value" typeKind="Int" lvalue="true" id="i"/>
                            </ArrayExp>
                        </PutInt>
                        <PutCh bline="10" bpos="5" eline="10" epos="14">
                            <IntExp bline="10" bpos="11" eline="10" epos="12" s_kind="Value" typeKind="Int" lvalue="false" val="32"/>
                        </PutCh>
                        <Assign bline="11" bpos="5" eline="11" epos="14">
                            <IdExp bline="11" bpos="5" eline="11" epos="5" s_kind="Value" typeKind="Int" lvalue="true" id="i"/>
                            <BinaryOp bline="11" bpos="9" eline="11" epos="13" s_kind="Value" typeKind="Int" lvalue="false">
                                <IdExp bline="11" bpos="9" eline="11" epos="9" s_kind="Value" typeKind="Int" lvalue="true" id="i"/>
                                <OpExp op="+" bline="11" bpos="11" eline="11" epos="11"/>
                                <IntExp bline="11" bpos="13" eline="11" epos="13" s_kind="Value" typeKind="Int" lvalue="false" val="1"/>
                            </BinaryOp>
                        </Assign>
                    </StmList>
                </Nested>
            </While>
            <PutCh bline="13" bpos="3" eline="13" epos="12">
                <IntExp bline="13" bpos="9" eline="13" epos="10" s_kind="Value" typeKind="Int" lvalue="false" val="10"/>
            </PutCh>
            <Return bline="14" bpos="3" eline="14" epos="11" s_kind="Value" typeKind="Int" lvalue="false">
                <IntExp bline="14" bpos="10" eline="14" epos="10" s_kind="Value" typeKind="Int" lvalue="false" val="0"/>
            </Return>
        </StmList>
    </MainMethod>
    <ClassDeclList>
        <ClassDecl bline="17" bpos="1" eline="37" epos="1">
            <IdExp id="b1" bline="17" bpos="14" eline="17" epos="15"/>
            <VarDeclList>
                <VarDecl bline="18" bpos="3" eline="18" epos="11">
                    <Type typeKind="INT" bline="18" bpos="3" eline="18" epos="5"/>
                    <IdExp id="temp" bline="18" bpos="7" eline="18" epos="10"/>
                </VarDecl>
            </VarDeclList>
            <MethodDeclList>
                <MethodDecl bline="19" bpos="3" eline="36" epos="3">
                    <Type typeKind="INT" bline="19" bpos="10" eline="19" epos="12"/>
                    <IdExp id="bubbleSort" bline="19" bpos="14" eline="19" epos="23"/>
                    <FormalList>
                        <Formal bline="19" bpos="25" eline="19" epos="35">
                            <Type typeKind="ARRAY" bline="19" bpos="25" eline="19" epos="29">
                                <Arity val="0" bline="19" bpos="28" eline="19" epos="28"/>
                            </Type>
                            <IdExp bline="19" bpos="31" eline="19" epos="35" id="array"/>
                        </Formal>
                        <Formal bline="19" bpos="42" eline="19" epos="45">
                            <Type typeKind="INT" bline="19" bpos="38" eline="19" epos="40"/>
                            <IdExp bline="19" bpos="42" eline="19" epos="45" id="size"/>
                        </Formal>
                    </FormalList>
                    <VarDeclList>
                        <VarDecl bline="20" bpos="5" eline="20" epos="14">
                            <Type typeKind="INT" bline="20" bpos="5" eline="20" epos="7"/>
                            <IdExp id="i" bline="20" bpos="9" eline="20" epos="9"/>
                        </VarDecl>
                    </VarDeclList>
                    <StmList>
                        <If bline="22" bpos="5" eline="24" epos="5">
                            <BinaryOp bline="22" bpos="9" eline="22" epos="17" s_kind="Value" typeKind="Int" lvalue="false">
                                <IdExp bline="22" bpos="9" eline="22" epos="12" s_kind="Value" typeKind="Int" lvalue="true" id="size"/>
                                <OpExp op="&lt;=" bline="22" bpos="14" eline="22" epos="15"/>
                                <IntExp bline="22" bpos="17" eline="22" epos="17" s_kind="Value" typeKind="Int" lvalue="false" val="1"/>
                            </BinaryOp>
                            <Nested bline="22" bpos="20" eline="24" epos="5">
                                <StmList>
                                    <Return bline="23" bpos="7" eline="23" epos="15" s_kind="Value" typeKind="Int" lvalue="false">
                                        <IntExp bline="23" bpos="14" eline="23" epos="14" s_kind="Value" typeKind="Int" lvalue="false" val="0"/>
                                    </Return>
                                </StmList>
                            </Nested>
                        </If>
                        <While bline="26" bpos="5" eline="33" epos="5">
                            <BinaryOp bline="26" bpos="12" eline="26" epos="23" s_kind="Value" typeKind="Int" lvalue="false">
                                <IdExp bline="26" bpos="12" eline="26" epos="12" s_kind="Value" typeKind="Int" lvalue="true" id="i"/>
                                <OpExp op="&lt;" bline="26" bpos="14" eline="26" epos="14"/>
                                <BinaryOp bline="26" bpos="16" eline="26" epos="23" s_kind="Value" typeKind="Int" lvalue="false">
                                    <IdExp bline="26" bpos="16" eline="26" epos="19" s_kind="Value" typeKind="Int" lvalue="true" id="size"/>
                                    <OpExp op="-" bline="26" bpos="21" eline="26" epos="21"/>
                                    <IntExp bline="26" bpos="23" eline="26" epos="23" s_kind="Value" typeKind="Int" lvalue="false" val="1"/>
                                </BinaryOp>
                            </BinaryOp>
                            <Nested bline="26" bpos="26" eline="33" epos="5">
                                <StmList>
                                    <If bline="27" bpos="7" eline="31" epos="7">
                                        <BinaryOp bline="27" bpos="11" eline="27" epos="33" s_kind="Value" typeKind="Int" lvalue="false">
                                            <ArrayExp bline="27" bpos="11" eline="27" epos="18" s_kind="Value" typeKind="Int" lvalue="true">
                                                <IdExp bline="27" bpos="11" eline="27" epos="15" s_kind="Value" typeKind="IntArray" lvalue="true" arity="0" id="array"/>
                                                <IdExp bline="27" bpos="17" eline="27" epos="17" s_kind="Value" typeKind="Int" lvalue="true" id="i"/>
                                            </ArrayExp>
                                            <OpExp op="&gt;" bline="27" bpos="20" eline="27" epos="20"/>
                                            <ArrayExp bline="27" bpos="22" eline="27" epos="33" s_kind="Value" typeKind="Int" lvalue="true">
                                                <IdExp bline="27" bpos="22" eline="27" epos="26" s_kind="Value" typeKind="IntArray" lvalue="true" arity="0" id="array"/>
                                                <BinaryOp bline="27" bpos="28" eline="27" epos="32" s_kind="Value" typeKind="Int" lvalue="false">
                                                    <IdExp bline="27" bpos="28" eline="27" epos="28" s_kind="Value" typeKind="Int" lvalue="true" id="i"/>
                                                    <OpExp op="+" bline="27" bpos="30" eline="27" epos="30"/>
                                                    <IntExp bline="27" bpos="32" eline="27" epos="32" s_kind="Value" typeKind="Int" lvalue="false" val="1"/>
                                                </BinaryOp>
                                            </ArrayExp>
                                        </BinaryOp>
                                        <Nested bline="27" bpos="36" eline="31" epos="7">
                                            <StmList>
                                                <Assign bline="28" bpos="9" eline="28" epos="24">
                                                    <IdExp bline="28" bpos="9" eline="28" epos="12" s_kind="Value" typeKind="Int" lvalue="true" id="temp"/>
                                                    <ArrayExp bline="28" bpos="16" eline="28" epos="23" s_kind="Value" typeKind="Int" lvalue="true">
                                                        <IdExp bline="28" bpos="16" eline="28" epos="20" s_kind="Value" typeKind="IntArray" lvalue="true" arity="0" id="array"/>
                                                        <IdExp bline="28" bpos="22" eline="28" epos="22" s_kind="Value" typeKind="Int" lvalue="true" id="i"/>
                                                    </ArrayExp>
                                                </Assign>
                                                <Assign bline="29" bpos="9" eline="29" epos="32">
                                                    <ArrayExp bline="29" bpos="9" eline="29" epos="16" s_kind="Value" typeKind="Int" lvalue="true">
                                                        <IdExp bline="29" bpos="9" eline="29" epos="13" s_kind="Value" typeKind="IntArray" lvalue="true" arity="0" id="array"/>
                                                        <IdExp bline="29" bpos="15" eline="29" epos="15" s_kind="Value" typeKind="Int" lvalue="true" id="i"/>
                                                    </ArrayExp>
                                                    <ArrayExp bline="29" bpos="20" eline="29" epos="31" s_kind="Value" typeKind="Int" lvalue="true">
                                                        <IdExp bline="29" bpos="20" eline="29" epos="24" s_kind="Value" typeKind="IntArray" lvalue="true" arity="0" id="array"/>
                                                        <BinaryOp bline="29" bpos="26" eline="29" epos="30" s_kind="Value" typeKind="Int" lvalue="false">
                                                            <IdExp bline="29" bpos="26" eline="29" epos="26" s_kind="Value" typeKind="Int" lvalue="true" id="i"/>
                                                            <OpExp op="+" bline="29" bpos="28" eline="29" epos="28"/>
                                                            <IntExp bline="29" bpos="30" eline="29" epos="30" s_kind="Value" typeKind="Int" lvalue="false" val="1"/>
                                                        </BinaryOp>
                                                    </ArrayExp>
                                                </Assign>
                                                <Assign bline="30" bpos="9" eline="30" epos="28">
                                                    <ArrayExp bline="30" bpos="9" eline="30" epos="20" s_kind="Value" typeKind="Int" lvalue="true">
                                                        <IdExp bline="30" bpos="9" eline="30" epos="13" s_kind="Value" typeKind="IntArray" lvalue="true" arity="0" id="array"/>
                                                        <BinaryOp bline="30" bpos="15" eline="30" epos="19" s_kind="Value" typeKind="Int" lvalue="false">
                                                            <IdExp bline="30" bpos="15" eline="30" epos="15" s_kind="Value" typeKind="Int" lvalue="true" id="i"/>
                                                            <OpExp op="+" bline="30" bpos="17" eline="30" epos="17"/>
                                                            <IntExp bline="30" bpos="19" eline="30" epos="19" s_kind="Value" typeKind="Int" lvalue="false" val="1"/>
                                                        </BinaryOp>
                                                    </ArrayExp>
                                                    <IdExp bline="30" bpos="24" eline="30" epos="27" s_kind="Value" typeKind="Int" lvalue="true" id="temp"/>
                                                </Assign>
                                            </StmList>
                                        </Nested>
                                    </If>
                                    <Assign bline="32" bpos="7" eline="32" epos="16">
                                        <IdExp bline="32" bpos="7" eline="32" epos="7" s_kind="Value" typeKind="Int" lvalue="true" id="i"/>
                                        <BinaryOp bline="32" bpos="11" eline="32" epos="15" s_kind="Value" typeKind="Int" lvalue="false">
                                            <IdExp bline="32" bpos="11" eline="32" epos="11" s_kind="Value" typeKind="Int" lvalue="true" id="i"/>
                                            <OpExp op="+" bline="32" bpos="13" eline="32" epos="13"/>
                                            <IntExp bline="32" bpos="15" eline="32" epos="15" s_kind="Value" typeKind="Int" lvalue="false" val="1"/>
                                        </BinaryOp>
                                    </Assign>
                                </StmList>
                            </Nested>
                        </While>
                        <Return bline="35" bpos="5" eline="35" epos="44" s_kind="Value" typeKind="Int" lvalue="false">
                            <CallExp bline="35" bpos="12" eline="35" epos="43" s_kind="Value" typeKind="Int" lvalue="false">
                                <This bline="35" bpos="12" eline="35" epos="15" s_kind="Value" typeKind="Class" lvalue="false" cid="b1"/>
                                <IdExp id="bubbleSort" bline="35" bpos="17" eline="35" epos="26" s_kind="MethodName"/>
                                <ParList>
                                    <IdExp bline="35" bpos="28" eline="35" epos="32" s_kind="Value" typeKind="IntArray" lvalue="true" arity="0" id="array"/>
                                    <BinaryOp bline="35" bpos="35" eline="35" epos="42" s_kind="Value" typeKind="Int" lvalue="false">
                                        <IdExp bline="35" bpos="35" eline="35" epos="38" s_kind="Value" typeKind="Int" lvalue="true" id="size"/>
                                        <OpExp op="-" bline="35" bpos="40" eline="35" epos="40"/>
                                        <IntExp bline="35" bpos="42" eline="35" epos="42" s_kind="Value" typeKind="Int" lvalue="false" val="1"/>
                                    </BinaryOp>
                                </ParList>
                            </CallExp>
                        </Return>
                    </StmList>
                </MethodDecl>
            </MethodDeclList>
        </ClassDecl>
    </ClassDeclList>
</Program>
```
