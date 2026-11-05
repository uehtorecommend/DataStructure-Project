/* Exercise the public queries without changing the standalone demo entry point. */
#define main campus_demo_main
#include "../DataStructure-Project/实验10-校园导游.c"
#undef main

int main(int argc, char **argv)
{
    CampusMap map;
    int edges, start, target;
    (void)argv;
    if (argc > 1) {
        return campus_demo_main();
    }
    initCampus(&map);
    if (scanf("%d %d %d", &edges, &start, &target) != 3 ||
        edges < 0 || start < -1 || start >= map.count ||
        target < -2 || target >= map.count) {
        return 2;
    }
    for (int i = 0; i < edges; ++i) {
        int a, b, length;
        if (scanf("%d %d %d", &a, &b, &length) != 3 ||
            a < 0 || a >= map.count || b < 0 || b >= map.count ||
            length < 0 || length >= INF) {
            return 2;
        }
        addRoad(&map, a, b, length);
    }
    const char *from = start == -1 ? "不存在景点" : map.spots[start].name;
    if (target == -2) {
        bestRoutesToAll(&map, from);
    } else {
        const char *to = target == -1 ? "不存在景点" : map.spots[target].name;
        bestRouteTo(&map, from, to);
    }
    return 0;
}
