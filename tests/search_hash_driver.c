/* Exercise the real implementation without changing its demonstration entry. */
#define main search_demo_main
#include "../DataStructure-Project/实验7-查找算法验证.c"
#undef main
#include <limits.h>
#include <string.h>

int main(int argc, char **argv)
{
    if (argc == 2 && strcmp(argv[1], "demo") == 0) {
        return search_demo_main();
    }
    printf("%d %d\n", INT_MIN, INT_MAX);
    HashTable table;
    initHashTable(&table);
    char op;
    int key;
    while (scanf(" %c", &op) == 1) {
        if (op == 'P') {
            printHashTable(&table);
        } else if (op == 'R') {
            initHashTable(&table);
        } else {
            if (scanf("%d", &key) != 1) return 2;
            if (op == 'I') {
                printf("I %d\n", insertHash(&table, key));
            } else if (op == 'S') {
                int count = -99;
                int pos = searchHash(&table, key, &count);
                printf("S %d %d\n", pos, count);
            } else if (op == 'H') {
                printf("H %d\n", hash(key));
            } else {
                return 2;
            }
        }
    }
    return 0;
}
