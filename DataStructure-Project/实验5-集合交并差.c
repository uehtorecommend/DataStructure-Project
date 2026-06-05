#include <stdio.h>
#include <stdlib.h>

typedef int ElemType;

typedef struct SetNode {
    ElemType data;
    struct SetNode *next;
} SetNode;

typedef SetNode *SetList;

/* 函数功能：创建带头结点的空集合链表。 */
SetList createEmptySet(void)
{
    SetList head = (SetList)malloc(sizeof(SetNode));
    if (head != NULL) {
        head->next = NULL;
    }
    return head;
}

/* 函数功能：释放集合链表。 */
void destroySet(SetList set)
{
    while (set != NULL) {
        SetNode *next = set->next;
        free(set);
        set = next;
    }
}

/* 函数功能：向有序集合插入元素，自动去重。 */
int insertOrderedUnique(SetList set, ElemType value)
{
    SetNode *pre = set;
    SetNode *cur = set->next;
    while (cur != NULL && cur->data < value) {
        pre = cur;
        cur = cur->next;
    }
    if (cur != NULL && cur->data == value) {
        return 1;
    }
    SetNode *node = (SetNode *)malloc(sizeof(SetNode));
    if (node == NULL) {
        return 0;
    }
    node->data = value;
    node->next = cur;
    pre->next = node;
    return 1;
}

/* 函数功能：由数组建立有序单链表集合。 */
SetList buildSet(const ElemType values[], int n)
{
    SetList set = createEmptySet();
    if (set == NULL) {
        return NULL;
    }
    for (int i = 0; i < n; ++i) {
        if (!insertOrderedUnique(set, values[i])) {
            destroySet(set);
            return NULL;
        }
    }
    return set;
}

/* 函数功能：输出集合。 */
void printSet(const char *name, SetList set)
{
    printf("%s = {", name);
    for (SetNode *p = set->next; p != NULL; p = p->next) {
        printf("%d", p->data);
        if (p->next != NULL) {
            printf(", ");
        }
    }
    printf("}\n");
}

/* 函数功能：将结点node接到tail之后，更新tail。 */
void appendExistingNode(SetNode **tail, SetNode *node)
{
    (*tail)->next = node;
    *tail = node;
    node->next = NULL;
}

/* 函数功能：原地求并集，使用A、B原有结点，不新申请数据结点。 */
SetList unionSet(SetList a, SetList b)
{
    SetNode dummy;
    SetNode *tail = &dummy;
    SetNode *pa = a->next;
    SetNode *pb = b->next;
    dummy.next = NULL;

    while (pa != NULL && pb != NULL) {
        if (pa->data < pb->data) {
            SetNode *next = pa->next;
            appendExistingNode(&tail, pa);
            pa = next;
        } else if (pa->data > pb->data) {
            SetNode *next = pb->next;
            appendExistingNode(&tail, pb);
            pb = next;
        } else {
            SetNode *nextA = pa->next;
            SetNode *nextB = pb->next;
            appendExistingNode(&tail, pa);
            free(pb);
            pa = nextA;
            pb = nextB;
        }
    }
    while (pa != NULL) {
        SetNode *next = pa->next;
        appendExistingNode(&tail, pa);
        pa = next;
    }
    while (pb != NULL) {
        SetNode *next = pb->next;
        appendExistingNode(&tail, pb);
        pb = next;
    }

    a->next = dummy.next;
    b->next = NULL;
    destroySet(b);
    return a;
}

/* 函数功能：原地求交集，结果保存在A中，释放无关结点。 */
SetList intersectSet(SetList a, SetList b)
{
    SetNode dummy;
    SetNode *tail = &dummy;
    SetNode *pa = a->next;
    SetNode *pb = b->next;
    dummy.next = NULL;

    while (pa != NULL && pb != NULL) {
        if (pa->data < pb->data) {
            SetNode *next = pa->next;
            free(pa);
            pa = next;
        } else if (pa->data > pb->data) {
            SetNode *next = pb->next;
            free(pb);
            pb = next;
        } else {
            SetNode *nextA = pa->next;
            SetNode *nextB = pb->next;
            appendExistingNode(&tail, pa);
            free(pb);
            pa = nextA;
            pb = nextB;
        }
    }
    while (pa != NULL) {
        SetNode *next = pa->next;
        free(pa);
        pa = next;
    }
    while (pb != NULL) {
        SetNode *next = pb->next;
        free(pb);
        pb = next;
    }

    a->next = dummy.next;
    b->next = NULL;
    destroySet(b);
    return a;
}

/* 函数功能：原地求差集A-B，结果保存在A中。 */
SetList differenceSet(SetList a, SetList b)
{
    SetNode dummy;
    SetNode *tail = &dummy;
    SetNode *pa = a->next;
    SetNode *pb = b->next;
    dummy.next = NULL;

    while (pa != NULL && pb != NULL) {
        if (pa->data < pb->data) {
            SetNode *next = pa->next;
            appendExistingNode(&tail, pa);
            pa = next;
        } else if (pa->data > pb->data) {
            SetNode *next = pb->next;
            free(pb);
            pb = next;
        } else {
            SetNode *nextA = pa->next;
            SetNode *nextB = pb->next;
            free(pa);
            free(pb);
            pa = nextA;
            pb = nextB;
        }
    }
    while (pa != NULL) {
        SetNode *next = pa->next;
        appendExistingNode(&tail, pa);
        pa = next;
    }
    while (pb != NULL) {
        SetNode *next = pb->next;
        free(pb);
        pb = next;
    }

    a->next = dummy.next;
    b->next = NULL;
    destroySet(b);
    return a;
}

/* 函数功能：运行一次集合运算用例。 */
void runCase(const char *title, const ElemType aValues[], int aLen,
             const ElemType bValues[], int bLen)
{
    printf("\n%s\n", title);
    SetList a = buildSet(aValues, aLen);
    SetList b = buildSet(bValues, bLen);
    printSet("A", a);
    printSet("B", b);

    SetList u = unionSet(buildSet(aValues, aLen), buildSet(bValues, bLen));
    SetList i = intersectSet(buildSet(aValues, aLen), buildSet(bValues, bLen));
    SetList d = differenceSet(buildSet(aValues, aLen), buildSet(bValues, bLen));
    printSet("A并B", u);
    printSet("A交B", i);
    printSet("A差B", d);

    destroySet(a);
    destroySet(b);
    destroySet(u);
    destroySet(i);
    destroySet(d);
}

/* 函数功能：程序入口，运行多组覆盖性测试。 */
int main(void)
{
    ElemType a1[] = {7, 1, 3, 3, 5, 9};
    ElemType b1[] = {8, 2, 3, 5, 10};
    ElemType a2[] = {1, 2, 3};
    ElemType b2[] = {4, 5, 6};
    ElemType a3[] = {2, 4, 6};
    ElemType b3[] = {2, 4, 6};
    ElemType a4[] = {0};
    ElemType b4[] = {1, 3, 5};

    runCase("用例1：有重复输入且部分相交", a1, 6, b1, 5);
    runCase("用例2：两个集合完全不相交", a2, 3, b2, 3);
    runCase("用例3：两个集合完全相同", a3, 3, b3, 3);
    runCase("用例4：A为空集", a4, 0, b4, 3);
    return 0;
}
