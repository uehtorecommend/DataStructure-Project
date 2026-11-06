# 实验6：图的存储与遍历回归测试

在仓库根目录运行（Python 3 与 C11 编译器，可通过 `CC` 指定）：

```sh
python3 tests/test_graph_traversal.py -v
ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1 python3 tests/test_graph_traversal.py --sanitize -v
```

C 驱动包含实际源码并重命名演示入口，调用原有的建图、邻接点查询、DFS/BFS 与销毁函数。13 项测试包括零权边最小反例、后续邻接点为零权边、混合权连通路径、正权边、孤立顶点与初始化对角线、非连通图、全零权环、原始连通图、接近 INF 的边权和负权边、空图、20 顶点完全零权图、60 个固定种子随机图，以及演示输出逐字节比较。每次执行设置 10 秒超时，编译产物在临时目录中。

独立 Python 参考实现按矩阵的下标升序、邻接表的头插顺序分别计算 DFS/BFS，检查全部邻接点、完整遍历顺序，并从每个顶点单独执行 DFS 比较可达集合。两种存储方式允许不同的合法访问顺序；全图遍历会补访断连分量，仅检查访问过全部顶点不足以发现漏边。

`fixtures/graph_traversal_demo.txt` 来自修改前 main 分支程序的实际标准输出。工作流 `graph-traversal.yml` 分别执行普通模式和 ASan/UBSan 模式，启用泄漏检测与 sanitizer 出错即失败。

沿用无自环的无向网测试范围和有限边权小于 `INF=100000000` 的表示约定。邻接点查询按下标排除对角线上的初始 0，非对角线使用 `< INF` 判断边存在，允许零权边。此修改不扩展到自环、重复边或输入校验；若未来要表示零权自环，需要单独的边存在标记，因为当前对角线初始值与零权自环均为 0。
