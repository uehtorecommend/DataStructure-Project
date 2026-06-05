#include <stdio.h>
#include <string.h>

#define MAX_SCENE 10
#define INF 100000000

typedef struct {
    char name[30];
    char intro[200];
} ScenicSpot;

typedef struct {
    ScenicSpot spots[MAX_SCENE];
    int dist[MAX_SCENE][MAX_SCENE];
    int count;
} CampusMap;

/* 函数功能：初始化校园图。 */
void initCampus(CampusMap *map)
{
    map->count = 6;
    strcpy(map->spots[0].name, "正门");
    strcpy(map->spots[0].intro, "校园主入口，适合作为参观起点。");
    strcpy(map->spots[1].name, "图书馆");
    strcpy(map->spots[1].intro, "学习资源中心，馆藏丰富。");
    strcpy(map->spots[2].name, "启航广场");
    strcpy(map->spots[2].intro, "校园活动与休憩空间。");
    strcpy(map->spots[3].name, "教学楼");
    strcpy(map->spots[3].intro, "主要教学场所。");
    strcpy(map->spots[4].name, "体育馆");
    strcpy(map->spots[4].intro, "体育课程与大型活动场馆。");
    strcpy(map->spots[5].name, "宿舍区");
    strcpy(map->spots[5].intro, "学生生活区。");

    for (int i = 0; i < map->count; ++i) {
        for (int j = 0; j < map->count; ++j) {
            map->dist[i][j] = (i == j) ? 0 : INF;
        }
    }
}

/* 函数功能：添加无向道路。 */
void addRoad(CampusMap *map, int a, int b, int length)
{
    map->dist[a][b] = length;
    map->dist[b][a] = length;
}

/* 函数功能：按名称定位景点。 */
int locateSpot(const CampusMap *map, const char *name)
{
    for (int i = 0; i < map->count; ++i) {
        if (strcmp(map->spots[i].name, name) == 0) {
            return i;
        }
    }
    return -1;
}

/* 函数功能：查询景点信息。 */
void querySpot(const CampusMap *map, const char *name)
{
    int idx = locateSpot(map, name);
    if (idx == -1) {
        printf("未找到景点：%s\n", name);
        return;
    }
    printf("景点：%s\n介绍：%s\n", map->spots[idx].name, map->spots[idx].intro);
}

/* 函数功能：查询邻接景点。 */
void queryAdjacent(const CampusMap *map, const char *name)
{
    int idx = locateSpot(map, name);
    if (idx == -1) {
        printf("未找到景点：%s\n", name);
        return;
    }
    printf("%s 的邻接景点：", name);
    for (int i = 0; i < map->count; ++i) {
        if (map->dist[idx][i] > 0 && map->dist[idx][i] < INF) {
            printf("%s(%dm) ", map->spots[i].name, map->dist[idx][i]);
        }
    }
    printf("\n");
}

/* 函数功能：Dijkstra求单源最短路径。 */
void dijkstra(const CampusMap *map, int start, int dist[], int prev[])
{
    int visited[MAX_SCENE] = {0};
    for (int i = 0; i < map->count; ++i) {
        dist[i] = map->dist[start][i];
        prev[i] = (dist[i] < INF && i != start) ? start : -1;
    }
    visited[start] = 1;
    dist[start] = 0;

    for (int k = 1; k < map->count; ++k) {
        int u = -1;
        int minDist = INF;
        for (int i = 0; i < map->count; ++i) {
            if (!visited[i] && dist[i] < minDist) {
                minDist = dist[i];
                u = i;
            }
        }
        if (u == -1) {
            break;
        }
        visited[u] = 1;
        for (int v = 0; v < map->count; ++v) {
            if (!visited[v] && map->dist[u][v] < INF &&
                dist[u] + map->dist[u][v] < dist[v]) {
                dist[v] = dist[u] + map->dist[u][v];
                prev[v] = u;
            }
        }
    }
}

/* 函数功能：递归输出路径。 */
void printPath(const CampusMap *map, int prev[], int v)
{
    if (prev[v] != -1) {
        printPath(map, prev, prev[v]);
        printf(" -> ");
    }
    printf("%s", map->spots[v].name);
}

/* 函数功能：查询到单个景点的最佳路线。 */
void bestRouteTo(const CampusMap *map, const char *from, const char *to)
{
    int s = locateSpot(map, from);
    int t = locateSpot(map, to);
    int dist[MAX_SCENE];
    int prev[MAX_SCENE];
    if (s == -1 || t == -1) {
        printf("起点或终点不存在。\n");
        return;
    }
    dijkstra(map, s, dist, prev);
    printf("%s 到 %s 的最佳路线：", from, to);
    printPath(map, prev, t);
    printf("，距离 %dm\n", dist[t]);
}

/* 函数功能：查询到所有景点的最佳路线。 */
void bestRoutesToAll(const CampusMap *map, const char *from)
{
    int s = locateSpot(map, from);
    int dist[MAX_SCENE];
    int prev[MAX_SCENE];
    if (s == -1) {
        printf("起点不存在。\n");
        return;
    }
    dijkstra(map, s, dist, prev);
    printf("从%s到所有景点的最佳路线：\n", from);
    for (int i = 0; i < map->count; ++i) {
        if (i == s) {
            continue;
        }
        printf("  到%s：", map->spots[i].name);
        printPath(map, prev, i);
        printf("，距离 %dm\n", dist[i]);
    }
}

/* 函数功能：修改景点介绍。 */
void modifySpot(CampusMap *map, const char *name, const char *intro)
{
    int idx = locateSpot(map, name);
    if (idx == -1) {
        printf("未找到景点：%s\n", name);
        return;
    }
    strcpy(map->spots[idx].intro, intro);
    printf("已修改%s的景点信息。\n", name);
}

/* 函数功能：程序入口，演示校园导游功能。 */
int main(void)
{
    CampusMap map;
    initCampus(&map);
    addRoad(&map, 0, 1, 300);
    addRoad(&map, 0, 2, 450);
    addRoad(&map, 1, 2, 200);
    addRoad(&map, 1, 3, 350);
    addRoad(&map, 2, 4, 500);
    addRoad(&map, 3, 5, 400);
    addRoad(&map, 4, 5, 250);
    addRoad(&map, 3, 4, 300);

    querySpot(&map, "图书馆");
    querySpot(&map, "不存在景点");
    queryAdjacent(&map, "图书馆");
    bestRouteTo(&map, "正门", "宿舍区");
    bestRoutesToAll(&map, "正门");
    modifySpot(&map, "体育馆", "体育课程、比赛和文体活动中心。");
    querySpot(&map, "体育馆");

    return 0;
}
