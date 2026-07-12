# Mac ARM64 分片回归

`scripts/sysy_mac_distributed_regression.sh` 用 Mac 上现有的 ARM64 Multipass
实例并行完成编译、链接、原生执行和精确输出检查。它面向频繁的正确性回归；性能用例的正式计时仍应独占核心串行运行，不能把并发回归中的运行时间当作 benchmark 成绩。

## 容量与并发选择

2026-07-12 的实机盘点结果：

| 资源 | Mac Studio | `docker-node` VM |
| --- | ---: | ---: |
| CPU | 32 核（24 性能核 + 8 能效核） | 32 vCPU |
| 内存 | 512 GiB | 31.3 GiB，测试前约 30 GiB 可用 |
| 可用磁盘 | 572 GiB | 约 188 GiB |

因此默认使用一个 VM 中的 16 个隔离 worker，而不克隆 16 个完整 VM。每个 worker
拥有独立目录，编译器和测试输入只读共享；这样同样避免输出碰撞，同时省掉 16 份镜像、启动和包管理开销。控制器要求至少 1 GiB 可用内存/worker，并拒绝超过 16 个 worker。

## 首次准备与常用命令

首次使用会在 VM 中安装 GCC、CMake、Ninja 和 GNU time：

```bash
scripts/sysy_mac_distributed_regression.sh --prepare --workers 16
```

以后运行 16 分片功能回归：

```bash
scripts/sysy_mac_distributed_regression.sh --workers 16
```

同时测量同一负载的一分片基线与 16 分片结果：

```bash
scripts/sysy_mac_distributed_regression.sh --benchmark --workers 16
```

指定测试树和编译参数：

```bash
SYSY_OPT="--parallel-native -O1" SYSY_THREADS=2 \
  scripts/sysy_mac_distributed_regression.sh \
  --test-root test/performance --workers 16
```

并发性能回归只适合发现错误。要记录单个性能用例的时间，应退出并发模式，并按统一 affinity 单独测量。

其他实用选项：

```bash
# 为疑似死循环的回归设置 120 秒超时；默认 0 表示不限制
scripts/sysy_mac_distributed_regression.sh --case-timeout 120

# 查看或清除内容寻址缓存
scripts/sysy_mac_distributed_regression.sh --list-cache
scripts/sysy_mac_distributed_regression.sh --clean-cache

# 调试通过的用例时也保留汇编、可执行文件和输出
KEEP_PASS_ARTIFACTS=1 scripts/sysy_mac_distributed_regression.sh
```

## 执行与校验模型

控制器对当前工作区中的编译器源码、测试树和 harness 做确定性打包并计算 SHA-256。缓存未命中时，负载只传输一次；VM 只运行一次 CMake/Ninja 构建，并把 `sylib.c` 预编译为共享只读对象。缓存命中时不会再启动 CMake 或 Ninja。

有期望输出的 `.sy` 文件按 `LC_ALL=C` 排序，第 `i` 个用例固定分配给
`i % worker_count`，所以失败总能复现到同一分片。每个用例依次完成：

1. 使用缓存中的同一个编译器生成 AArch64 汇编；
2. 在 ARM64 VM 中原生链接；
3. 使用对应 `.in` 原生执行；
4. 去除 CR、补齐末尾换行、追加进程退出码，再与 `.out` 逐字节比较。

成功用例的大体积中间产物默认删除。失败用例完整保留 compile/link stdout/stderr、程序 stdout/stderr、规范化输出和 diff。控制器直接通过 SSH/Multipass 流式拉回压缩结果，不在 Mac 主机留下中转包。

本地结果默认位于 `/tmp/sysy_mac_distributed_regression/<run-id>/`：

- `summary.env`：总数、分类计数、墙钟、CPU 时间与 RSS；
- `failures.tsv`：所有失败的紧凑汇总；
- `shards/shard-NN.log`：逐用例状态；
- `shards/shard-NN/resource.txt`：GNU time 资源报告；
- `shards/shard-NN/cases/`：仅失败用例的完整产物。

`--keep-remote` 可保留 VM 端 run 目录；默认在结果拉回成功后清除。内容缓存保存在
`/home/ubuntu/.cache/sysy-mac-regression/cache/`，源码、测试或 harness 任一内容变化都会自动生成新缓存键。

## 实测结果

测试对象为当前 `test/functional` 的 137 个用例；串行和并行均为
`137/137 PASS`，无编译、链接、超时或输出错误。构建时间不计入下表 VM 内执行时间。

| 模式 | VM 内墙钟 | 加速比 | 墙钟减少 | 最大单分片 RSS |
| --- | ---: | ---: | ---: | ---: |
| 1 worker | 7.159 s | 1.00x | — | 138,016 KiB |
| 16 workers | 1.439 s | 4.97x | 79.9% | 138,020 KiB |

16 分片各自峰值 RSS 的保守求和为 337,064 KiB，远低于 VM 可用内存。其累计 user/system CPU 时间分别为 3.280 s / 5.520 s；串行对应 3.160 s / 3.570 s，额外 system 时间主要来自并发进程创建、链接和文件操作。

缓存命中、包含容量检查、确定性打包、SSH 调度和日志拉回的端到端计时为：

| 模式 | 完整命令墙钟 | 加速比 | 墙钟减少 |
| --- | ---: | ---: | ---: |
| 1 worker | 8.93 s | 1.00x | — |
| 16 workers | 3.85 s | 2.32x | 56.9% |

通过用例产物清理后，拉回的串行结果目录约 48 KiB，16 分片结果约 400 KiB；失败时只增加失败 case 的诊断产物。
