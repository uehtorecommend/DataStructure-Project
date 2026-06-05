#include <stdio.h>
#include <stdlib.h>

#define MAXSIZE 100

typedef int ElemType;

typedef struct {
    ElemType data[MAXSIZE];
    int length;
} SqList;

typedef struct LNode {
    ElemType data;
    struct LNode *next;
} LNode;

typedef LNode *LinkList;

/* 函数功能：初始化顺序表为空表。 */
void initSqList(SqList *list)
{
    list->length = 0;
}

/* 函数功能：清空顺序表。 */
void clearSqList(SqList *list)
{
    list->length = 0;
}

/* 函数功能：返回顺序表长度。 */
int sqListLength(const SqList *list)
{
    return list->length;
}

/* 函数功能：按位序取得顺序表元素，位序从1开始。 */
int getSqElem(const SqList *list, int pos, ElemType *value)
{
    if (pos < 1 || pos > list->length) {
        return 0;
    }
    *value = list->data[pos - 1];
    return 1;
}

/* 函数功能：定位顺序表中第一个等于value的元素，返回位序。 */
int locateSqElem(const SqList *list, ElemType value)
{
    for (int i = 0; i < list->length; ++i) {
        if (list->data[i] == value) {
            return i + 1;
        }
    }
    return 0;
}

/* 函数功能：在顺序表第pos个位置前插入元素。 */
int insertSqElem(SqList *list, int pos, ElemType value)
{
    if (pos < 1 || pos > list->length + 1 || list->length >= MAXSIZE) {
        return 0;
    }
    for (int i = list->length; i >= pos; --i) {
        list->data[i] = list->data[i - 1];
    }
    list->data[pos - 1] = value;
    ++list->length;
    return 1;
}

/* 函数功能：删除顺序表第pos个元素，并通过value返回被删值。 */
int deleteSqElem(SqList *list, int pos, ElemType *value)
{
    if (pos < 1 || pos > list->length) {
        return 0;
    }
    *value = list->data[pos - 1];
    for (int i = pos; i < list->length; ++i) {
        list->data[i - 1] = list->data[i];
    }
    --list->length;
    return 1;
}

/* 函数功能：由数组建立顺序表。 */
int createSqList(SqList *list, const ElemType values[], int n)
{
    if (n < 0 || n > MAXSIZE) {
        return 0;
    }
    list->length = n;
    for (int i = 0; i < n; ++i) {
        list->data[i] = values[i];
    }
    return 1;
}

/* 函数功能：输出顺序表元素。 */
void printSqList(const SqList *list)
{
    printf("[");
    for (int i = 0; i < list->length; ++i) {
        printf("%d", list->data[i]);
        if (i + 1 < list->length) {
            printf(", ");
        }
    }
    printf("]\n");
}

/* 函数功能：释放不带头结点的单链表。 */
void destroyNoHeadList(LinkList head)
{
    while (head != NULL) {
        LNode *next = head->next;
        free(head);
        head = next;
    }
}

/* 函数功能：用头插法建立不带头结点的单链表。 */
LinkList createNoHeadListByHeadInsert(const ElemType values[], int n)
{
    LinkList head = NULL;
    for (int i = 0; i < n; ++i) {
        LNode *node = (LNode *)malloc(sizeof(LNode));
        if (node == NULL) {
            destroyNoHeadList(head);
            return NULL;
        }
        node->data = values[i];
        node->next = head;
        head = node;
    }
    return head;
}

/* 函数功能：初始化带头结点的单链表。 */
LinkList initHeadList(void)
{
    LinkList head = (LinkList)malloc(sizeof(LNode));
    if (head != NULL) {
        head->next = NULL;
    }
    return head;
}

/* 函数功能：销毁带头结点的单链表。 */
void destroyHeadList(LinkList head)
{
    while (head != NULL) {
        LNode *next = head->next;
        free(head);
        head = next;
    }
}

/* 函数功能：用尾插法建立带头结点的单链表。 */
LinkList createHeadListByTailInsert(const ElemType values[], int n)
{
    LinkList head = initHeadList();
    if (head == NULL) {
        return NULL;
    }
    LNode *tail = head;
    for (int i = 0; i < n; ++i) {
        LNode *node = (LNode *)malloc(sizeof(LNode));
        if (node == NULL) {
            destroyHeadList(head);
            return NULL;
        }
        node->data = values[i];
        node->next = NULL;
        tail->next = node;
        tail = node;
    }
    return head;
}

/* 函数功能：求带头结点单链表长度。 */
int linkListLength(LinkList head)
{
    int length = 0;
    for (LNode *p = head->next; p != NULL; p = p->next) {
        ++length;
    }
    return length;
}

