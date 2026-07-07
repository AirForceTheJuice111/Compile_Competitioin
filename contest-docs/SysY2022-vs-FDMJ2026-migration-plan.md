# SysY2022 与 FDMJ2026 差异及编译器迁移清单

本文用于把当前 FDMJ 编译器迁移到 2026 编译系统设计赛 SysY2022 实现赛道。contest 分支已经把原 `final/` 工程迁移到仓库根目录。当前仓库根目录的比赛入口和迁移保留结构为：

- SysY 比赛入口：`tools/compiler/main.cc`，构建产物为 `build/compiler`。
- SysY 原生前端：`include/sysy/*`、`lib/sysy/lexer.cc`、`lib/sysy/parser.cc`、`lib/sysy/semantics.cc`。
- SysY 原生 lowering：`lib/sysy/lower_tree.cc`，面向迁移后的 Tree IR。
- 迁移复用的 IR/Quad/SSA/优化/ARM 后端：`include/ir`、`lib/ir`、`include/quad`、`lib/quad`、`lib/opt`、`lib/instr`、`lib/reg`。
- 旧 FDMJ 源文件：`include/ast`、`lib/ast`、`include/frontend`、`lib/frontend`、`tools/fmjcc`、`tools/fmjinterp`，默认不构建，仅在 `BUILD_LEGACY_FMJ=ON` 时作为迁移调试工具。

迁移不是改扩展名这么简单。FDMJ 是教学用的类 Java 语言；SysY2022 是接近 C 子集的过程式语言，并增加 `float`、多维数组、全局对象和运行时库 ABI。

## 0. 当前迁移状态

本分支当前已经完成 SysY2022 功能性迁移的默认 native 基线：

- 新增 `tools/compiler/main.cc`，构建产物为 `build/compiler`，支持竞赛常见调用 `compiler -S -o out.s in.sy`。
- `CMakeLists.txt` 的项目名已经改为 `SysY2022ContestCompiler`，并构建 `compiler` 入口。
- `vendor/libsysy/` 中加入官方 ARM 运行时 `libsysy_arm.a`、`sylib.c` 和 `sylib.h`。
- `test/` 已替换为官方 `functional.zip` 中的 SysY2022 测试，旧 `.fmj` 测试已从该目录移除。
- `make sysy-functional-regression` 会递归扫描 `test/` 中的 `.sy` 文件，编译、链接、qemu 运行，并与 `.out` 精确比较。
- 已验证 `make sysy-functional-regression` 结果为 `total=141 pass=141 compile_fail=0 link_fail=0 run_fail=0 wrong=0`，其中 140 个来自官方 functional/h_functional，1 个为本地 `putf` 规格补测。
- 已验证 `make compile` 结果为 `total=141 pass=141 compile_fail=0`。
- 新增 `make sysy-performance-regression`，可对官方性能 zip 按需解压并复用同一套编译运行比较流程；官方 `ARM-性能.zip` 全部 59 个用例通过，`ARM决赛性能用例.zip` 全部 60 个用例通过。
- 新增 `include/sysy/lexer.hh` 与 `lib/sysy/lexer.cc`，作为原生 SysY 前端的第一块基础设施；`compiler --dump-tokens file.sy` 可以用该 lexer 输出 token 流。
- 新增 `include/sysy/ast.hh`、`include/sysy/parser.hh` 与 `lib/sysy/parser.cc`，实现 SysY2022 递归下降 parser 和通用 AST 骨架；`compiler --dump-ast file.sy` 可输出解析树，`make sysy-parse-regression` 已验证 `test/` 中 153 个 `.sy` 文件全部 parse 通过。
- 新增 `include/sysy/semantics.hh` 与 `lib/sysy/semantics.cc`，实现基础语义检查骨架：全局/局部作用域、重定义、`main` 唯一性、未声明引用、const 赋值、`break`/`continue` 位置、`return` 基础约束、运行库函数签名和 `starttime`/`stoptime` 无参别名；`make sysy-semantic-regression` 已验证 141 个可运行样例通过、12 个本地 reject 样例按预期失败。
- 默认 `compiler -S -o out.s in.sy` 已将 SysY AST 接到迁移后的 Tree/Quad/SSA/ARM 后端；兼容保留 `--native-backend`，显式 `--gcc-bridge` 可作为调试 fallback。
- native 路径已通过官方 functional/h_functional 全部 140 个用例，包括 float、float 数组、float 参数/返回值和浮点运行时 I/O。
- native 路径已通过官方 `ARM-性能.zip` 全部 59 个用例，以及 `ARM决赛性能用例.zip` 全部 60 个用例。

