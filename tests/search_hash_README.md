# 实验7：整数关键字哈希表回归

在仓库根目录运行（Python 3、C11 编译器；可用 `CC` 指定编译器）：

```sh
python3 tests/test_search_hash.py -v
python3 tests/test_search_hash.py --sanitize -v
```

测试驱动直接包含实验7源码并重命名演示入口，临时目录编译，启用 `-Wall -Wextra -Werror -pedantic`。12 项测试覆盖正数、负数、零、-1、由 C 编译器提供的 INT_MIN/INT_MAX、空表、冲突回绕、比较次数、重复插入、满表的新键失败和旧键成功、不存在关键字、重新初始化、-1 的打印，以及原始演示完整输出。100 组固定种子随机序列使用 Python 字典核对成员关系、容量、返回位置一致性和位置唯一性，不复刻 C 探测循环；每次插入后检查所有候选键，包括插入失败后的旧键。fixture 是修复前 main 分支演示的原始标准输出，逐字节比较。

## 实际复现

基线提交：`73a8f194fb4007d92087e7b6fd7f7559c9a65c58`。将以下驱动与该提交实验7源码（另存为 `original.c`）放在同一临时目录：

```c
#define main demo_main
#include "original.c"
#undef main
int main(int argc, char **argv) {
    HashTable table;
    int count;
    (void)argv;
    initHashTable(&table);
    if (argc == 1) return insertHash(&table, -2);
    return searchHash(&table, -2, &count);
}
```

```sh
cc -std=c11 -g -fsanitize=address,undefined -fno-sanitize-recover=all -no-pie repro.c -o repro
./repro
./repro search
```

两次运行均退出 1，AddressSanitizer 分别在原源码第132行 `insertHash` 和第148行 `searchHash` 报告 `stack-buffer-overflow`，访问位于 `table` 前方。

另外，临时仅归一化基线 `hash()`、保留空槽哨兵后，空表查找 -1 错误返回12；依次插入 -1 和12都返回12，原槽被12覆盖，而再次查找 -1 错误返回空槽0。这证明仅修正取模仍不足以支持 -1。

## 修复契约与边界

`hash()` 对负余数加 HASH_SIZE，不对 key 取反，因此 INT_MIN 不会触发取反溢出。地址范围为0～12，探测次数最多13，`address + i` 最多24。所有合法 int 值均可作为关键字，空槽由独立 `occupied` 数组表示；插入、查找和打印共用该状态。重复插入返回原位置，不消耗容量；满表插入新键和查找缺失键返回-1。查找比较次数每次调用重置，最多13。与原接口相同，调用方须先初始化表，并提供有效的表指针及比较次数输出指针；本实现不提供删除操作。

## CI 与验证

独立工作流 `.github/workflows/search-hash.yml` 按实验7源码、驱动、测试、fixture 和工作流自身的路径触发 push/pull_request，普通与 ASan/UBSan 分任务执行。每次子进程10秒超时，任务10分钟超时。sanitizer 禁止错误恢复，CI 设置 `ASAN_OPTIONS=detect_leaks=1:halt_on_error=1` 和 `UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1`。

本地普通测试和 ASan/UBSan 均通过。此运行环境的 LeakSanitizer 无法读取 `/proc/<pid>/task`，故本地使用 `ASAN_OPTIONS=detect_leaks=0:halt_on_error=1`；地址与未定义行为检查仍启用。实验10的16项测试、实验11的344个路径用例及已有契约/演示检查亦在普通和 sanitizer 模式下通过。本次不修改实验6、10、11源码、测试或工作流。
