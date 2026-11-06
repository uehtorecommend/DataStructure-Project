/* Call the actual implementation while preserving its standalone demo. */
#define main graph_demo_main
#include "../DataStructure-Project/实验6-图的存储与遍历.c"
#undef main

int main(int argc, char **argv)
{
    (void)argv;
    if (argc > 1) {
        return graph_demo_main();
    }
    const char *names[MAX_VERTEX_NUM] = {
        "A", "B", "C", "D", "E", "F", "G", "H", "I", "J",
        "K", "L", "M", "N", "O", "P", "Q", "R", "S", "T"
    };
    Edge edges[MAX_VERTEX_NUM * MAX_VERTEX_NUM];
    int n, m;
    if (scanf("%d %d", &n, &m) != 2 || n < 0 || n > MAX_VERTEX_NUM ||
        m < 0 || m > MAX_VERTEX_NUM * MAX_VERTEX_NUM) {
        return 2;
    }
    for (int e = 0; e < m; ++e) {
        if (scanf("%d %d %d", &edges[e].u, &edges[e].v, &edges[e].weight) != 3 ||
            edges[e].u < 0 || edges[e].u >= n || edges[e].v < 0 || edges[e].v >= n ||
            edges[e].u == edges[e].v || edges[e].weight >= INF) {
            return 2;
        }
    }
    MGraph matrix;
    ALGraph list;
    createMatrixGraph(&matrix, names, n, edges, m);
    createListGraph(&list, names, n, edges, m);
    for (int i = 0; i < n; ++i) {
        printf("M%d:", i);
        for (int j = firstAdjVexMatrix(&matrix, i); j != -1;
             j = nextAdjVexMatrix(&matrix, i, j)) {
            printf(" %d", j);
        }
        printf("\nL%d:", i);
        for (int j = firstAdjVexList(&list, i); j != -1;
             j = nextAdjVexList(&list, i, j)) {
            printf(" %d", j);
        }
        printf("\n");
    }
    printf("MD:"); dfsMatrix(&matrix);
    printf("MB:"); bfsMatrix(&matrix);
    printf("LD:"); dfsList(&list);
    printf("LB:"); bfsList(&list);
    /* Directly exercise DFS from each root: forest traversals alone can hide missing edges. */
    for (int i = 0; i < n; ++i) {
        int matrixVisited[MAX_VERTEX_NUM] = {0};
        int listVisited[MAX_VERTEX_NUM] = {0};
        printf("MR%d:", i); dfsMatrixVisit(&matrix, i, matrixVisited); printf("\n");
        printf("LR%d:", i); dfsListVisit(&list, i, listVisited); printf("\n");
    }
    printf("bounds: %d %d %d %d %d %d\n",
           firstAdjVexMatrix(&matrix, -1), firstAdjVexMatrix(&matrix, n),
           nextAdjVexMatrix(&matrix, -1, -1), nextAdjVexMatrix(&matrix, n, -1),
           nextAdjVexMatrix(&matrix, 0, -2), nextAdjVexMatrix(&matrix, 0, n));
    destroyListGraph(&list);
    return 0;
}
