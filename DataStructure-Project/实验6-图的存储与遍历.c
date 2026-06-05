#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_VERTEX_NUM 20
#define INF 100000000

typedef char VertexType[20];

typedef struct {
    VertexType vexs[MAX_VERTEX_NUM];
    int arcs[MAX_VERTEX_NUM][MAX_VERTEX_NUM];
    int vexnum;
    int arcnum;
} MGraph;

typedef struct ArcNode {
    int adjvex;
    int weight;
    struct ArcNode *next;
} ArcNode;

typedef struct {
    VertexType data;
    ArcNode *firstarc;
} VNode;

typedef struct {
    VNode vertices[MAX_VERTEX_NUM];
    int vexnum;
    int arcnum;
} ALGraph;

typedef struct {
    int u;
    int v;
    int weight;
} Edge;

/* 函数功能：在邻接矩阵图中定位顶点。 */
int locateVexMatrix(const MGraph *g, const char *v)
{
    for (int i = 0; i < g->vexnum; ++i) {
        if (strcmp(g->vexs[i], v) == 0) {
            return i;
        }
    }
    return -1;
}

/* 函数功能：在邻接表图中定位顶点。 */
int locateVexList(const ALGraph *g, const char *v)
{
    for (int i = 0; i < g->vexnum; ++i) {
        if (strcmp(g->vertices[i].data, v) == 0) {
            return i;
        }
    }
    return -1;
}

/* 函数功能：建立无向网的邻接矩阵。 */
void createMatrixGraph(MGraph *g, const char *vexs[], int vexnum,
                       const Edge edges[], int arcnum)
{
    g->vexnum = vexnum;
    g->arcnum = arcnum;
    for (int i = 0; i < vexnum; ++i) {
        strcpy(g->vexs[i], vexs[i]);
        for (int j = 0; j < vexnum; ++j) {
            g->arcs[i][j] = (i == j) ? 0 : INF;
        }
    }
    for (int k = 0; k < arcnum; ++k) {
        int u = edges[k].u;
        int v = edges[k].v;
        g->arcs[u][v] = edges[k].weight;
        g->arcs[v][u] = edges[k].weight;
    }
}

/* 函数功能：建立无向网的邻接表。 */
void createListGraph(ALGraph *g, const char *vexs[], int vexnum,
                     const Edge edges[], int arcnum)
{
    g->vexnum = vexnum;
    g->arcnum = arcnum;
    for (int i = 0; i < vexnum; ++i) {
        strcpy(g->vertices[i].data, vexs[i]);
        g->vertices[i].firstarc = NULL;
    }
    for (int k = 0; k < arcnum; ++k) {
        int u = edges[k].u;
        int v = edges[k].v;
        ArcNode *uv = (ArcNode *)malloc(sizeof(ArcNode));
        ArcNode *vu = (ArcNode *)malloc(sizeof(ArcNode));
        if (uv == NULL || vu == NULL) {
            printf("内存分配失败\n");
            exit(1);
        }
        uv->adjvex = v;
        uv->weight = edges[k].weight;
        uv->next = g->vertices[u].firstarc;
        g->vertices[u].firstarc = uv;

        vu->adjvex = u;
        vu->weight = edges[k].weight;
        vu->next = g->vertices[v].firstarc;
        g->vertices[v].firstarc = vu;
    }
}

/* 函数功能：释放邻接表存储空间。 */
void destroyListGraph(ALGraph *g)
{
    for (int i = 0; i < g->vexnum; ++i) {
        ArcNode *p = g->vertices[i].firstarc;
        while (p != NULL) {
            ArcNode *next = p->next;
            free(p);
            p = next;
        }
        g->vertices[i].firstarc = NULL;
    }
}

/* 函数功能：返回邻接矩阵中顶点i的第一个邻接点。 */
int firstAdjVexMatrix(const MGraph *g, int i)
{
    if (i < 0 || i >= g->vexnum) {
        return -1;
    }
    for (int j = 0; j < g->vexnum; ++j) {
        if (g->arcs[i][j] != 0 && g->arcs[i][j] < INF) {
            return j;
        }
    }
    return -1;
}