默认比赛入口已经不再通过 ARM GCC bridge 生成汇编。`tree::Type`/`QuadType` 增加了 `FLOAT`，`float` 常量以 IEEE-754 raw bits 在整数寄存器和内存中传递，浮点算术/比较/转换通过 `__aeabi_*` helper 降低，`getfloat`/`putfloat` 等 hard-float 运行库调用用 VFP `vmov` 连接。`putf` 的字符串 literal 会生成 `.rodata` 标签，`%d/%c/%f` 参数按格式串检查并 lowered；其中 `%f` 按 C 可变参规则提升为 double，再按 ARM AAPCS core-register/stack 规则传给运行库。为了通过大型性能测试，native 后端还修复了跨调用 caller-saved 寄存器冲突、带副作用 `%` 表达式重复求值、十六进制整数字面量误判为 float，以及运行库数组 I/O 对多维数组实参的兼容规则。剩余非默认工作主要是优化调优。

## 1. 输入、输出和提交接口

| 项目 | FDMJ2026 | SysY2022 | 需要修改 |
| --- | --- | --- | --- |
| 源文件扩展名 | `.fmj` | `.sy` | 驱动、脚本、测试收集从 `.fmj` 改为 `.sy`，可临时兼容两者。 |
| 程序入口 | 固定 `public int main() { ... }` | 顶层 `int main()`，无参数且唯一 | parser 和语义检查改为顶层 `CompUnit`；删除 `MainMethod` 特殊类包装。 |
| 输出 main 返回值 | 课程 harness 会打印 FMJ return | SysY 运行时/评测按程序退出码和 stdout | 竞赛通常由评测器运行二进制，编译器不应额外打印 main 返回值。 |
| 命令行 | `fmjcc --k --opt-mode file.fmj` | 竞赛通常要求 `compiler -S -o out.s in.sy` 或指定技术方案接口 | 新增大赛兼容 CLI，同时保留内部调试参数。 |
| 链接库 | 当前 Makefile 依赖 `libsysy32.s` | 大赛提供 `libsysy.a` / `libsysy.so`，通常静态链接 | `vendor/libsysy` 应迁入根目录，Makefile 改为链接目标平台静态库。 |

## 2. 语法层差异

### 2.1 顶层结构

FDMJ：

```ebnf
Program -> MainMethod ClassDecl*
ClassDecl -> 'public' 'class' id ['extends' id] '{' VarDecl* MethodDecl* '}'
```

SysY：

```ebnf
CompUnit -> [ CompUnit ] ( Decl | FuncDef )
Decl     -> ConstDecl | VarDecl
FuncDef  -> FuncType Ident '(' [FuncFParams] ')' Block
```

需要重写 `parser.yy` 的开始符号和 AST 构造：

- 删除 `public class`、`extends`、`class id`、`this`、对象方法调用、字段访问、`new id()`。
- 新增顶层声明、顶层函数定义、函数声明作用域、块内声明与语句交错。
- `Block` 中允许 `{ BlockItem* }`，`BlockItem` 可为声明或语句；FDMJ 当前 `VarDecl* Statement*` 的顺序限制不适用。

### 2.2 声明与初始化

FDMJ：

- 变量声明只支持 `Type id;` 和数组字面量形式 `int[] a = {1,2};`
- 明确不允许 `int i = 0;`

SysY：

- 支持 `int i = 0;`
- 支持一条声明中声明多个变量：`int a, b = 1, c[4] = {1,2};`
- 支持 `const int N = 10;`
- 支持全局变量、局部变量、全局常量、局部常量。
- 支持多维数组声明、嵌套初始化、部分初始化补零。

需要新增 AST 节点或重构现有节点：

- `ConstDecl`、`ConstDef`
- `VarDecl` 包含多个 `VarDef`
- `InitVal` / `ConstInitVal` 的树形初始化列表
- 多维数组维度列表
- 常量表达式求值器

### 2.3 表达式与语句

SysY 新增或不同点：

- 算术：新增 `%`、`!=`、`<=`、`>`、`>=`。
- 浮点：`floatConst`、浮点算术、浮点比较。
- 表达式语句：`[Exp] ';'`，允许空语句 `;`。
- 函数调用是普通 `Ident '(' args ')'`，不是 `obj.method(args)`。
- `return` 可无表达式，用于 `void` 函数。
- `if` 的 `else` 可选且遵循就近匹配。
- `while` 与 `break` / `continue` 保留，但语义应基于 SysY 条件类型。