/* 函数功能：按位序取得带头结点单链表元素。 */
int getLinkElem(LinkList head, int pos, ElemType *value)
{
    if (pos < 1) {
        return 0;
    }
    int index = 1;
    for (LNode *p = head->next; p != NULL; p = p->next, ++index) {
        if (index == pos) {
            *value = p->data;
            return 1;
        }
    }
    return 0;
}

/* 函数功能：定位带头结点单链表中第一个等于value的结点。 */
int locateLinkElem(LinkList head, ElemType value)
{
    int index = 1;
    for (LNode *p = head->next; p != NULL; p = p->next, ++index) {
        if (p->data == value) {
            return index;
        }
    }
    return 0;
}

/* 函数功能：在带头结点单链表第pos个位置前插入元素。 */
int insertLinkElem(LinkList head, int pos, ElemType value)
{
    if (pos < 1) {
        return 0;
    }
    LNode *pre = head;
    for (int i = 1; i < pos && pre != NULL; ++i) {
        pre = pre->next;
    }
    if (pre == NULL) {
        return 0;
    }
    LNode *node = (LNode *)malloc(sizeof(LNode));
    if (node == NULL) {
        return 0;
    }
    node->data = value;
    node->next = pre->next;
    pre->next = node;
    return 1;
}

/* 函数功能：删除带头结点单链表第pos个元素。 */
int deleteLinkElem(LinkList head, int pos, ElemType *value)
{
    if (pos < 1) {
        return 0;
    }
    LNode *pre = head;
    for (int i = 1; i < pos && pre->next != NULL; ++i) {
        pre = pre->next;
    }
    if (pre->next == NULL) {
        return 0;
    }
    LNode *del = pre->next;
    *value = del->data;
    pre->next = del->next;
    free(del);
    return 1;
}

/* 函数功能：输出不带头结点单链表。 */
void printNoHeadList(LinkList head)
{
    printf("[");
    for (LNode *p = head; p != NULL; p = p->next) {
        printf("%d", p->data);
        if (p->next != NULL) {
            printf(", ");
        }
    }
    printf("]\n");
}

/* 函数功能：输出带头结点单链表。 */
void printHeadList(LinkList head)
{
    printf("[");
    for (LNode *p = head->next; p != NULL; p = p->next) {
        printf("%d", p->data);
        if (p->next != NULL) {
            printf(", ");
        }
    }
    printf("]\n");
}

/* 函数功能：运行顺序表和单链表的覆盖性测试。 */
int main(void)
{
    ElemType values[] = {10, 20, 30, 40, 50};
    SqList sq;
    ElemType value = 0;
    int ok = 0;

    printf("一、顺序表测试\n");
    initSqList(&sq);
    createSqList(&sq, values, 5);
    printf("建立顺序表：");
    printSqList(&sq);
    printf("长度：%d\n", sqListLength(&sq));
    ok = getSqElem(&sq, 3, &value);
    printf("取第3个元素：%s，值=%d\n", ok ? "成功" : "失败", value);
    printf("定位40：%d\n", locateSqElem(&sq, 40));
    printf("定位99：%d\n", locateSqElem(&sq, 99));
    printf("在第1位插入5：%s\n", insertSqElem(&sq, 1, 5) ? "成功" : "失败");
    printf("在表尾插入60：%s\n", insertSqElem(&sq, sq.length + 1, 60) ? "成功" : "失败");
    printSqList(&sq);
    ok = deleteSqElem(&sq, 4, &value);
    printf("删除第4位：%s，被删值=%d\n", ok ? "成功" : "失败", value);
    printf("非法删除第99位：%s\n", deleteSqElem(&sq, 99, &value) ? "成功" : "失败");
    printSqList(&sq);
    clearSqList(&sq);
    printf("置空后长度：%d\n\n", sqListLength(&sq));

    printf("二、不带头结点单链表头插法建表测试\n");
    LinkList noHead = createNoHeadListByHeadInsert(values, 5);
    printf("头插法结果：");
    printNoHeadList(noHead);
    destroyNoHeadList(noHead);

    printf("\n三、带头结点单链表测试\n");
    LinkList head = createHeadListByTailInsert(values, 5);
    if (head == NULL) {
        printf("链表创建失败\n");
        return 1;
    }
    printf("尾插法结果：");
    printHeadList(head);
    printf("长度：%d\n", linkListLength(head));
    ok = getLinkElem(head, 2, &value);
    printf("取第2个元素：%s，值=%d\n", ok ? "成功" : "失败", value);
    printf("定位30：%d\n", locateLinkElem(head, 30));
    printf("插入第3位25：%s\n", insertLinkElem(head, 3, 25) ? "成功" : "失败");
    ok = deleteLinkElem(head, 5, &value);
    printf("删除第5位：%s，被删值=%d\n", ok ? "成功" : "失败", value);
    printf("非法取第20位：%s\n", getLinkElem(head, 20, &value) ? "成功" : "失败");
    printHeadList(head);
    destroyHeadList(head);

    return 0;
}
