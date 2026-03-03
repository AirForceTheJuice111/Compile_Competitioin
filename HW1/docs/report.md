---
title: "HW1 实验报告"
author: "王思宇"
date: \today
using_title: true
using_table_of_content: true
---

# HW1 实验报告

## 参考材料

1. **虎书 (Modern Compiler Implementation in C/Java)**：理解 AST 节点结构与 Visitor Pattern 的设计思想。
2. **FDMJ-SLP 文法描述**（[FDMJSLPGrammar.md](../docs/FDMJSLPGrammar.md)）：定义了 FDMJ 语言的文法规则，包括终结符、非终结符和产生式。
3. **FDMJ AST Class Hierarchy**（[FDMJSLPClassHierarchy.md](../docs/FDMJSLPClassHierarchy.md)）：定义了 AST 节点的类层次结构和成员。
4. **Visitor Pattern 介绍**（[秒懂设计模式之访问者模式 - 知乎](https://zhuanlan.zhihu.com/p/380161731)）：理解如何在不修改 AST 数据结构的前提下新增遍历操作。
5. **MinusIntConverter 参考实现**（项目内 `lib/ast/minusIntConverter.cc`）：作为实现 ConstantPropagator 和 Executor 的模板，学习 Visitor 遍历 AST 并生成新节点的模式。

## 关键技术实现

### 1. 常量折叠（Constant Propagation）

**实现思路**：

常量折叠分两步进行：

1. **预处理**：调用 `minusIntRewrite(root)` 将所有 `UnaryOp(-, IntExp(n))` 形式的节点转换为 `IntExp(-n)`，消除一元取反操作中的常量。
2. **折叠**：使用 `ConstantPropagator` Visitor 自底向上遍历 AST，当发现 `BinaryOp` 的左右子节点都是 `IntExp` 时，直接计算结果并替换为新的 `IntExp` 节点。

**核心代码**（BinaryOp 的 visit 方法）：

```cpp
void ConstantPropagator::visit(BinaryOp *node) {
  // 自底向上：先递归处理左右子树
  node->left->accept(*this);
  Exp *l = static_cast<Exp *>(newNode);
  node->right->accept(*this);
  Exp *r = static_cast<Exp *>(newNode);

  // 核心逻辑：若左右都是 IntExp，直接计算并折叠
  if (l->getASTKind() == ASTKind::IntExp &&
      r->getASTKind() == ASTKind::IntExp) {
    int lval = static_cast<IntExp *>(l)->val;
    int rval = static_cast<IntExp *>(r)->val;
    // 根据 op 计算结果，生成新的 IntExp 节点
    newNode = new IntExp(node->getPos()->clone(), result);
    return;
  }
  // 无法折叠，保留原结构
  newNode = new BinaryOp(node->getPos()->clone(), l, node->op->clone(), r);
}
```

此外，`UnaryOp` 的 visit 也处理了经过子树折叠后新产生的 `UnaryOp(-, IntExp)` 情况，确保 `(-(3+4))` 这类表达式也能被完全折叠为 `IntExp(-7)`。

### 2. 程序执行器（Executor）

**实现思路**：

Executor 是一个解释执行的 Visitor，维护一个 `map<string, int>` 变量表。遍历过程中：

- **Assign**：求值右侧表达式，将结果存入变量表。
- **Return**：求值表达式，结果保存在 `result` 成员中。
- **BinaryOp**：递归求值左右操作数，根据运算符计算。
- **UnaryOp**：递归求值操作数，取反。
- **IdExp**：查变量表。若变量未定义，假设值为 0，并在 stderr 报告位置（行号和列号）。
- **IntExp**：直接返回整数值。

**未定义变量处理**：

```cpp
void Executor::visit(IdExp *node) {
  if (varDefined.find(node->id) == varDefined.end()) {
    Pos *p = node->getPos();
    cerr << "Warning: variable '" << node->id
         << "' used before definition at line " << p->sline
         << ", column " << p->scolumn << endl;
    result = 0;
    varTable[node->id] = 0;
    varDefined[node->id] = true;  // 标记已定义，避免重复报告
  } else {
    result = varTable[node->id];
  }
}
```

### 3. 额外测试用例

编写了 test5 ~ test12 共 8 个额外测试，覆盖以下场景：

| 测试 | 场景 | 预期结果 |
|------|------|---------|
| test5 | 未定义变量直接 return | 0（报 Warning） |
| test6 | 纯常量表达式 + 变量混合 | 18 |
| test7 | 0 值传播 | 0 |
| test8 | 多层嵌套取反 + 常量折叠 | -9 |
| test9 | 变量自更新 | 22 |
| test10 | 全常量折叠 + 变量求和 | -16 |
| test11 | 直接 return 常量 | 42 |
| test12 | 整除 | 3 |

## Git 提交记录

```
2c23363 HW1: add extra test cases (test5-test12) for edge coverage
1abc8ab HW1: implement constantPropagation and executor, pass all 4 tests
d066116 chore: fixed .gitignore, added .github
55136e8 Slight modification
30b4d72 HW1 published
71e0770 OK
27abdb3 Remove README.md
392043f Initialization
a8569e5 Initial commit
```

## 测试结果
