# SysY2022 与 FDMJ2026 差异及 AArch64 迁移清单

本文记录从课程 FDMJ 编译器到 2026 编译系统设计赛 SysY2022 实现赛道的迁移结果。当前 `contest` 分支已经把原 `final/` 工程迁移到仓库根目录，并收敛为 AArch64-only 竞赛编译器。

## 当前工程结构

- 比赛入口：`tools/compiler/main.cc`，构建产物为 `build/compiler`。
- SysY 前端：`include/sysy/*`、`lib/sysy/lexer.cc`、`lib/sysy/parser.cc`、`lib/sysy/semantics.cc`。
- SysY lowering：`lib/sysy/lower_tree.cc`，把 SysY AST 降到 Tree IR。
- 共享中端：`include/ir`、`lib/ir`、`include/quad`、`lib/quad`、`include/quadflow`、`lib/quadflow`、`include/opt`、`lib/opt`。
- AArch64 后端：`include/backend/backend_driver.hh`、`lib/backend/backend_driver.cc`。
- SysY 运行库：`vendor/libsysy/sylib.c`、`vendor/libsysy/sylib.h`。

已经删除的旧组件：

- FDMJ frontend、AST、interpreter、XML bridge、旧 course harness。
- ARM32 instruction selection 和 register allocation 目录：`include/instr`、`lib/instr`、`include/reg`、`lib/reg`。
- 旧 32/64 位课程 runtime：`libsysy32.*`、`libsysy64.*`、`libsysy_arm.a`。
- 外部 GCC/asm bridge 回归脚本。

## 当前迁移状态

- `compiler -S -o out.s in.sy` 默认直接使用 SysY frontend、Tree/Quad/SSA/优化和 native AArch64 backend。
- `--target aarch64` 作为兼容 no-op；ARM32 target、GCC bridge、AArch64 GCC bridge 和 parallel asm bridge 均被拒绝。
- Tree/Quad 已支持 `INT`、`FLOAT` 和 `PTR` value 类型；`VOID` 只作为函数签名语义存在。
- AArch64 后端遵守 AAPCS64：整数/指针走 `w/x` 寄存器，浮点走 `s/d` 寄存器，`putf` 的 `%f` vararg 提升为 double。
- lowering 和 native parallel path 使用 8 字节 pointer layout；并行 context 不再保留 ARM32 的 4 字节指针假设。
- `-O1`/`allopt` 默认启用 native loop parallelization，除非显式传 `--no-parallel-native`。
- `make run*`、`make compile` 和 regression 脚本均按 AArch64 编译、链接和运行。

## SysY 与 FDMJ 的核心差异

| 方面 | FDMJ | SysY2022 | 当前处理 |
| --- | --- | --- | --- |
| 顶层结构 | `public class`、`main` method、类声明 | 顶层声明和函数定义 | 已用 SysY `CompUnit` parser 替换。 |
| 对象系统 | 类、继承、字段、方法、动态派发 | 不存在 | 旧对象语义和 lowering 已删除。 |
| 函数 | 方法属于类 | 顶层过程式函数 | 语义阶段维护函数符号和块作用域。 |
| 类型 | `int`、一维 `int[]`、对象引用 | `int`、`float`、`void`、多维数组 | lowering 和 AArch64 ABI 已支持。 |
| 数组 | FDMJ 数组首 word 保存长度 | 连续行优先布局，无内建长度 | 已改为裸地址和线性下标计算。 |
| 初始化 | 简单声明和数组字面量 | 全局/局部/const、多维初始化、补零 | 已在 SysY AST/lowering 中处理。 |
| 运行时库 | 部分内建语句/外部调用 | `sylib` 函数 ABI | 语义内置签名，后端生成标准调用。 |
| 输出接口 | 课程 harness | `compiler -S -o out.s input.sy` | 已实现 contest CLI。 |

## AArch64 ABI 和后端实现

目标平台是 ARMv8-A/AArch64。当前 backend 不再复用 ARM32 指令选择，而是在 `lib/backend/backend_driver.cc` 中直接从最终 Quad 生成 AArch64 汇编。

已实现：

- 64 位栈帧和指针栈槽。
- AAPCS64 整数、指针、浮点参数和返回值传递。
- `int`/`float` 转换、浮点算术、浮点比较使用 AArch64 FP 指令。
- 全局变量、全局数组、字符串常量和 `.rodata`/`.data` 输出。
- `getint/getfloat/getarray/getfarray/put*`、`starttime/stoptime` 和 `putf` 调用。
- 嵌入式 pthread 并行 runtime，仅当汇编中实际生成并行 loop helper 时才需要 `-pthread`。

后续主要性能工作：

- 把 stack-code baseline 升级为 AArch64 register-allocating backend。
- 增加 AArch64 peephole 和寻址模式融合。
- 审计 LICM/strength reduction 对 SysY 数组、浮点和并行 lowering 的盈利性。
- 研究 NEON/vector lowering、函数内联、GVN/MemSSA、循环展开和更细粒度并行 profitability。

## 测试策略

当前测试目录包含：

- `test/functional`
- `test/h_functional`
- `test/performance_final`
- `test/reject`
- FINALREPORT 补充用例

推荐验证命令：

```sh
make build
make compile
make run
SYSY_OPT='-O1' make run
THREADS=2 make sysy-parallel-native-regression
```

`make run` 会递归扫描 `test/` 下带 `.out` 的可执行 `.sy` 文件，逐个编译、AArch64 链接、`qemu-aarch64` 运行，并比较 stdout 与 return code。

## 已完成目标对应关系

1. 迁移目录结构到 contest 分支根目录：已完成。
2. 建立 `compiler -S -o out.s in.sy` 竞赛入口：已完成。
3. 删除 FDMJ-only 源码和工具：已完成。
4. 重写 SysY lexer/parser/semantic checker：已完成。
5. SysY AST 降到 Tree/Quad/SSA：已完成。
6. 支持 `int`、`float`、`void`、多维数组、全局对象和运行时库 ABI：已完成。
7. 后端迁到 AArch64 ARMv8-A：已完成。
8. 删除 ARM32 backend 和 32 位 runtime：已完成。
9. 接入优化和 native parallel lowering：已完成。
10. 深度性能优化：持续进行，当前重点是 AArch64 寄存器分配和后端 peephole。
