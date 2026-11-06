# 实验12候选人字段约束与回归验证

五个字段均为以空字符结尾的 C 字符串，以下上限按字节计，不包含结尾空字符；UTF-8 中文通常占多个字节。

| 字段 | 最大有效字节数 | 空字符串 |
| --- | ---: | --- |
| name | 29 | 拒绝 |
| className | 39 | 允许 |
| major | 39 | 允许 |
| grade | 19 | 允许 |
| achievement | 159 | 允许 |

所有字段均拒绝 NULL、超长或在相应容量内没有终止符的输入。调用者必须传入有效的 C 字符串，或至少可读取相应容量字节的缓冲区；C 接口无法判断任意悬空指针是否有效。不截断、不改变姓名字节，避免不同姓名被合并、哈希与存储姓名不一致或截断 UTF-8 字符。

`makeCandidate()` 保留原有按值返回接口：成功 `used=1, votes=0`；任一字段无效则返回全零记录（`used=0`）。`nominate()` 会拒绝无效记录并输出失败提示，不修改散列表；外部直接构造的记录也须满足上述约束、`used=1` 且票数非负。重复姓名仍按完整字节精确比较，拒绝重复提名且保留已有记录和票数。姓名是本程序唯一键，同名不同人仍无法分别提名，未引入新的身份字段。

`findCandidate()` 无效姓名返回 -1，比较次数为0（计数输出指针可为 NULL）。`vote()`、`showCandidate()` 对无效姓名输出固定错误提示，不将 NULL 或未终止输入交给 `%s`。正常投票只更新精确匹配者，票数达到 `INT_MAX` 后拒绝继续增加。线性探测保留，最多检查31个槽位。初始化后的表须通过 `nominate()` 写入，调用者不得直接破坏内部字符串或票数。

运行：

```sh
python3 tests/test_youth_candidates.py -v
ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1 python3 tests/test_youth_candidates.py --sanitize -v
```

测试驱动包含完整生产源文件（仅将其 main 重命名），另行编译原程序入口。6组回归测试包括：五字段的空值、1字节、最大长度、最大长度+1/+2、319和4095字节；原报告32字节姓名；五字段 NULL、精确容量且未终止的输入和直接构造的无效记录；完整姓名与 UTF-8 边界；重复提名；31个同哈希姓名导致的跨尾部探测、满表、缺失查找；正常查询、投票、排名与票数溢出保护。失败提名/投票检查表内容未被修改。

`tests/fixtures/youth_demo.txt` 在修改前由 main 分支原程序生成，修复后的标准输出须逐字节相同。独立工作流 `.github/workflows/youth-candidates.yml` 在相关文件变化时分别执行普通与 ASan/UBSan 测试。

本地受限环境的 LeakSanitizer 无法读取 `/proc` 线程信息，故本地验证使用 `ASAN_OPTIONS=detect_leaks=0:halt_on_error=1`，地址与未定义行为检测均启用；GitHub Actions 仍开启泄漏检测。
