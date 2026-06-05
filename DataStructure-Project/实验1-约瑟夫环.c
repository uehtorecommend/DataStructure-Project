#include <stdio.h>
#include <stdlib.h>

typedef struct Student {
    int num;
    int pwd;
} Student;

/* 定义单向循环链表结点，用于存储约瑟夫环中的每个人。 */
typedef struct Node {
    Student data;
    struct Node *next;
} Node;

/*
 * 函数功能：创建包含n个人的单向循环链表，所有人的密码均为m。
 * 参数说明：n为人数，m为统一密码。
 * 返回值：创建成功返回循环链表头指针，创建失败返回NULL。
 */
Node *createJosephRing(int n, int m)
{
    Node *head = NULL;
    Node *tail = NULL;

    /* 依次创建编号为1到n的结点，并按线性顺序连接。 */
    for (int i = 1; i <= n; i++) {
        Node *newNode = (Node *)malloc(sizeof(Node));
        if (newNode == NULL) {
            fprintf(stderr, "内存分配失败\n");

            /* 若创建过程中失败，释放已经申请的结点空间。 */
            while (head != NULL) {
                Node *next = head->next;
                free(head);
                if (next == head || next == NULL) {
                    break;
                }
                head = next;
            }
            return NULL;
        }

        /* 初始化当前人员的编号和密码。 */
        newNode->data.num = i;
        newNode->data.pwd = m;
        newNode->next = NULL;

        /* 将新结点插入链表尾部。 */
        if (head == NULL) {
            head = newNode;
            tail = newNode;
        } else {
            tail->next = newNode;
            tail = newNode;
        }
    }

    /* 尾结点指向头结点，形成单向循环链表。 */
    tail->next = head;
    return head;
}

/*
 * 函数功能：按照约瑟夫环规则依次删除出圈人员，并记录出圈顺序。
 * 参数说明：head为循环链表头指针，n为总人数，outOrder用于保存出圈编号。
 */
void josephOut(Node *head, int n, int outOrder[])
{
    Node *p = head;
    Node *q = head;
    int currentNumber = 1;
    int count = 0;

    /* q始终指向p的前驱结点，先将q移动到尾结点。 */
    while (q->next != head) {
        q = q->next;
    }

    /* 循环报数，直到所有人员均已出圈。 */
    while (count < n) {
        if (currentNumber == p->data.pwd) {
            /* 当前报数等于密码时，记录该人员编号并准备删除结点。 */
            outOrder[count++] = p->data.num;
            currentNumber = 1;

            /* 删除当前结点，并让p指向下一位继续报数。 */
            q->next = p->next;
            free(p);
            p = q->next;

            if (count == n) {
                break;
            }
        } else {
            /* 未达到密码时，报数加1，指针顺序后移。 */
            currentNumber++;
            q = p;
            p = p->next;
        }
    }
}

/*
 * 函数功能：输出约瑟夫环中人员的出圈顺序。
 * 参数说明：outOrder为出圈顺序数组，n为数组长度。
 * 返回值：无。
 */
void printOutOrder(const int outOrder[], int n)
{
    for (int i = 0; i < n; i++) {
        if (i > 0) {
            printf(" ");
        }
        printf("%d", outOrder[i]);
    }
    printf("\n");
}

/*
 * 函数功能：程序入口，完成数据输入、建环、出圈处理和结果输出。
 * 返回值：程序正常结束返回0，输入或内存错误返回1。
 */
int main(void)
{
    int n, m;
    int *outOrder = NULL;
    Node *head = NULL;

    /* 输入总人数n和统一的密码m */
    printf("请输入人数n和密码m：");
    if (scanf("%d %d", &n, &m) != 2) {
        printf("输入格式错误\n");
        return 1;
    }

    /* 检查输入数据是否合法 */
    if (n <= 0 || m <= 0) {
        printf("n和m必须为正整数\n");
        return 1;
    }

    /* 申请数组空间，用于保存最终出圈顺序 */
    outOrder = (int *)malloc(sizeof(int) * n);
    if (outOrder == NULL) {
        fprintf(stderr, "内存分配失败\n");
        return 1;
    }

    /* 建立约瑟夫环 */
    head = createJosephRing(n, m);
    if (head == NULL) {
        free(outOrder);
        return 1;
    }

    /* 执行出圈操作并输出结果 */
    josephOut(head, n, outOrder);

    printf("出圈顺序为：");
    printOutOrder(outOrder, n);

    free(outOrder);
    return 0;
}