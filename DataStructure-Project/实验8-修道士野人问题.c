#include <stdio.h>
#include <stdlib.h>

#define MAX_N 10
#define MAX_C 5
#define MAX_STATES 512

typedef struct {
    int mLeft;
    int cLeft;
    int boatLeft;
} State;

typedef struct {
    State state;
    int prev;
    int moveM;
    int moveC;
} QueueNode;

/* 函数功能：判断某一岸的修道士是否安全。 */
int safeBank(int m, int c)
{
    return m == 0 || m >= c;
}

/* 函数功能：判断状态是否合法且安全。 */
int validState(State s, int n)
{
    int mRight = n - s.mLeft;
    int cRight = n - s.cLeft;
    if (s.mLeft < 0 || s.mLeft > n || s.cLeft < 0 || s.cLeft > n) {
        return 0;
    }
    if (s.boatLeft != 0 && s.boatLeft != 1) {
        return 0;
    }
    return safeBank(s.mLeft, s.cLeft) && safeBank(mRight, cRight);
}

/* 函数功能：把状态映射为访问数组下标。 */
int stateIndex(State s, int n)
{
    return (s.mLeft * (n + 1) + s.cLeft) * 2 + s.boatLeft;
}

/* 函数功能：按更清晰的格式输出路径。 */
void printReadablePath(QueueNode queue[], int end, int n)
{
    int stack[MAX_STATES];
    int top = 0;
    for (int i = end; i != -1; i = queue[i].prev) {
        stack[top++] = i;
    }
    printf("安全过河路径如下：\n");
    for (int i = top - 1; i >= 0; --i) {
        State s = queue[stack[i]].state;
        printf("步骤%02d：左岸(M=%d,C=%d) 右岸(M=%d,C=%d) 船在%s\n",
               top - 1 - i, s.mLeft, s.cLeft, n - s.mLeft, n - s.cLeft,
               s.boatLeft ? "左岸" : "右岸");
        if (i > 0) {
            QueueNode next = queue[stack[i - 1]];
            printf("       运送 %d 个修道士、%d 个野人，%s\n",
                   next.moveM, next.moveC,
                   s.boatLeft ? "左岸->右岸" : "右岸->左岸");
        }
    }
}

/* 函数功能：使用广度优先搜索寻找最短安全渡河方案。 */
int solveMissionaries(int n, int cap)
{
    QueueNode queue[MAX_STATES];
    int visited[MAX_STATES] = {0};
    int front = 0;
    int rear = 0;

    State start = {n, n, 1};
    State target = {0, 0, 0};
    queue[rear++] = (QueueNode){start, -1, 0, 0};
    visited[stateIndex(start, n)] = 1;

    while (front < rear) {
        QueueNode cur = queue[front];
        if (cur.state.mLeft == target.mLeft && cur.state.cLeft == target.cLeft &&
            cur.state.boatLeft == target.boatLeft) {
            printReadablePath(queue, front, n);
            return 1;
        }

        for (int m = 0; m <= cap; ++m) {
            for (int c = 0; c <= cap; ++c) {
                int total = m + c;
                if (total < 1 || total > cap) {
                    continue;
                }
                State next = cur.state;
                if (cur.state.boatLeft) {
                    next.mLeft -= m;
                    next.cLeft -= c;
                } else {
                    next.mLeft += m;
                    next.cLeft += c;
                }
                next.boatLeft = 1 - cur.state.boatLeft;
                if (!validState(next, n)) {
                    continue;
                }
                int idx = stateIndex(next, n);
                if (!visited[idx]) {
                    visited[idx] = 1;
                    queue[rear++] = (QueueNode){next, front, m, c};
                }
            }
        }
        ++front;
    }
    printf("N=%d, C=%d 时无安全过河方案。\n", n, cap);
    return 0;
}

/* 函数功能：运行多组测试用例。 */
int main(void)
{
    int cases[][2] = {{3, 2}, {3, 3}, {4, 2}, {1, 1}};
    int count = (int)(sizeof(cases) / sizeof(cases[0]));

    for (int i = 0; i < count; ++i) {
        printf("\n测试用例%d：N=%d, C=%d\n", i + 1, cases[i][0], cases[i][1]);
        solveMissionaries(cases[i][0], cases[i][1]);
    }
    return 0;
}
