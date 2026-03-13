---
title: "HW2 实验报告"
author: "王思宇"
date: \today
using_title: true
using_table_of_content: true
---

# HW2 实验报告

## 实验概述

本次实验实现了 FDMJ2026 语言的语义分析器，包括两个核心组件：

1. **Name Map Visitor**（`setnamemaps.cc`）：遍历 AST，构建符号表，记录所有类、方法、变量的声明及其关系。
2. **Semantic Analyzer Visitor**（`semantanlyzer.cc`）：基于符号表，遍历 AST 进行全面的类型检查和语义验证。

## 参考资料

1. **虎书（Modern Compiler Implementation in C/Java/ML）**：第 4-5 章，语义分析与类型检查的理论基础，包括符号表设计和类型兼容性规则。
2. **FDMJ2026 语言规范**（`docs/FDMJ2026Specification.md`）：定义了语言的类型系统、继承规则、运算规则等。
3. **FDMJAST.hh 文档**（`docs/FDMJAST.hh.md`）：AST 节点类的结构定义。
4. **namemaps.hh 文档**（`docs/namemap.hh.md`）：符号表数据结构和 API 说明。
5. **semant.hh 文档**（`docs/semant.hh.md`）：语义信息结构和语义分析 Visitor 的接口说明。

## 关键技术实现

### 1. Name Map 构建（`setnamemaps.cc`）

**核心设计**：在 `visit(Program*)` 中先预注册所有类名，再依次遍历各类声明。这解决了 FDMJ2026 允许父类声明在子类之后的问题。

- `MainMethod` 被视为特殊的 `__main__` 类中的 `main` 方法。
- 每个方法的形参列表末尾追加一个 `__return__` 伪形参来存储返回类型，便于后续统一处理。
- 变量注册区分类级别和方法级别：根据 `current_visiting_method` 是否为空来判断当前变量属于类还是方法。

### 2. 语义分析（`semantanlyzer.cc`）

#### 类型检查清单

本程序实现了以下类型检查：

- **赋值检查**：左侧必须是 lvalue；两侧类型必须兼容（子类可赋给父类变量）
- **算术/比较运算符**（`+`, `-`, `*`, `/`, `<`, `>`, `<=`, `>=`, `==`, `!=`）：要求 INT 操作数
- **逻辑运算符**（`&&`, `||`）：要求 INT 操作数
- **一元运算符**（`-`, `!`）：要求 INT 操作数
- **数组下标访问**：基址必须是 ARRAY 类型，索引必须是 INT 类型
- **方法调用**：对象必须是 CLASS 类型，方法必须存在于类或其父类中，参数数量和类型必须匹配
- **return 语句**：返回类型与方法声明的返回类型兼容（支持协变返回类型）
- **putint / putch**：参数必须是 INT 类型
- **putarray**：第一个参数 INT，第二个参数 ARRAY
- **length()**：参数必须是 ARRAY 类型
- **new int[size]**：size 必须是 INT 类型
- **new ClassName()**：类必须存在
- **break / continue**：必须在 while 循环内
- **ClassVar 访问**：对象必须是 CLASS 类型，字段必须存在于类或父类中
- **this 关键字**：只能在类方法中使用，不能在 main 中使用
- **getarray**：参数必须是 ARRAY 类型
- **继承合法性**：单层继承（父类不能再有父类）、无循环继承
- **方法重写**：参数数量与类型必须完全匹配，返回类型支持协变
- **VarDecl 初始化**：int 初始化只能用于 INT 变量，数组初始化只能用于 ARRAY 变量
- **CLASS 类型引用**：声明中引用的类必须存在

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

### 3. 遇到的坑

1. **类变量访问**：最初在 `IdExp` 的名称解析中没有考虑类变量，导致 `bubblesort.fmj` 中 `temp`（一个类变量）无法解析。实际上虽然规范说类变量只能通过 `obj.id` 访问，但测试用例中类方法可以直接引用自己的类变量（相当于隐式 `this`）。

2. **前向引用**：父类可以声明在子类之后，需要在 `visit(Program*)` 中预先注册所有类名。

3. **返回类型存储**：方法的返回类型是作为形参列表的最后一个元素 `__return__` 存储的，需要在构建 formal list 时正确处理。

4. **ARRAY 类型 arity**：返回类型为 ARRAY 时，如果原始声明没有 arity，需要补上 `arity=0`，否则创建 Formal 时会报错。

## Git 提交记录

```
17406c9 test(hw2): add additional test cases for inheritance, IO, errors, and class hierarchy
5b8ee79 feat(hw2): implement name maps and semantic analyzer - basic type checking
b69912f make format fixed: hw2
5dc4dee make format: hw2
0daa9e5 chore: ignored .ast
```

## 测试结果
