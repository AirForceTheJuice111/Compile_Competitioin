这里给出一系列的Tests，供大家参考。Final Report最后批改时，会使用其中一部分tests加上一些新的tests来测试大家的编译器。

oldtests: 是一些之前用过的fmj程序。有些有语义错误，有些有运行错误，但都进行了编译，输出结果供参考。

newtests：是一些新的fmj程序（包括两个“古老的”bubblesort和fibonacci）。这些都应该没有语义错误，也都可以运行。

一个fmj程序（若编译成功）会产生下面几个优化后的程序，命名规则：（1）root_constopt 指的是经过Conditional Constant Propagation；（2）root_loophoist 指的是经过 loop invariant computation hoist 优化过的程序；3) root_loopind 指的是经过 induction variable strength reduction 优化过的程序；4) root_loophoist_loopind 指的是同时经过两个loop优化的程序。其他文件都是整个学期一路使用的命名规则。另外，root.output 文件收集了一些编译时输出的诊断信息，也供参考。

每个成功编译的程序最终使用k=2, 3, 5, 9几个寄存器的分配方法，并将最后的ARM汇编程序放到了相应的 directory 下面，可以用HW12给出的方法用qemu-arm直接运行。