当前 FDMJ 已有 `if`、`while`、`break`、`continue`、`return` 和短路逻辑基础，但 parser、AST 和语义规则需要按 SysY 重做。

## 3. 类型系统差异

| 类型能力 | FDMJ 当前 | SysY2022 要求 | 改动 |
| --- | --- | --- | --- |
| `int` | 支持 | 支持 32 位有符号 | 保留。 |
| `float` | `config` 中有长度常量，但 AST/Quad 主体基本按 int/ptr | 完整支持 32 位单精度 | 当前 contest 分支已在 SysY AST lowering、Tree Type、QuadType 和 ARM 指令选择中加入 float；后续优化 pass 仍需按浮点语义继续审计。 |
| `void` | 方法主要返回 `Type`，main 固定 int | 函数可 `void` | 新增 void 类型和 return 检查。 |
| 数组 | 一维 `int[]`，运行时布局含长度 word | 多维 `int`/`float` 数组，按行优先，无内建 length | 改为连续内存布局；删除 FDMJ `length(exp)` 语义。 |
| 对象/类 | 核心特性 | 不存在 | 删除对象布局、vtable、动态派发和继承语义。 |
| 常量 | 只有整数 literal 和局部语义 | `const` 符号常量、常量数组、编译期 ConstExp | 新增常量表和常量折叠。 |

## 4. 作用域和符号表

FDMJ 当前 `namemaps` 围绕 class/method 构建：

- class map
- method map
- class field map
- method local/formal map
- inheritance map

SysY 需要 C 风格作用域：

- 顶层函数/全局变量/全局常量不能重复定义同名标识符。
- 顶层作用域从声明处开始到文件末尾。
- 块作用域允许隐藏外层变量/常量。
- 同名局部变量作用域不能重叠。
- 变量名可以与函数名相同。
- 函数调用需要检查实参数量、标量/数组维度、`int`/`float` 类型。

建议新增 `ScopeStack` 和 `Symbol`：

```text
SymbolKind = Var | Const | Func
Type = Int | Float | Void | Array(base, dims, firstDimUnknown?)
Storage = Global | Local | Param
```

保留 FDMJ `NameMaps` 会导致对象语义残留，迁移时建议替换而不是硬改。

## 5. 数组与内存布局

FDMJ：

- 一维 `int[]`
- `new int[n]`
- 数组首 word 存长度，用于 `length(a)` 和越界检查。

SysY：

- 多维数组，行优先存储。
- 全局数组静态分配，未初始化部分补 0。
- 局部数组通常在栈上分配。
- 形参数组第一维省略，实参传递起始地址。
- 无 `length()`，运行时库也不做数组越界检查。

需要改动：

1. `ast2tree.cc` 的数组访问地址计算改为：

   ```text
   addr = base + element_size * linear_index
   linear_index = (((i0 * dim1 + i1) * dim2 + i2) ...)
   ```

2. 支持数组作为函数实参时的“降维地址”：

   ```c
   int a[4][3];
   f(a[1]);   // 传入 &a[1][0]
   ```

3. 全局数组和全局标量需要进入 `.data` / `.bss`，不能都当作堆对象。

4. 删除 FDMJ 数组头长度布局。若保留内部 `PTR` 类型，也只能表示裸地址。

## 6. 运行时库和内建函数

FDMJ 把 `putint`、`putch`、`putarray`、`getint`、`getch`、`getarray`、`starttime`、`stoptime` 作为语言构造或外部调用。

SysY 把 I/O 和计时全部作为运行时库函数：

```c
int getint();
int getch();
float getfloat();
int getarray(int[]);
int getfarray(float[]);
void putint(int);
void putch(int);
void putfloat(float);
void putarray(int, int[]);
void putfarray(int, float[]);
void putf(char[], ...);
void starttime();
void stoptime();
```

迁移要点：

- parser 不应把 `putint` 等作为关键字语句；它们应作为普通函数调用解析。
- 语义阶段内置这些库函数签名，无需源程序声明。
- `starttime()` / `stoptime()` 实际应降为 `_sysy_starttime(line)` / `_sysy_stoptime(line)`，行号来自源码位置。
- `putf` 第一个参数是字符串，SysY 语法本身没有通用字符串类型，需要 lexer/parser 特判字符串 literal 并在后端生成只读数据段。
- 浮点库函数要求遵守目标 ABI 的浮点参数传递规则。

## 7. IR、Quad 和优化层影响

当前 IR/Quad 主体只区分：

- `INT`
- `PTR`

SysY2022 至少需要：

- `INT`
- `FLOAT`
- `PTR`
- `VOID` 只用于函数签名，不需要成为 Quad value。

