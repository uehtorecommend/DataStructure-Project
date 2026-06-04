#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define PLAYERS_COUNT 512

typedef struct Participant {
    int num;
    int score;
} Participant;

typedef struct TournamentNode {
    Participant winner;
    Participant loser;
    int left;
    int right;
} TournamentNode;

Participant players[PLAYERS_COUNT]; /* 存储玩家信息，以数组形式存储 */
int compareCount = 0;

/*
 * 功能：比较两名玩家的得分，返回得分较高者。
 */
Participant betterParticipant(Participant a, Participant b)
{
    compareCount++; /* 记录一次分数比较 */
    if (a.score >= b.score) {
        return a;
    }
    return b;
}

/*
 * 功能：随机生成512名玩家的编号和0~999之间的得分。
 */
void scoreGenerate()
{
    srand(time(NULL));
    for (int i = 0; i < PLAYERS_COUNT; i++) {
        players[i].num = i + 1;       /* 玩家编号从1开始 */
        players[i].score = rand() % 1000; /* 玩家分数处于0~999之间 */
    }
}

/*
 * 功能：按照编号顺序输出所有玩家及其得分。
 */
void printPlayers()
{
    printf("所有游戏参与者及其得分：\n");
    for (int i = 0; i < PLAYERS_COUNT; i++) {
        printf("编号：%3d  得分：%3d", players[i].num, players[i].score);
        if ((i + 1) % 4 == 0) {
            printf("\n"); /* 每行输出4名玩家，便于查看 */
        } else {
            printf("    ");
        }
    }
    printf("\n");
}

/*
 * 功能：使用顺序查找方法找出最高分和次高分玩家，并输出比较次数。
 */
void sortByOrder()
{
    Participant max1, max2;
    compareCount = 0;

    /* 先比较前两名玩家，确定当前最高分和次高分 */
    if (betterParticipant(players[0], players[1]).num == players[0].num) {
        max1 = players[0];
        max2 = players[1];
    } else {
        max1 = players[1];
        max2 = players[0];
    }

    for (int i = 2; i < PLAYERS_COUNT; i++) {
        compareCount++; /* 当前玩家与最高分比较 */
        if (players[i].score > max1.score) {
            max2 = max1;      /* 原最高分变为次高分 */
            max1 = players[i]; /* 当前玩家成为最高分 */
        } else {
            compareCount++; /* 当前玩家与次高分比较 */
            if (players[i].score > max2.score) {
                max2 = players[i]; /* 更新次高分 */
            }
        }
    }

    printf("顺序查找法：\n");
    printf("最高分：编号%d，得分%d\n", max1.num, max1.score);
    printf("次高分：编号%d，得分%d\n", max2.num, max2.score);
    printf("比较次数：%d\n\n", compareCount);
}

/*
 * 功能：使用锦标赛法找出最高分和次高分玩家，并输出比较次数。
 */
void sortByTournament()
{
    TournamentNode tree[PLAYERS_COUNT * 2];
    int leafStart = PLAYERS_COUNT;
    compareCount = 0;

    /* 叶子结点保存原始玩家信息 */
    for (int i = 0; i < PLAYERS_COUNT; i++) {
        int pos = leafStart + i;
        tree[pos].winner = players[i];
        tree[pos].loser = players[i];
        tree[pos].left = tree[pos].right = 0;
    }

    /* 自底向上建立锦标赛树，内部结点保存胜者和败者 */
    for (int i = leafStart - 1; i >= 1; i--) {
        Participant leftWinner = tree[i * 2].winner;
        Participant rightWinner = tree[i * 2 + 1].winner;
        tree[i].left = i * 2;
        tree[i].right = i * 2 + 1;

        compareCount++; /* 两个孩子的胜者进行一次比较 */
        if (leftWinner.score >= rightWinner.score) {
            tree[i].winner = leftWinner;
            tree[i].loser = rightWinner;
        } else {
            tree[i].winner = rightWinner;
            tree[i].loser = leftWinner;
        }
    }

    Participant max1 = tree[1].winner;
    Participant max2;
    int node = 1;
    int hasSecond = 0;

    /* 沿最高分玩家的晋级路径查找其所有直接手下败者中的最大值 */
    while (node < leafStart) {
        Participant loser = tree[node].loser;
        if (!hasSecond) {
            max2 = loser;
            hasSecond = 1;
        } else {
            compareCount++; /* 候选败者与当前次高分比较 */
            if (loser.score > max2.score) {
                max2 = loser;
            }
        }

        /* 继续进入保存最高分玩家的子树 */
        if (tree[tree[node].left].winner.num == max1.num) {
            node = tree[node].left;
        } else {
            node = tree[node].right;
        }
    }

    printf("锦标赛法：\n");
    printf("最高分：编号%d，得分%d\n", max1.num, max1.score);
    printf("次高分：编号%d，得分%d\n", max2.num, max2.score);
    printf("比较次数：%d\n\n", compareCount);
}

/*
 * 功能：对堆中指定结点进行向下调整，维护大顶堆性质。
 */
void heapAdjust(Participant heap[], int root, int length)
{
    Participant temp = heap[root];

    for (int child = root * 2 + 1; child < length; child = child * 2 + 1) {
        if (child + 1 < length) {
            compareCount++; /* 比较左右孩子，选出得分更高的孩子 */
            if (heap[child + 1].score > heap[child].score) {
                child++;
            }
        }

        compareCount++; /* 比较父结点暂存值与较大孩子 */
        if (heap[child].score > temp.score) {
            heap[root] = heap[child]; /* 较大孩子上移 */
            root = child;
        } else {
            break;
        }
    }
    heap[root] = temp; /* 将暂存值放到最终位置 */
}

/*
 * 功能：通过无序序列建大顶堆和一次堆调整找出最高分、次高分，并输出比较次数。
 */
void sortByHeap()
{
    Participant heap[PLAYERS_COUNT];
    compareCount = 0;

    /* 拷贝原数组，避免堆调整影响其他算法的数据 */
    for (int i = 0; i < PLAYERS_COUNT; i++) {
        heap[i] = players[i];
    }

    /* 从最后一个非叶子结点开始，将无序序列建成大顶堆 */
    for (int i = PLAYERS_COUNT / 2 - 1; i >= 0; i--) {
        heapAdjust(heap, i, PLAYERS_COUNT);
    }

    Participant max1 = heap[0]; /* 堆顶即最高分玩家 */

    /* 删除堆顶，用堆尾元素补到堆顶后重新调整 */
    heap[0] = heap[PLAYERS_COUNT - 1];
    heapAdjust(heap, 0, PLAYERS_COUNT - 1);

    Participant max2 = heap[0]; /* 新堆顶即次高分玩家 */

    printf("堆调整法：\n");
    printf("最高分：编号%d，得分%d\n", max1.num, max1.score);
    printf("次高分：编号%d，得分%d\n", max2.num, max2.score);
    printf("比较次数：%d\n\n", compareCount);
}

/*
 * 功能：程序入口，完成随机数据生成、数据输出和三种查找算法调用。
 */
int main()
{
    scoreGenerate(); /* 随机生成512名玩家的分数 */
    printPlayers();  /* 输出所有玩家编号及其得分 */

    /* 用顺序查找方法查找最高分和次高分，并给出比较次数 */
    sortByOrder();

    /* 按照锦标赛法查找最高分和次高分，并给出比较次数 */
    sortByTournament();

    /* 通过无序序列建堆和堆调整得到最高分和次高分，并给出比较次数 */
    sortByHeap();

    system("pause");
    
    return 0;
}
