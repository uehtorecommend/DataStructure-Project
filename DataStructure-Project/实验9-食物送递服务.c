#include <math.h>
#include <stdio.h>

#define MAX_HOUSE 12
#define INF 1e18

typedef struct {
    char name[20];
    double x;
    double y;
    double deadline;
} Household;

typedef struct {
    double time;
    int prevMask;
    int prevHouse;
} DPState;

/* 函数功能：计算两个点之间的欧氏距离。 */
double distance(double x1, double y1, double x2, double y2)
{
    double dx = x1 - x2;
    double dy = y1 - y2;
    return sqrt(dx * dx + dy * dy);
}

/* 函数功能：输出送递路线。 */
void printRoute(Household houses[], int n, DPState dp[][MAX_HOUSE], int endHouse)
{
    int mask = (1 << n) - 1;
    int route[MAX_HOUSE];
    int top = 0;
    int cur = endHouse;

    while (cur != -1) {
        route[top++] = cur;
        int pm = dp[mask][cur].prevMask;
        int ph = dp[mask][cur].prevHouse;
        mask = pm;
        cur = ph;
    }

    printf("最优送递路线：救援点");
    for (int i = top - 1; i >= 0; --i) {
        printf(" -> %s", houses[route[i]].name);
    }
    printf("\n");
}

/* 函数功能：使用状态压缩动态规划求满足截止时间的最短送递方案。 */
int solveFoodDelivery(Household houses[], int n)
{
    DPState dp[1 << MAX_HOUSE][MAX_HOUSE];
    int totalMask = 1 << n;

    for (int mask = 0; mask < totalMask; ++mask) {
        for (int i = 0; i < n; ++i) {
            dp[mask][i].time = INF;
            dp[mask][i].prevMask = -1;
            dp[mask][i].prevHouse = -1;
        }
    }

    for (int i = 0; i < n; ++i) {
        double t = distance(0, 0, houses[i].x, houses[i].y);
        if (t <= houses[i].deadline) {
            int mask = 1 << i;
            dp[mask][i].time = t;
        }
    }

    for (int mask = 1; mask < totalMask; ++mask) {
        for (int last = 0; last < n; ++last) {
            if (dp[mask][last].time >= INF / 2) {
                continue;
            }
            for (int next = 0; next < n; ++next) {
                if (mask & (1 << next)) {
                    continue;
                }
                double arrive = dp[mask][last].time +
                                distance(houses[last].x, houses[last].y,
                                         houses[next].x, houses[next].y);
                if (arrive <= houses[next].deadline) {
                    int nextMask = mask | (1 << next);
                    if (arrive < dp[nextMask][next].time) {
                        dp[nextMask][next].time = arrive;
                        dp[nextMask][next].prevMask = mask;
                        dp[nextMask][next].prevHouse = last;
                    }
                }
            }
        }
    }

    int full = totalMask - 1;
    int best = -1;
    double bestTime = INF;
    for (int i = 0; i < n; ++i) {
        if (dp[full][i].time < bestTime) {
            bestTime = dp[full][i].time;
            best = i;
        }
    }

    if (best == -1) {
        printf("不存在能在所有住户自救时间内完成送递的路线。\n");
        return 0;
    }

    printRoute(houses, n, dp, best);
    printf("全部送达最短时间：%.2f\n", bestTime);
    printf("到达时间校验：全部不超过对应住户自救时间。\n");
    return 1;
}

/* 函数功能：输出住户信息。 */
void printHouseholds(Household houses[], int n)
{
    printf("住户信息：\n");
    for (int i = 0; i < n; ++i) {
        printf("%s 位置(%.1f, %.1f) 自救时间 %.1f\n",
               houses[i].name, houses[i].x, houses[i].y, houses[i].deadline);
    }
}

/* 函数功能：运行一组测试。 */
void runCase(const char *title, Household houses[], int n)
{
    printf("\n%s\n", title);
    printHouseholds(houses, n);
    solveFoodDelivery(houses, n);
}

/* 函数功能：程序入口，运行多组覆盖性测试。 */
int main(void)
{
    Household case1[] = {
        {"H1", 1, 2, 4.0},
        {"H2", 3, 1, 7.0},
        {"H3", 4, 4, 11.0},
        {"H4", 6, 2, 13.0},
        {"H5", 7, 5, 18.0}
    };
    Household case2[] = {
        {"A", 2, 1, 2.5},
        {"B", 5, 1, 4.0},
        {"C", 7, 1, 5.0}
    };
    Household case3[] = {
        {"P1", 1, 0, 1.5},
        {"P2", 2, 0, 3.0},
        {"P3", 3, 0, 4.5}
    };

    runCase("测试用例1：5户可行路线", case1, 5);
    runCase("测试用例2：截止时间过紧，无解", case2, 3);
    runCase("测试用例3：线性分布，可行", case3, 3);
    return 0;
}
