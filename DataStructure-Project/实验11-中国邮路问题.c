#include <stdio.h>
#include <limits.h>

#define MAXV 12
#define MAXE 40
#define INF (LLONG_MAX / 4)

typedef struct {
    int u;
    int v;
    int w;
} Edge;

typedef struct {
    int n;
    int m;
    long long graph[MAXV][MAXV];
    Edge edges[MAXE];
} PostGraph;

/* 函数功能：初始化邮路图。 */
int initGraph(PostGraph *g, int n)
{
    g->n = 0;
    g->m = 0;
    if (n < 1 || n > MAXV) {
        return 0;
    }
    g->n = n;
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            g->graph[i][j] = (i == j) ? 0 : INF;
        }
    }
    return 1;
}

/* 添加非负权无向街道；失败返回0，且不改变图。允许平行边和自环。 */
int addStreet(PostGraph *g, int u, int v, int w)
{
    if (g->n < 1 || g->n > MAXV || g->m < 0 || g->m >= MAXE ||
        u < 0 || u >= g->n || v < 0 || v >= g->n || w < 0) {
        return 0;
    }
    /* 最短路径只需最轻的平行边；每条实际街道仍须单独计长、计度。 */
    if (w < g->graph[u][v]) {
        g->graph[u][v] = w;
        g->graph[v][u] = w;
    }
    g->edges[g->m++] = (Edge){u, v, w};
    return 1;
}

/* 函数功能：Floyd算法求任意两点最短路径。 */
void floyd(const PostGraph *g, long long dist[][MAXV])
{
    for (int i = 0; i < g->n; ++i) {
        for (int j = 0; j < g->n; ++j) {
            dist[i][j] = g->graph[i][j];
        }
    }
    for (int k = 0; k < g->n; ++k) {
        for (int i = 0; i < g->n; ++i) {
            for (int j = 0; j < g->n; ++j) {
                if (dist[i][k] < INF && dist[k][j] < INF &&
                    dist[i][k] + dist[k][j] < dist[i][j]) {
                    dist[i][j] = dist[i][k] + dist[k][j];
                }
            }
        }
    }
}

/* 函数功能：递归求奇点最小完美匹配代价。 */
long long matchOdd(int odd[], int oddCount, int used[], long long dist[][MAXV], int pair[][2], int temp[][2], int depth)
{
    int first = -1;
    for (int i = 0; i < oddCount; ++i) {
        if (!used[i]) {
            first = i;
            break;
        }
    }
    if (first == -1) {
        for (int i = 0; i < depth; ++i) {
            pair[i][0] = temp[i][0];
            pair[i][1] = temp[i][1];
        }
        return 0;
    }

    long long best = INF;
    used[first] = 1;
    for (int j = first + 1; j < oddCount; ++j) {
        if (!used[j]) {
            used[j] = 1;
            temp[depth][0] = odd[first];
            temp[depth][1] = odd[j];
            int subPair[MAXV][2];
            long long subCost = matchOdd(odd, oddCount, used, dist, subPair, temp, depth + 1);
            long long cost = dist[odd[first]][odd[j]] + subCost;
            if (cost < best) {
                best = cost;
                for (int k = 0; k < depth; ++k) {
                    pair[k][0] = temp[k][0];
                    pair[k][1] = temp[k][1];
                }
                pair[depth][0] = odd[first];
                pair[depth][1] = odd[j];
                for (int k = depth + 1; k < oddCount / 2; ++k) {
                    pair[k][0] = subPair[k][0];
                    pair[k][1] = subPair[k][1];
                }
            }
            used[j] = 0;
        }
    }
    used[first] = 0;
    return best;
}

/* 输出并返回最短闭合投递长度；非法邮局或不可达街道返回-1。
 * 仅计算长度及奇点配对，不生成逐街道行走序列。g须由上述接口构造。
 */
long long solveChinesePostman(const PostGraph *g, int postOffice)
{
    int degree[MAXV] = {0};
    int odd[MAXV];
    int oddCount = 0;
    long long total = 0;
    long long dist[MAXV][MAXV];
    int used[MAXV] = {0};
    int pair[MAXV][2] = {{0}};
    int temp[MAXV][2] = {{0}};

    if (g->n < 1 || g->n > MAXV || postOffice < 0 || postOffice >= g->n) {
        printf("无解：图规模或邮局位置无效。\n");
        return -1;
    }

    for (int i = 0; i < g->m; ++i) {
        total += g->edges[i].w;
        degree[g->edges[i].u]++;
        degree[g->edges[i].v]++;
    }
    for (int i = 0; i < g->n; ++i) {
        if (degree[i] % 2 == 1) {
            odd[oddCount++] = i;
        }
    }

    floyd(g, dist);
    /* 即使所有度数为偶数，街道也可能位于不连通的分量中。 */
    for (int i = 0; i < g->n; ++i) {
        if (degree[i] > 0 && dist[postOffice][i] == INF) {
            printf("无解：存在从邮局不可达的街道。\n");
            return -1;
        }
    }

    printf("邮局位置：%d\n", postOffice);
    printf("所有街道总长度：%lld\n", total);
    printf("奇度顶点：");
    for (int i = 0; i < oddCount; ++i) {
        printf("%d ", odd[i]);
    }
    printf("\n");

    if (oddCount == 0) {
        printf("该辖区已存在欧拉回路，最短投递路线长度为：%lld\n", total);
        return total;
    }

    long long addCost = matchOdd(odd, oddCount, used, dist, pair, temp, 0);
    printf("需要重复经过的最短补边配对：");
    for (int i = 0; i < oddCount / 2; ++i) {
        printf("(%d,%d,补长%lld) ", pair[i][0], pair[i][1], dist[pair[i][0]][pair[i][1]]);
    }
    printf("\n");
    printf("最短投递路线长度：%lld\n", total + addCost);
    printf("路线说明：从邮局%d出发，按欧拉化后的多重图经过每条街至少一次后回到邮局。\n",
           postOffice);
    return total + addCost;
}

/* 函数功能：输出辖区街道。 */
void printGraph(const PostGraph *g)
{
    printf("辖区街道：\n");
    for (int i = 0; i < g->m; ++i) {
        printf("%d -- %d 长度 %d\n", g->edges[i].u, g->edges[i].v, g->edges[i].w);
    }
}

/* 函数功能：运行测试用例。 */
int main(void)
{
    PostGraph g1;
    initGraph(&g1, 6);
    addStreet(&g1, 0, 1, 4);
    addStreet(&g1, 0, 2, 3);
    addStreet(&g1, 1, 2, 2);
    addStreet(&g1, 1, 3, 5);
    addStreet(&g1, 2, 4, 6);
    addStreet(&g1, 3, 4, 1);
    addStreet(&g1, 3, 5, 7);
    addStreet(&g1, 4, 5, 2);

    printf("测试用例1：一般辖区\n");
    printGraph(&g1);
    solveChinesePostman(&g1, 0);

    PostGraph g2;
    initGraph(&g2, 4);
    addStreet(&g2, 0, 1, 2);
    addStreet(&g2, 1, 2, 3);
    addStreet(&g2, 2, 3, 4);
    addStreet(&g2, 3, 0, 5);

    printf("\n测试用例2：已存在欧拉回路的辖区\n");
    printGraph(&g2);
    solveChinesePostman(&g2, 0);
    return 0;
}
