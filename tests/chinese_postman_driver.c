/* Compile the real implementation, keeping its original demonstration callable. */
#include <assert.h>
#include <string.h>
#define main postman_demo_main
#include "../DataStructure-Project/实验11-中国邮路问题.c"
#undef main

static void checkInputContracts(void)
{
    PostGraph g = {0};
    assert(!initGraph(&g, 0));
    assert(!addStreet(&g, 0, 0, 1));
    assert(solveChinesePostman(&g, 0) == -1);
    assert(!initGraph(&g, MAXV + 1));
    assert(!initGraph(&g, -1));
    assert(initGraph(&g, 2));
    PostGraph before = g;
    assert(!addStreet(&g, -1, 1, 1));
    assert(!addStreet(&g, 0, 2, 1));
    assert(!addStreet(&g, 2, 0, 1));
    assert(!addStreet(&g, 0, -1, 1));
    assert(!addStreet(&g, 0, 1, -1));
    assert(memcmp(&g, &before, sizeof g) == 0);
    assert(solveChinesePostman(&g, -1) == -1);
    assert(solveChinesePostman(&g, 2) == -1);
    for (int i = 0; i < MAXE; ++i) {
        assert(addStreet(&g, 0, 1, 10));
    }
    before = g;
    /* Even a rejected lighter edge must not change the distance matrix. */
    assert(!addStreet(&g, 0, 1, 0));
    assert(memcmp(&g, &before, sizeof g) == 0);
}

int main(void)
{
    checkInputContracts();
    int n, m, office;
    while (scanf("%d%d%d", &n, &m, &office) == 3) {
        PostGraph g;
        assert(initGraph(&g, n));
        for (int i = 0; i < m; ++i) {
            int u, v, w;
            assert(scanf("%d%d%d", &u, &v, &w) == 3);
            assert(addStreet(&g, u, v, w));
        }
        puts("CASE");
        long long result = solveChinesePostman(&g, office);
        printf("RESULT %lld\n", result);
    }
    return 0;
}
