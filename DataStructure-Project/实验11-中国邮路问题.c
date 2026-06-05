#include <stdio.h>

#define MAXV 12
#define INF 100000000

typedef struct {
    int u;
    int v;
    int w;
} Edge;

typedef struct {
    int n;
    int m;
    int graph[MAXV][MAXV];
    Edge edges[40];
} PostGraph;

/* 函数功能：初始化邮路图。 */
void initGraph(PostGraph *g, int n)
{
    g->n = n;
    g->m = 0;
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            g->graph[i][j] = (i == j) ? 0 : INF;
        }
    }
}

/* 函数功能：添加无向街道。 */
void addStreet(PostGraph *g, int u, int v, int w)
{
    g->graph[u][v] = w;
    g->graph[v][u] = w;
    g->edges[g->m++] = (Edge){u, v, w};
}

/* 函数功能：Floyd算法求任意两点最短路径。 */
void floyd(const PostGraph *g, int dist[][MAXV])
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
int matchOdd(int odd[], int oddCount, int used[], int dist[][MAXV], int pair[][2], int temp[][2], int depth)
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

    int best = INF;
    used[first] = 1;
    for (int j = first + 1; j < oddCount; ++j) {
        if (!used[j]) {
            used[j] = 1;
            temp[depth][0] = odd[first];
            temp[depth][1] = odd[j];
            int subPair[MAXV][2];
            int subCost = matchOdd(odd, oddCount, used, dist, subPair, temp, depth + 1);
            int cost = dist[odd[first]][odd[j]] + subCost;
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

/* 函数功能：求中国邮路问题的最短闭合投递路线长度。 */
void solveChinesePostman(PostGraph *g, int postOffice)
{
    int degree[MAXV] = {0};
    int odd[MAXV];
    int oddCount = 0;
    int total = 0;
    int dist[MAXV][MAXV];
    int used[MAXV] = {0};
    int pair[MAXV][2] = {{0}};
    int temp[MAXV][2] = {{0}};

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

    printf("邮局位置：%d\n", postOffice);
    printf("所有街道总长度：%d\n", total);
    printf("奇度顶点：");
    for (int i = 0; i < oddCount; ++i) {
        printf("%d ", odd[i]);
    }
    printf("\n");

    if (oddCount == 0) {
        printf("该辖区已存在欧拉回路，最短投递路线长度为：%d\n", total);
        return;
    }

    int addCost = matchOdd(odd, oddCount, used, dist, pair, temp, 0);
    printf("需要重复经过的最短补边配对：");
    for (int i = 0; i < oddCount / 2; ++i) {
        printf("(%d,%d,补长%d) ", pair[i][0], pair[i][1], dist[pair[i][0]][pair[i][1]]);
    }
    printf("\n");
    printf("最短投递路线长度：%d\n", total + addCost);
    printf("路线说明：从邮局%d出发，按欧拉化后的多重图经过每条街至少一次后回到邮局。\n",
           postOffice);
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