/* 函数功能：返回邻接矩阵中顶点i相对于j的下一个邻接点。 */
int nextAdjVexMatrix(const MGraph *g, int i, int j)
{
    if (i < 0 || i >= g->vexnum || j < -1 || j >= g->vexnum) {
        return -1;
    }
    for (int k = j + 1; k < g->vexnum; ++k) {
        if (g->arcs[i][k] != 0 && g->arcs[i][k] < INF) {
            return k;
        }
    }
    return -1;
}

/* 函数功能：返回邻接表中顶点i的第一个邻接点。 */
int firstAdjVexList(const ALGraph *g, int i)
{
    if (i < 0 || i >= g->vexnum || g->vertices[i].firstarc == NULL) {
        return -1;
    }
    return g->vertices[i].firstarc->adjvex;
}

/* 函数功能：返回邻接表中顶点i相对于j的下一个邻接点。 */
int nextAdjVexList(const ALGraph *g, int i, int j)
{
    if (i < 0 || i >= g->vexnum) {
        return -1;
    }
    for (ArcNode *p = g->vertices[i].firstarc; p != NULL; p = p->next) {
        if (p->adjvex == j) {
            return p->next == NULL ? -1 : p->next->adjvex;
        }
    }
    return -1;
}

/* 函数功能：邻接矩阵深度优先遍历的递归过程。 */
void dfsMatrixVisit(const MGraph *g, int v, int visited[])
{
    visited[v] = 1;
    printf("%s ", g->vexs[v]);
    for (int w = firstAdjVexMatrix(g, v); w != -1; w = nextAdjVexMatrix(g, v, w)) {
        if (!visited[w]) {
            dfsMatrixVisit(g, w, visited);
        }
    }
}

/* 函数功能：邻接矩阵深度优先遍历。 */
void dfsMatrix(const MGraph *g)
{
    int visited[MAX_VERTEX_NUM] = {0};
    for (int i = 0; i < g->vexnum; ++i) {
        if (!visited[i]) {
            dfsMatrixVisit(g, i, visited);
        }
    }
    printf("\n");
}

/* 函数功能：邻接表深度优先遍历的递归过程。 */
void dfsListVisit(const ALGraph *g, int v, int visited[])
{
    visited[v] = 1;
    printf("%s ", g->vertices[v].data);
    for (ArcNode *p = g->vertices[v].firstarc; p != NULL; p = p->next) {
        if (!visited[p->adjvex]) {
            dfsListVisit(g, p->adjvex, visited);
        }
    }
}

/* 函数功能：邻接表深度优先遍历。 */
void dfsList(const ALGraph *g)
{
    int visited[MAX_VERTEX_NUM] = {0};
    for (int i = 0; i < g->vexnum; ++i) {
        if (!visited[i]) {
            dfsListVisit(g, i, visited);
        }
    }
    printf("\n");
}

/* 函数功能：邻接矩阵广度优先遍历。 */
void bfsMatrix(const MGraph *g)
{
    int visited[MAX_VERTEX_NUM] = {0};
    int queue[MAX_VERTEX_NUM];
    int front = 0;
    int rear = 0;

    for (int i = 0; i < g->vexnum; ++i) {
        if (!visited[i]) {
            visited[i] = 1;
            printf("%s ", g->vexs[i]);
            queue[rear++] = i;
            while (front < rear) {
                int v = queue[front++];
                for (int w = firstAdjVexMatrix(g, v); w != -1; w = nextAdjVexMatrix(g, v, w)) {
                    if (!visited[w]) {
                        visited[w] = 1;
                        printf("%s ", g->vexs[w]);
                        queue[rear++] = w;
                    }
                }
            }
        }
    }
    printf("\n");
}

