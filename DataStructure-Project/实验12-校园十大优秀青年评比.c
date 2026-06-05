#include <stdio.h>
#include <string.h>

#define HASH_SIZE 31
#define MAX_CANDIDATE 31

typedef struct {
    char name[30];
    char className[40];
    char major[40];
    char grade[20];
    char achievement[160];
    int votes;
    int used;
} Candidate;

typedef struct {
    Candidate data[HASH_SIZE];
} HashTable;

/* 函数功能：初始化候选人散列表。 */
void initTable(HashTable *table)
{
    for (int i = 0; i < HASH_SIZE; ++i) {
        table->data[i].used = 0;
        table->data[i].votes = 0;
    }
}

/* 函数功能：字符串哈希函数。 */
int hashName(const char *name)
{
    unsigned int h = 0;
    while (*name) {
        h = h * 131u + (unsigned char)(*name);
        ++name;
    }
    return (int)(h % HASH_SIZE);
}

/* 函数功能：查找候选人在散列表中的位置。 */
int findCandidate(const HashTable *table, const char *name, int *compareCount)
{
    int start = hashName(name);
    *compareCount = 0;
    for (int i = 0; i < HASH_SIZE; ++i) {
        int pos = (start + i) % HASH_SIZE;
        ++(*compareCount);
        if (!table->data[pos].used) {
            return -1;
        }
        if (strcmp(table->data[pos].name, name) == 0) {
            return pos;
        }
    }
    return -1;
}

/* 函数功能：提名候选人。 */
int nominate(HashTable *table, Candidate candidate)
{
    int compare = 0;
    if (findCandidate(table, candidate.name, &compare) != -1) {
        printf("候选人%s已存在，不能重复提名。\n", candidate.name);
        return 0;
    }
    int start = hashName(candidate.name);
    for (int i = 0; i < HASH_SIZE; ++i) {
        int pos = (start + i) % HASH_SIZE;
        if (!table->data[pos].used) {
            candidate.used = 1;
            table->data[pos] = candidate;
            printf("提名成功：%s，散列地址=%d\n", candidate.name, pos);
            return 1;
        }
    }
    printf("散列表已满，提名失败。\n");
    return 0;
}

/* 函数功能：为候选人投票。 */
int vote(HashTable *table, const char *name)
{
    int compare = 0;
    int pos = findCandidate(table, name, &compare);
    if (pos == -1) {
        printf("投票失败：未找到候选人%s。\n", name);
        return 0;
    }
    table->data[pos].votes++;
    printf("投票成功：%s 当前票数=%d，查找比较次数=%d\n",
           name, table->data[pos].votes, compare);
    return 1;
}

/* 函数功能：查看候选人基本信息。 */
void showCandidate(const HashTable *table, const char *name)
{
    int compare = 0;
    int pos = findCandidate(table, name, &compare);
    if (pos == -1) {
        printf("未找到候选人%s。\n", name);
        return;
    }
    Candidate c = table->data[pos];
    printf("姓名：%s  班级：%s  专业：%s  年级：%s  票数：%d\n事迹：%s\n",
           c.name, c.className, c.major, c.grade, c.votes, c.achievement);
}

/* 函数功能：显示所有提名学生票数。 */
void showAllVotes(const HashTable *table)
{
    printf("所有候选人票数：\n");
    for (int i = 0; i < HASH_SIZE; ++i) {
        if (table->data[i].used) {
            printf("地址%02d：%-8s 票数=%d\n", i, table->data[i].name, table->data[i].votes);
        }
    }
}

/* 函数功能：显示排行榜。 */
void showRanking(const HashTable *table)
{
    Candidate arr[MAX_CANDIDATE];
    int n = 0;
    for (int i = 0; i < HASH_SIZE; ++i) {
        if (table->data[i].used) {
            arr[n++] = table->data[i];
        }
    }
    for (int i = 0; i < n - 1; ++i) {
        for (int j = 0; j < n - 1 - i; ++j) {
            if (arr[j].votes < arr[j + 1].votes) {
                Candidate tmp = arr[j];
                arr[j] = arr[j + 1];
                arr[j + 1] = tmp;
            }
        }
    }
    printf("排行榜：\n");
    for (int i = 0; i < n; ++i) {
        printf("第%d名：%s  票数=%d\n", i + 1, arr[i].name, arr[i].votes);
    }
}

/* 函数功能：创建候选人记录。 */
Candidate makeCandidate(const char *name, const char *className, const char *major,
                        const char *grade, const char *achievement)
{
    Candidate c;
    strcpy(c.name, name);
    strcpy(c.className, className);
    strcpy(c.major, major);
    strcpy(c.grade, grade);
    strcpy(c.achievement, achievement);
    c.votes = 0;
    c.used = 1;
    return c;
}

/* 函数功能：程序入口，演示评比系统功能。 */
int main(void)
{
    HashTable table;
    initTable(&table);

    nominate(&table, makeCandidate("学生A", "匿名班级1", "计算机类", "2024级", "积极参加科研训练与志愿服务。"));
    nominate(&table, makeCandidate("学生B", "匿名班级2", "软件工程", "2024级", "竞赛成绩突出，参与多项校园服务。"));
    nominate(&table, makeCandidate("学生C", "匿名班级3", "人工智能", "2024级", "学习成绩优秀，组织班级活动。"));
    nominate(&table, makeCandidate("学生D", "匿名班级4", "网络空间安全", "2024级", "热心公益，技术实践能力强。"));
    nominate(&table, makeCandidate("学生A", "匿名班级1", "计算机类", "2024级", "重复提名测试。"));

    vote(&table, "学生A");
    vote(&table, "学生B");
    vote(&table, "学生B");
    vote(&table, "学生C");
    vote(&table, "学生C");
    vote(&table, "学生C");
    vote(&table, "不存在学生");
    vote(&table, "学生D");

    printf("\n查看指定学生信息：\n");
    showCandidate(&table, "学生C");
    showCandidate(&table, "不存在学生");

    printf("\n");
    showAllVotes(&table);
    printf("\n");
    showRanking(&table);
    return 0;
}
