#include <stdio.h>
#include <stdlib.h>

#define HASH_SIZE 13
#define EMPTY_KEY -1

typedef struct BSTNode {
    int key;
    struct BSTNode *left;
    struct BSTNode *right;
} BSTNode;

typedef struct {
    int data[HASH_SIZE];
} HashTable;

/* 函数功能：顺序查找，返回元素下标，失败返回-1。 */
int sequentialSearch(const int data[], int n, int key, int *compareCount)
{
    *compareCount = 0;
    for (int i = 0; i < n; ++i) {
        ++(*compareCount);
        if (data[i] == key) {
            return i;
        }
    }
    return -1;
}

/* 函数功能：折半查找，数组必须按升序排列。 */
int binarySearch(const int data[], int n, int key, int *compareCount)
{
    int low = 0;
    int high = n - 1;
    *compareCount = 0;
    while (low <= high) {
        int mid = low + (high - low) / 2;
        ++(*compareCount);
        if (data[mid] == key) {
            return mid;
        }
        if (data[mid] < key) {
            low = mid + 1;
        } else {
            high = mid - 1;
        }
    }
    return -1;
}

/* 函数功能：创建二叉排序树结点。 */
BSTNode *createBSTNode(int key)
{
    BSTNode *node = (BSTNode *)malloc(sizeof(BSTNode));
    if (node != NULL) {
        node->key = key;
        node->left = NULL;
        node->right = NULL;
    }
    return node;
}

/* 函数功能：向二叉排序树插入关键字，重复关键字不插入。 */
BSTNode *insertBST(BSTNode *root, int key)
{
    if (root == NULL) {
        return createBSTNode(key);
    }
    if (key < root->key) {
        root->left = insertBST(root->left, key);
    } else if (key > root->key) {
        root->right = insertBST(root->right, key);
    }
    return root;
}

/* 函数功能：在二叉排序树中查找关键字。 */
BSTNode *searchBST(BSTNode *root, int key, int *compareCount)
{
    while (root != NULL) {
        ++(*compareCount);
        if (key == root->key) {
            return root;
        }
        root = key < root->key ? root->left : root->right;
    }
    return NULL;
}

/* 函数功能：中序遍历二叉排序树。 */
void inorderBST(BSTNode *root)
{
    if (root == NULL) {
        return;
    }
    inorderBST(root->left);
    printf("%d ", root->key);
    inorderBST(root->right);
}

/* 函数功能：释放二叉排序树。 */
void destroyBST(BSTNode *root)
{
    if (root == NULL) {
        return;
    }
    destroyBST(root->left);
    destroyBST(root->right);
    free(root);
}

/* 函数功能：初始化哈希表。 */
void initHashTable(HashTable *table)
{
    for (int i = 0; i < HASH_SIZE; ++i) {
        table->data[i] = EMPTY_KEY;
    }
}

/* 函数功能：哈希函数。 */
int hash(int key)
{
    return key % HASH_SIZE;
}

/* 函数功能：用线性探测法插入哈希表。 */
int insertHash(HashTable *table, int key)
{
    int address = hash(key);
    for (int i = 0; i < HASH_SIZE; ++i) {
        int pos = (address + i) % HASH_SIZE;
        if (table->data[pos] == EMPTY_KEY || table->data[pos] == key) {
            table->data[pos] = key;
            return pos;
        }
    }
    return -1;
}

/* 函数功能：在线性探测哈希表中查找关键字。 */
int searchHash(const HashTable *table, int key, int *compareCount)
{
    int address = hash(key);
    *compareCount = 0;
    for (int i = 0; i < HASH_SIZE; ++i) {
        int pos = (address + i) % HASH_SIZE;
        ++(*compareCount);
        if (table->data[pos] == key) {
            return pos;
        }
        if (table->data[pos] == EMPTY_KEY) {
            return -1;
        }
    }
    return -1;
}

/* 函数功能：输出哈希表。 */
void printHashTable(const HashTable *table)
{
    printf("哈希表地址: ");
    for (int i = 0; i < HASH_SIZE; ++i) {
        printf("%3d", i);
    }
    printf("\n哈希表数据: ");
    for (int i = 0; i < HASH_SIZE; ++i) {
        if (table->data[i] == EMPTY_KEY) {
            printf("%3s", "--");
        } else {
            printf("%3d", table->data[i]);
        }
    }
    printf("\n");
}

/* 函数功能：打印查找结果。 */
void printSearchResult(const char *name, int key, int pos, int compareCount)
{
    if (pos >= 0) {
        printf("%s 查找 %d 成功，位置=%d，比较次数=%d\n", name, key, pos, compareCount);
    } else {
        printf("%s 查找 %d 失败，比较次数=%d\n", name, key, compareCount);
    }
}

/* 函数功能：程序入口，验证四类查找结构。 */
int main(void)
{
    int unordered[] = {45, 12, 78, 23, 56, 89, 34, 67, 90, 11};
    int ordered[] = {11, 12, 23, 34, 45, 56, 67, 78, 89, 90};
    int n = (int)(sizeof(unordered) / sizeof(unordered[0]));
    int compareCount = 0;
    int pos = -1;

    printf("一、顺序查找验证\n");
    pos = sequentialSearch(unordered, n, 56, &compareCount);
    printSearchResult("顺序查找", 56, pos, compareCount);
    pos = sequentialSearch(unordered, n, 100, &compareCount);
    printSearchResult("顺序查找", 100, pos, compareCount);
    pos = sequentialSearch(unordered, n, 45, &compareCount);
    printSearchResult("顺序查找", 45, pos, compareCount);

    printf("\n二、折半查找验证\n");
    pos = binarySearch(ordered, n, 56, &compareCount);
    printSearchResult("折半查找", 56, pos, compareCount);
    pos = binarySearch(ordered, n, 10, &compareCount);
    printSearchResult("折半查找", 10, pos, compareCount);
    pos = binarySearch(ordered, n, 90, &compareCount);
    printSearchResult("折半查找", 90, pos, compareCount);

    printf("\n三、二叉排序树验证\n");
    BSTNode *root = NULL;
    for (int i = 0; i < n; ++i) {
        root = insertBST(root, unordered[i]);
    }
    printf("二叉排序树中序遍历：");
    inorderBST(root);
    printf("\n");
    compareCount = 0;
    pos = searchBST(root, 34, &compareCount) ? 1 : -1;
    printSearchResult("二叉排序树", 34, pos, compareCount);
    compareCount = 0;
    pos = searchBST(root, 99, &compareCount) ? 1 : -1;
    printSearchResult("二叉排序树", 99, pos, compareCount);

    printf("\n四、哈希表验证\n");
    HashTable table;
    initHashTable(&table);
    for (int i = 0; i < n; ++i) {
        insertHash(&table, unordered[i]);
    }
    printHashTable(&table);
    pos = searchHash(&table, 78, &compareCount);
    printSearchResult("哈希表", 78, pos, compareCount);
    pos = searchHash(&table, 24, &compareCount);
    printSearchResult("哈希表", 24, pos, compareCount);
    pos = searchHash(&table, 90, &compareCount);
    printSearchResult("哈希表", 90, pos, compareCount);

    destroyBST(root);
    return 0;
}