/* 函数功能：邻接表广度优先遍历。 */
void bfsList(const ALGraph *g)
{
    int visited[MAX_VERTEX_NUM] = {0};
    int queue[MAX_VERTEX_NUM];
    int front = 0;
    int rear = 0;

    for (int i = 0; i < g->vexnum; ++i) {
        if (!visited[i]) {
            visited[i] = 1;
            printf("%s ", g->vertices[i].data);
            queue[rear++] = i;
            while (front < rear) {
                int v = queue[front++];
                for (ArcNode *p = g->vertices[v].firstarc; p != NULL; p = p->next) {
                    if (!visited[p->adjvex]) {
                        visited[p->adjvex] = 1;
                        printf("%s ", g->vertices[p->adjvex].data);
                        queue[rear++] = p->adjvex;
                    }
                }
            }
        }
    }
    printf("\n");
}

/* 函数功能：输出邻接矩阵。 */
void printMatrix(const MGraph *g)
{
    printf("邻接矩阵：\n      ");
    for (int i = 0; i < g->vexnum; ++i) {
        printf("%5s", g->vexs[i]);
    }
    printf("\n");
    for (int i = 0; i < g->vexnum; ++i) {
        printf("%5s ", g->vexs[i]);
        for (int j = 0; j < g->vexnum; ++j) {
            if (g->arcs[i][j] >= INF) {
                printf("%5s", "INF");
            } else {
                printf("%5d", g->arcs[i][j]);
            }
        }
        printf("\n");
    }
}

/* 函数功能：输出邻接表。 */
void printList(const ALGraph *g)
{
    printf("邻接表：\n");
    for (int i = 0; i < g->vexnum; ++i) {
        printf("%s:", g->vertices[i].data);
        for (ArcNode *p = g->vertices[i].firstarc; p != NULL; p = p->next) {
            printf(" -> %s(%d)", g->vertices[p->adjvex].data, p->weight);
        }
        printf("\n");
    }
}

/* 函数功能：程序入口，建立无向网并测试全部基本运算。 */
int main(void)
{
    const char *vexs[] = {"A", "B", "C", "D", "E", "F"};
    Edge edges[] = {
        {0, 1, 5}, {0, 2, 2}, {1, 3, 7}, {2, 3, 4},
        {2, 4, 8}, {3, 5, 3}, {4, 5, 6}
    };
    MGraph matrixGraph;
    ALGraph listGraph;
    int vexnum = 6;
    int arcnum = (int)(sizeof(edges) / sizeof(edges[0]));

    createMatrixGraph(&matrixGraph, vexs, vexnum, edges, arcnum);
    createListGraph(&listGraph, vexs, vexnum, edges, arcnum);

    printMatrix(&matrixGraph);
    printf("\n");
    printList(&listGraph);

    printf("\n顶点D在邻接矩阵中的位置：%d\n", locateVexMatrix(&matrixGraph, "D"));
    printf("顶点Z在邻接表中的位置：%d\n", locateVexList(&listGraph, "Z"));

    int a = locateVexMatrix(&matrixGraph, "A");
    int first = firstAdjVexMatrix(&matrixGraph, a);
    int next = nextAdjVexMatrix(&matrixGraph, a, first);
    printf("邻接矩阵：A的第一个邻接点=%s，下一个邻接点=%s\n",
           matrixGraph.vexs[first], matrixGraph.vexs[next]);

    int d = locateVexList(&listGraph, "D");
    int firstList = firstAdjVexList(&listGraph, d);
    int nextList = nextAdjVexList(&listGraph, d, firstList);
    printf("邻接表：D的第一个邻接点=%s，下一个邻接点=%s\n",
           listGraph.vertices[firstList].data, listGraph.vertices[nextList].data);

    printf("\n邻接矩阵DFS：");
    dfsMatrix(&matrixGraph);
    printf("邻接矩阵BFS：");
    bfsMatrix(&matrixGraph);
    printf("邻接表DFS：");
    dfsList(&listGraph);
    printf("邻接表BFS：");
    bfsList(&listGraph);

    destroyListGraph(&listGraph);
    return 0;
}
