# 校园导游回归测试

在仓库根目录运行（需要 Python 3 和支持 C11 的编译器，可通过 `CC` 指定）：

```sh
python3 tests/test_campus_routes.py
python3 tests/test_campus_routes.py --sanitize
```

`--sanitize` 启用 GCC/Clang AddressSanitizer 和 UndefinedBehaviorSanitizer，适用于支持这些选项的 Linux 环境。如果运行环境不支持 LeakSanitizer，可使用 `ASAN_OPTIONS=detect_leaks=0` 保留地址和未定义行为检查。

测试驱动重命名并包含原程序的 `main`，直接调用实际查询函数，无需修改演示入口。13 项测试覆盖最小反例、多个断连分量、可达与不可达目标混合、孤立起点、无道路、同一起终点、双向直连、间接最短路、零权道路、接近 INF 的可达距离及不存在的景点。固定种子的 40 个随机非负权无向图另检查全部 240 次多目标查询（1200 个目标），由独立的 Floyd 计算核对距离，并验证输出路径的端点、边和总长。

`fixtures/campus_demo.txt` 是修复前直接编译运行原始演示的完整标准输出，用于逐字节比较，保证连通图示例行为保持一致。测试使用临时目录编译，不在源码目录留下二进制文件。

本次只修复查询结果的可达性处理，沿用现有算法的非负权及有限距离小于 `INF=100000000` 的表示约定；不以 `prev == -1` 判断不可达，因为起终点相同也没有前驱，仍应输出自身及距离 0m。
