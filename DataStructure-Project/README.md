# DataStructure-Project

本项目整理了数据结构课程实验的源代码与实验报告。

## 已包含实验

| 序号 | 实验题目 | 源码 | 报告 |
|---|---|---|---|
| 1 | 约瑟夫环 | `实验1-约瑟夫环.c` | `数据结构实验(1).docx` |
| 2 | 教学计划编制 | `实验2-教学计划编制.cpp` | `数据结构实验(2).docx` |
| 3 | 查找最高分与次高分 | `实验3-查找最高分与次高分.c` | `数据结构实验(3).docx` |
| 4 | 线性表基本操作 | `实验4-线性表基本操作.c` | `数据结构实验(4).docx` |
| 5 | 集合的交并差运算 | `实验5-集合交并差.c` | `数据结构实验(5).docx` |
| 6 | 图的存储与遍历 | `实验6-图的存储与遍历.c` | `数据结构实验(6).docx` |
| 7 | 查找算法验证 | `实验7-查找算法验证.c` | `数据结构实验(7).docx` |
| 8 | 修道士野人问题 | `实验8-修道士野人问题.c` | `数据结构实验(8).docx` |
| 9 | 食物送递服务 | `实验9-食物送递服务.c` | `数据结构实验(9).docx` |
| 10 | 校园导游 | `实验10-校园导游.c` | `数据结构实验(10).docx` |
| 11 | 中国邮路问题 | `实验11-中国邮路问题.c` | `数据结构实验(11).docx` |
| 12 | 校园十大优秀青年评比 | `实验12-校园十大优秀青年评比.c` | `数据结构实验(12).docx` |

## 编译运行

Windows + MinGW 示例：

```powershell
gcc -std=c11 -Wall -Wextra -pedantic "实验1-约瑟夫环.c" -o "实验1-约瑟夫环.exe"
g++ -std=c++11 -Wall -Wextra -pedantic "实验2-教学计划编制.cpp" -o "实验2-教学计划编制.exe"
gcc -std=c11 -Wall -Wextra -pedantic "实验3-查找最高分与次高分.c" -o "实验3-查找最高分与次高分.exe"
gcc -std=c11 -Wall -Wextra -pedantic "实验4-线性表基本操作.c" -o "实验4-线性表基本操作.exe"
gcc -std=c11 -Wall -Wextra -pedantic "实验5-集合交并差.c" -o "实验5-集合交并差.exe"
gcc -std=c11 -Wall -Wextra -pedantic "实验6-图的存储与遍历.c" -o "实验6-图的存储与遍历.exe"
gcc -std=c11 -Wall -Wextra -pedantic "实验7-查找算法验证.c" -o "实验7-查找算法验证.exe"
gcc -std=c11 -Wall -Wextra -pedantic "实验8-修道士野人问题.c" -o "实验8-修道士野人问题.exe"
gcc -std=c11 -Wall -Wextra -pedantic "实验9-食物送递服务.c" -o "实验9-食物送递服务.exe" -lm
gcc -std=c11 -Wall -Wextra -pedantic "实验10-校园导游.c" -o "实验10-校园导游.exe"
gcc -std=c11 -Wall -Wextra -pedantic "实验11-中国邮路问题.c" -o "实验11-中国邮路问题.exe"
gcc -std=c11 -Wall -Wextra -pedantic "实验12-校园十大优秀青年评比.c" -o "实验12-校园十大优秀青年评比.exe"
gcc -std=c11 -Wall -Wextra -pedantic "实验13-教学计划编制重写版.c" -o "实验13-教学计划编制重写版.exe"
```

多数程序带内置测试数据，编译后直接运行即可查看覆盖性测试结果。