具体改动：

- `include/ir/treep.hh`、`include/quad/quad.hh` 增加 `FLOAT` 类型。
- `tree::Const` 不能只存 `int`，需支持 `float` 常量或新增 `FloatConst`。
- `QuadMoveBinop` 的操作要区分 int op 与 float op，或由 operand type 决定。
- 常量传播、SSA PHI、GVN、LICM、strength reduction 要识别浮点运算的合法性。浮点加法/乘法不能随意按整数代数律重排，需谨慎处理 NaN、舍入和竞赛可接受语义。
- `%` 只适用于 int。
- 比较结果仍为 int 0/1。

迁移策略：

1. 已实现整数 SysY 子集，跑通功能样例。
2. 已接入 `float` AST/IR/native 后端和运行时库，默认 native 入口通过 140 个官方 functional/h_functional 用例和本地 `putf` 规格补测。
3. 默认 native 入口通过两个官方 ARM 性能归档；下一步主要是优化调优，避免旧 FDMJ 优化错误处理浮点和 SysY 内存语义。

## 8. ARM/AArch64 后端与 ABI

当前 final 使用 `arm-linux-gnueabihf-gcc`、`qemu-arm` 和 32 位 ARM 汇编。区域赛技术方案需要确认目标平台；章程只说有 ARM 和 RISC-V 后端硬件平台。

需要按实际赛道确认：

- 如果是 AArch64：需要重写或新增 AArch64 后端，整数参数在 `x0`/`w0` 起，浮点参数在 `s0`/`d0` 起。
- 如果是 32 位 ARM hard-float：整数参数在 `r0` 起，float 参数通常走 VFP `s0` 起，需确认 ABI 与 libsysy 构建方式一致。
- `float` 与 `int` 转换需调用 ABI helper 或使用 VFP 指令，如 `__aeabi_i2f`、`__aeabi_f2iz` 等；当前 native 后端已经用这些 helper 处理转换和浮点算术/比较。
- 全局数据、字符串、浮点常量需要生成 `.data` / `.rodata`。
- 多维局部数组可能占用较大栈空间，需要处理栈帧对齐和大立即数偏移。

## 9. 需要删除或停用的 FDMJ 特性

迁移到 SysY 后，以下特性不应出现在语言层：

- `public class`
- `extends`
- `this`
- `class id`
- `obj.field`
- `obj.method(...)`
- `new id()`
- `new int[n]`
- `length(a)`
- FDMJ 对象字段默认初始化、vtable/method pointer、动态派发。
- FDMJ 数组运行时长度 word。

这些代码可以暂时留在内部文件中，但 parser 和语义不应允许 SysY 源程序触发它们。长期应清理，降低错误路径复杂度。

## 10. 测试与验证迁移

建议建立三层测试：

1. **语法/语义测试**：SysY 官方功能测试、边界语义、非法重复定义、作用域、初始化列表、数组传参。
2. **差分测试**：把 SysY 程序转换/包裹为 C 程序，用 `clang/gcc` 或官方运行时库输出作为 oracle；对浮点输出需按官方格式比较。
3. **性能测试**：使用 contest performance tests，记录 baseline、每个优化 pass 的影响、编译时间和运行时间。

原 FDMJ 的解释器不能直接作为 SysY oracle。可以保留其框架，但需要重写 parser/AST 解释语义，或者更现实地使用 C 编译器作为功能 oracle。

## 11. 建议实施顺序

1. 迁移目录结构到 contest 分支根目录，保证现有 FDMJ 编译器仍能 build。
2. 保持 Makefile 不依赖外层 HW 目录；当前 `vendor/libsysy` 已包含 `libsysy32.s`、32 位头/源文件和 64 位头/源文件。
3. 新增 `compiler` 命令名和大赛 CLI，保留 `fmjcc` 作为过渡。
4. 重写 lexer/parser 为 SysY，生成新的 SysY AST。
5. 重写符号表与语义检查，先支持 int、函数、局部/全局变量、一维/多维 int 数组。
6. 改 AST 到 IR，支持全局数据、局部数组栈分配、函数调用。
7. 接入运行时库函数和 `starttime` / `stoptime` 行号。
8. 增加 `float` 类型、浮点常量、浮点运算、隐式类型转换。
9. 恢复 SSA/常量传播/DCE/LICM/strength reduction，对不支持的 float 和内存场景先保守跳过。
10. 增加性能优化：内联、GVN、MemSSA、循环展开、窥孔优化、寄存器分配改进。

截至当前分支，1-8 已形成可验证实现；9-10 属于后续性能和优化正确性工作。
