#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
#include <limits>
#include <queue>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

const int MAX_COURSES = 100;
const int MAX_PRE = 40;
const int MAX_SEMESTERS = 8;
const int DEFAULT_SEMESTERS = 8;
const double DEFAULT_CREDIT_CAP = 15.0;
const int DEFAULT_COURSE_CAP = 5;

struct Course {
    string id;
    string name;
    double credit;
    vector<string> pre;
};

struct Graph {
    vector<vector<int> > adj;
    vector<int> indegree;
};

struct SemesterPlan {
    vector<int> items;
    double totalCredit;

    SemesterPlan() : totalCredit(0) {}
};

static vector<Course> courses;
static int semesterCount = DEFAULT_SEMESTERS;
static double creditCap = DEFAULT_CREDIT_CAP;
static int courseCap = DEFAULT_COURSE_CAP;
static vector<SemesterPlan> lastPlan;
static string lastPlanTitle;

/* 按两行格式输出无解原因，使错误类型和具体位置更清晰 */
static void printNoSolution(const string &type, const string &detail) {
    cout << "\n【无解】 " << type << "\n";
    cout <<  detail << "\n";
}

/* 将数值转为字符串，便于拼接提示信息 */
static string toStr(double value) {
    ostringstream out;
    out << value;
    return out.str();
}

/* 根据课程编号查找课程下标 */
static int findCourseIndex(const string &id) {
    for (int i = 0; i < (int)courses.size(); ++i) {
        if (courses[i].id == id) {
            return i;
        }
    }
    return -1;
}

/* 根据课程数组和先修关系建立 AOV 图 */
static bool createGraph(Graph &g) {
    int n = (int)courses.size();
    g.adj.assign(n, vector<int>());
    g.indegree.assign(n, 0);

    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < (int)courses[i].pre.size(); ++j) {
            int preIndex = findCourseIndex(courses[i].pre[j]);
            if (preIndex == -1) {
                printNoSolution("先修课程不存在",
                                "课程 " + courses[i].id + " 的先修课程 " +
                                courses[i].pre[j] + " 不存在。");
                return false;
            }
            g.adj[preIndex].push_back(i); // 先修课指向后续课程
            ++g.indegree[i];
        }
    }
    return true;
}

/* 拓扑排序：成功时输出拓扑序列，并计算最早可修层数 */
static bool topologicalSort(const Graph &g, vector<int> &topoOrder, vector<int> &earliest) {
    vector<int> indegree = g.indegree;
    queue<int> q;

    topoOrder.clear();
    earliest.assign(courses.size(), 1);
    for (int i = 0; i < (int)indegree.size(); ++i) {
        if (indegree[i] == 0) {
            q.push(i); // 入度为0的课程可以最先学习
        }
    }

    while (!q.empty()) {
        int v = q.front();
        q.pop();
        topoOrder.push_back(v);
        for (int i = 0; i < (int)g.adj[v].size(); ++i) {
            int w = g.adj[v][i];
            earliest[w] = max(earliest[w], earliest[v] + 1); // 更新后继课程层数
            if (--indegree[w] == 0) {
                q.push(w);
            }
        }
    }
    return topoOrder.size() == courses.size();
}

/* 判断一门课程是否还能放入某个学期 */
static bool canPut(const SemesterPlan &sem, int idx) {
    return (int)sem.items.size() < courseCap &&
           sem.totalCredit + courses[idx].credit <= creditCap;
}

/* 将课程加入指定学期 */
static void putCourse(SemesterPlan &sem, int idx) {
    sem.items.push_back(idx);
    sem.totalCredit += courses[idx].credit;
}

/* 检查课程数量、学分等基础约束 */
static bool checkBasicLimits() {
    if (courses.empty()) {
        cout << "当前没有课程，请先读取或输入课程目录。\n";
        return false;
    }
    for (int i = 0; i < (int)courses.size(); ++i) {
        if (courses[i].credit > creditCap) {
            printNoSolution("学分不足",
                            "课程 " + courses[i].id + " 学分为 " +
                            toStr(courses[i].credit) + "，超过单学期学分上限 " +
                            toStr(creditCap) + "。");
            return false;
        }
    }
    if ((int)courses.size() > semesterCount * courseCap) {
        printNoSolution("课程容量不足",
                        "课程总数 " + toStr((int)courses.size()) + "，" +
                        toStr(semesterCount) + " 个学期最多只能安排 " +
                        toStr(semesterCount * courseCap) + " 门课程。");
        return false;
    }
    return true;
}

/* 建图并完成拓扑排序，是两种排课策略共同的准备步骤 */
static bool prepareTopo(Graph &g, vector<int> &topoOrder, vector<int> &earliest) {
    if (!checkBasicLimits()) {
        return false;
    }
    if (!createGraph(g)) {
        return false;
    }
    if (!topologicalSort(g, topoOrder, earliest)) {
        printNoSolution("先修课程矛盾", "课程先修关系图中存在回路，无法完成拓扑排序。");
        return false;
    }
    return true;
}

/* 在控制台输出教学计划 */
static void printPlan(const vector<SemesterPlan> &plan, const string &title) {
    cout << "\n" << title << "\n";
    for (int i = 0; i < (int)plan.size(); ++i) {
        cout << "第 " << i + 1 << " 学期：课程数 " << plan[i].items.size()
             << "，学分 " << plan[i].totalCredit << "\n";
        if (plan[i].items.empty()) {
            cout << "  （无）\n";
        }
        for (int j = 0; j < (int)plan[i].items.size(); ++j) {
            int idx = plan[i].items[j];
            cout << "  " << courses[idx].id << "  "
                 << courses[idx].name << "  "
                 << courses[idx].credit << " 学分\n";
        }
    }
}

/* 从文件读取课程进修目录
 * 文件第一行为课程总数。
 * 后续每行格式要求：<课程编号> <课程名称> <学分> <先修课程数> <先修1编号> <先修2编号> ...
 * 说明：课程名称不能包含空格；学分允许输入小数；先修课程仍使用课程编号表示。
*/
void readCoursefile() {
    string filename;
    int n;

    cout << "请输入课程目录文件名：";
    cin >> filename;

    ifstream fin(filename.c_str());
    if (!fin) {
        cout << "文件打开失败。\n";
        return;
    }
    if (!(fin >> n) || n < 0 || n > MAX_COURSES) {
        cout << "文件格式错误：第一行应为课程数量（0~" << MAX_COURSES << "）。\n";
        return;
    }

    courses.clear(); // 读取新目录时覆盖原有课程数据
    for (int i = 0; i < n; ++i) {
        Course c;
        int preCount;
        if (!(fin >> c.id >> c.name >> c.credit >> preCount)) {
            cout << "文件格式错误：第 " << i + 1 << " 门课程信息不完整。\n";
            courses.clear();
            return;
        }
        if (preCount < 0 || preCount > MAX_PRE) {
            cout << "文件格式错误：课程 " << c.id << " 的先修课程数量非法。\n";
            courses.clear();
            return;
        }
        c.pre.resize(preCount);
        for (int j = 0; j < preCount; ++j) {
            fin >> c.pre[j];
        }
        if (findCourseIndex(c.id) != -1) {
            cout << "文件格式错误：课程编号 " << c.id << " 重复。\n";
            courses.clear();
            return;
        }
        courses.push_back(c);
    }
    lastPlan.clear();
    cout << "成功读取 " << courses.size() << " 门课程。\n";
}

/* 通过控制台逐项输入课程进修目录
 * 格式要求与文件读取一致：课程编号、课程名称、学分、直接先修课程数量、各先修课程编号。
 * 说明：课程名称不能包含空格；学分允许输入小数；先修课程输入课程编号。
*/
void manualInput() {
    int n;
    cout << "请输入课程数量：";
    if (!(cin >> n) || n < 0 || n > MAX_COURSES) {
        cout << "课程数量非法。\n";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        return;
    }

    courses.clear(); // 手动输入视为重新建立课程目录
    for (int i = 0; i < n; ++i) {
        Course c;
        int preCount;
        cout << "\n第 " << i + 1 << " 门课程编号：";
        cin >> c.id;
        if (findCourseIndex(c.id) != -1) {
            cout << "课程编号重复，本次输入终止。\n";
            courses.clear();
            return;
        }
        cout << "课程名称（不含空格）：";
        cin >> c.name;
        cout << "学分：";
        cin >> c.credit;
        cout << "直接先修课程数量：";
        cin >> preCount;
        if (preCount < 0 || preCount > MAX_PRE) {
            cout << "先修课程数量非法，本次输入终止。\n";
            courses.clear();
            return;
        }
        c.pre.resize(preCount);
        for (int j = 0; j < preCount; ++j) {
            cout << "第 " << j + 1 << " 个先修课程编号：";
            cin >> c.pre[j];
        }
        courses.push_back(c);
    }
    lastPlan.clear();
    cout << "手动输入完成，共 " << courses.size() << " 门课程。\n";
}

/* 显示所有课程及其直接先修课程
 * 显示格式：课程编号、课程名称、学分、直接先修课程名称。
 * 说明：先修课程在输入和存储时仍为编号，此处只在展示时转换为课程名称。
*/
void showCourserelations() {
    if (courses.empty()) {
        cout << "当前没有课程。\n";
        return;
    }
    cout << "\n课程编号\t课程名称\t学分\t直接先修课程\n";
    cout << "----------------------------------------\n";
    for (int i = 0; i < (int)courses.size(); ++i) {
        cout << courses[i].id << "\t" << courses[i].name
             << "\t" << courses[i].credit << "\t";
        if (courses[i].pre.empty()) {
            cout << "无";
        } else {
            for (int j = 0; j < (int)courses[i].pre.size(); ++j) {
                int preIndex = findCourseIndex(courses[i].pre[j]);
                if (j > 0) {
                    cout << ",";
                }
                // 输入和存储仍使用课程编号，显示时转换为课程名称
                if (preIndex == -1) {
                    cout << courses[i].pre[j] << "（未找到）";
                } else {
                    cout << courses[preIndex].name;
                }
            }
        }
        cout << "\n";
    }
}

/* 增加一门课程
 * 输入格式要求：课程编号、课程名称、学分、直接先修课程数量、各先修课程编号。
 * 说明：课程名称不能包含空格；学分允许输入小数。
*/
static void addCourse() {
    Course c;
    int preCount;
    if ((int)courses.size() >= MAX_COURSES) {
        cout << "课程数量已达上限。\n";
        return;
    }
    cout << "课程编号：";
    cin >> c.id;
    if (findCourseIndex(c.id) != -1) {
        cout << "课程编号已存在。\n";
        return;
    }
    cout << "课程名称（不含空格）：";
    cin >> c.name;
    cout << "学分：";
    cin >> c.credit;
    cout << "直接先修课程数量：";
    cin >> preCount;
    if (preCount < 0 || preCount > MAX_PRE) {
        cout << "先修课程数量非法。\n";
        return;
    }
    c.pre.resize(preCount);
    for (int j = 0; j < preCount; ++j) {
        cout << "第 " << j + 1 << " 个先修课程编号：";
        cin >> c.pre[j];
    }
    courses.push_back(c);
    lastPlan.clear();
    cout << "课程已增加。\n";
}

/* 删除一门课程，并移除其他课程中指向它的先修关系 */
static void deleteCourse() {
    string id;
    cout << "请输入要删除的课程编号：";
    cin >> id;

    int idx = findCourseIndex(id);
    if (idx == -1) {
        cout << "未找到该课程。\n";
        return;
    }
    courses.erase(courses.begin() + idx);
    for (int i = 0; i < (int)courses.size(); ++i) {
        courses[i].pre.erase(remove(courses[i].pre.begin(), courses[i].pre.end(), id),
                             courses[i].pre.end()); // 删除相关先修关系
    }
    lastPlan.clear();
    cout << "课程已删除，相关先修关系也已移除。\n";
}

/* 修改一门课程的编号、名称、学分和先修课程
 * 输入格式要求：新的课程编号、课程名称、学分、直接先修课程数量、各先修课程编号。
 * 说明：若课程编号改变，其他课程中引用的旧编号会同步更新。
*/
static void modifyCourse() {
    string oldId;
    Course c;
    int preCount;
    cout << "请输入要修改的课程编号：";
    cin >> oldId;

    int idx = findCourseIndex(oldId);
    if (idx == -1) {
        cout << "未找到该课程。\n";
        return;
    }
    cout << "新的课程编号：";
    cin >> c.id;
    if (oldId != c.id && findCourseIndex(c.id) != -1) {
        cout << "新课程编号已存在。\n";
        return;
    }
    cout << "新的课程名称（不含空格）：";
    cin >> c.name;
    cout << "新的学分：";
    cin >> c.credit;
    cout << "新的直接先修课程数量：";
    cin >> preCount;
    if (preCount < 0 || preCount > MAX_PRE) {
        cout << "先修课程数量非法。\n";
        return;
    }
    c.pre.resize(preCount);
    for (int j = 0; j < preCount; ++j) {
        cout << "第 " << j + 1 << " 个先修课程编号：";
        cin >> c.pre[j];
    }
    if (oldId != c.id) {
        for (int i = 0; i < (int)courses.size(); ++i) {
            replace(courses[i].pre.begin(), courses[i].pre.end(), oldId, c.id);
        }
    }
    courses[idx] = c;
    lastPlan.clear();
    cout << "课程信息已修改。\n";
}

/* 课程目录编辑菜单 */
void editCourse() {
    int op;
    cout << "\n1. 增加课程\n2. 删除课程\n3. 修改课程\n请输入操作编号：";
    if (!(cin >> op)) {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "输入无效。\n";
        return;
    }
    switch (op) {
        case 1: addCourse(); break;
        case 2: deleteCourse(); break;
        case 3: modifyCourse(); break;
        default: cout << "未知操作。\n"; break;
    }
}

/* 设置学期总数、学分上限和每学期课程数上限 */
void setCourseparameter() {
    int s, count;
    double credit;
    cout << "请输入学期总数（1~" << MAX_SEMESTERS << "）：";
    cin >> s;
    cout << "请输入一学期学分上限：";
    cin >> credit;
    cout << "请输入一学期课程数上限（不超过5）：";
    cin >> count;
    if (s < 1 || s > MAX_SEMESTERS || credit <= 0 || count < 1 || count > DEFAULT_COURSE_CAP) {
        cout << "参数非法，设置失败。\n";
        return;
    }
    semesterCount = s;
    creditCap = credit;
    courseCap = count;
    lastPlan.clear();
    cout << "参数设置成功：学期数=" << semesterCount
         << "，学分上限=" << creditCap
         << "，课程数上限=" << courseCap << "。\n";
}

/* 计算全部课程的总学分，用于确定均匀分布目标 */
static double totalCourseCredit() {
    double sum = 0;
    for (int i = 0; i < (int)courses.size(); ++i) {
        sum += courses[i].credit;
    }
    return sum;
}

/* 计算当前计划相对平均学分的偏差 */
static double planCreditVariance(const vector<SemesterPlan> &plan, double target) {
    double variance = 0;
    for (int i = 0; i < (int)plan.size(); ++i) {
        double diff = plan[i].totalCredit - target;
        variance += diff * diff;
    }
    return variance;
}

/* 计算某门课在先修约束下最早能安排到的学期下标
 * assigned 存储的是1开始的学期号，因此返回值正好是0开始的下一学期下标。
*/
static int earliestSemesterByPrereq(int courseIndex, const vector<int> &assigned) {
    int startIndex = 0;
    for (int i = 0; i < (int)courses[courseIndex].pre.size(); ++i) {
        int preIndex = findCourseIndex(courses[courseIndex].pre[i]);
        if (preIndex == -1 || assigned[preIndex] == 0) {
            return -1;
        }
        startIndex = max(startIndex, assigned[preIndex]);
    }
    return startIndex;
}
/* 反向扫描拓扑序，计算每门课在先修链约束下的最晚可排学期下标 */
static vector<int> calculateLatestSemester(const Graph &g, const vector<int> &topoOrder) {
    vector<int> latest(courses.size(), semesterCount - 1);
    for (int i = (int)topoOrder.size() - 1; i >= 0; --i) {
        int v = topoOrder[i];
        for (int j = 0; j < (int)g.adj[v].size(); ++j) {
            int w = g.adj[v][j];
            latest[v] = min(latest[v], latest[w] - 1);
        }
    }
    return latest;
}

/* 检查当前选择是否会让尚未安排的课程立刻失去可行学期 */
static bool hasFuturePosition(const vector<SemesterPlan> &plan,
                              const vector<int> &assigned,
                              const vector<int> &topoOrder,
                              const vector<int> &earliest,
                              const vector<int> &latest,
                              int nextPos) {
    for (int i = nextPos; i < (int)topoOrder.size(); ++i) {
        int idx = topoOrder[i];
        int start = earliest[idx] - 1;
        bool ok = false;

        for (int j = 0; j < (int)courses[idx].pre.size(); ++j) {
            int preIndex = findCourseIndex(courses[idx].pre[j]);
            if (preIndex != -1 && assigned[preIndex] > 0) {
                start = max(start, assigned[preIndex]);
            }
        }
        for (int s = start; s <= latest[idx] && s < semesterCount; ++s) {
            if (s >= 0 && canPut(plan[s], idx)) {
                ok = true;
                break;
            }
        }
        if (!ok) {
            return false;
        }
    }
    return true;
}

/* 按学习负担尽量均匀的策略生成教学计划
 * 先按拓扑序处理课程，再从最早合法学期开始选择当前最均衡的位置。
*/
void Evengenerate() {
    Graph g;
    vector<int> topoOrder, earliest, latest, assigned(courses.size(), 0);
    vector<SemesterPlan> plan(semesterCount);
    double target;

    if (!prepareTopo(g, topoOrder, earliest)) {
        return;
    }

    latest = calculateLatestSemester(g, topoOrder);
    target = totalCourseCredit() / semesterCount;
    for (int i = 0; i < (int)topoOrder.size(); ++i) {
        int idx = topoOrder[i];
        int preStart = earliestSemesterByPrereq(idx, assigned);
        int legalStart;
        int bestSem = -1;
        double bestVariance = 0;
        double bestAfterCredit = 0;

        if (preStart < 0) {
            printNoSolution("先修课程未完成", "课程 " + courses[idx].id + " 的直接先修课程尚未全部安排。");
            return;
        }

        legalStart = max(earliest[idx] - 1, preStart); // earliest为1开始层数，plan为0开始下标
        if (legalStart > latest[idx]) {
            printNoSolution("学期安排冲突", "课程 " + courses[idx].id + " 已超过最晚可排学期。");
            return;
        }
        for (int s = legalStart; s <= latest[idx] && s < semesterCount; ++s) {
            if (canPut(plan[s], idx)) {
                vector<SemesterPlan> temp = plan;
                double variance;
                double afterCredit;

                putCourse(temp[s], idx);
                assigned[idx] = s + 1;
                if (!hasFuturePosition(temp, assigned, topoOrder, earliest, latest, i + 1)) {
                    assigned[idx] = 0;
                    continue;
                }
                assigned[idx] = 0;
                variance = planCreditVariance(temp, target);
                afterCredit = temp[s].totalCredit;

                // 优先选择整体偏差最小的位置；若相同，则选择放入后负担较轻的学期，避免挤在前面
                if (bestSem == -1 || variance < bestVariance - 1e-8 ||
                    (variance <= bestVariance + 1e-8 && afterCredit < bestAfterCredit - 1e-8)) {
                    bestSem = s;
                    bestVariance = variance;
                    bestAfterCredit = afterCredit;
                }
            }
        }

        if (bestSem == -1) {
            printNoSolution("学期安排冲突",
                            "课程 " + courses[idx].id + " 无法在当前学期数、学分上限和课程数上限内安排。");
            return;
        }

        putCourse(plan[bestSem], idx);
        assigned[idx] = bestSem + 1; // assigned记录1开始的学期号，便于后继课程计算下一学期下标
    }

    lastPlan = plan;
    lastPlanTitle = "按学习负担尽量均匀生成的教学计划";
    printPlan(lastPlan, lastPlanTitle);
}

/* 搜索结果严格区分穷尽无解与预算耗尽。每个期限最多展开20万个结点，
 * 深度不超过课程数；按拓扑序枚举每门课的所有合法学期，搜索完整时可证明无解。
 * 正学分使容量与未来位置剪枝成立。 */
enum SearchResult { PLAN_FOUND, PLAN_IMPOSSIBLE, PLAN_UNKNOWN };

static SearchResult searchSchedule(const vector<int> &order,
                                   const vector<int> &earliest,
                                   const vector<int> &latest,
                                   vector<int> &assigned,
                                   vector<SemesterPlan> &plan,
                                   int pos, int &budget) {
    if (pos == (int)order.size()) return PLAN_FOUND;
    if (budget <= 0) return PLAN_UNKNOWN;
    --budget;
    if (!hasFuturePosition(plan, assigned, order, earliest, latest, pos))
        return PLAN_IMPOSSIBLE;
    int idx = order[pos];
    int start = max(earliest[idx] - 1, earliestSemesterByPrereq(idx, assigned));
    for (int sem = start; sem <= latest[idx]; ++sem) {
        if (!canPut(plan[sem], idx)) continue;
        // 保存原值，避免浮点加减撤销产生累计误差。
        double oldCredit = plan[sem].totalCredit;
        putCourse(plan[sem], idx);
        assigned[idx] = sem + 1;
        SearchResult result = searchSchedule(order, earliest, latest, assigned,
                                             plan, pos + 1, budget);
        if (result == PLAN_FOUND) return result;
        assigned[idx] = 0;
        plan[sem].items.pop_back();
        plan[sem].totalCredit = oldCredit;
        if (result == PLAN_UNKNOWN) return result;
    }
    return PLAN_IMPOSSIBLE;
}

/* 依次尝试递增的完成期限。较小期限未证无解时，不宣称结果最优。 */
void Shortestgenerate(int searchBudget = 200000) {
    Graph g;
    vector<int> topoOrder, earliest;
    lastPlan.clear();
    if (!prepareTopo(g, topoOrder, earliest)) return;
    for (int i = 0; i < (int)courses.size(); ++i) {
        if (!(courses[i].credit > 0) || !isfinite(courses[i].credit)) {
            cout << "课程学分必须为有限正数。\n";
            return;
        }
    }
    vector<int> latest = calculateLatestSemester(g, topoOrder);
    // 在保持拓扑序的前提下，优先处理后续链最长的课程。
    vector<int> order, indegree = g.indegree;
    while (order.size() < courses.size()) {
        int best = -1;
        for (int i = 0; i < (int)courses.size(); ++i) {
            if (indegree[i] != 0) continue;
            if (best == -1 || latest[i] < latest[best] ||
                (latest[i] == latest[best] && courses[i].credit > courses[best].credit))
                best = i;
        }
        order.push_back(best);
        indegree[best] = -1;
        for (int j = 0; j < (int)g.adj[best].size(); ++j) --indegree[g.adj[best][j]];
    }
    int lower = max(*max_element(earliest.begin(), earliest.end()),
                    ((int)courses.size() + courseCap - 1) / courseCap);
    bool unresolved = false;
    for (int horizon = lower; horizon <= semesterCount; ++horizon) {
        if (totalCourseCredit() > horizon * creditCap + 1e-8) continue;
        vector<int> limits = latest;
        for (int i = 0; i < (int)limits.size(); ++i)
            limits[i] -= semesterCount - horizon;
        vector<int> assigned(courses.size(), 0);
        vector<SemesterPlan> plan(semesterCount);
        int budget = searchBudget;
        SearchResult result = searchSchedule(order, earliest, limits, assigned, plan, 0, budget);
        if (result == PLAN_UNKNOWN) unresolved = true;
        if (result != PLAN_FOUND) continue;
        lastPlan = plan;
        lastPlanTitle = unresolved ? "已找到可行教学计划（尚未证明最短）" :
                                    "按尽可能短时间生成的教学计划";
        printPlan(lastPlan, lastPlanTitle);
        return;
    }
    if (unresolved) {
        cout << "\n【搜索未完成】达到搜索预算，当前策略未找到解，不能据此判断无解。\n";
    } else {
        printNoSolution("学期安排不可行", "已通过约束检查或完整搜索证明指定期限内不存在合法教学计划。");
    }
}

/* 将最近一次生成的教学计划输出到文本文件 */
void OutputScheduletofile() {
    string filename;
    if (lastPlan.empty()) {
        cout << "当前没有可输出的教学计划，请先生成教学计划。\n";
        return;
    }
    cout << "请输入输出文件名：";
    cin >> filename;

    ofstream fout(filename.c_str());
    if (!fout) {
        cout << "文件创建失败。\n";
        return;
    }
    fout << lastPlanTitle << "\n";
    fout << "学期总数：" << lastPlan.size()
         << "，学分上限：" << creditCap
         << "，课程数上限：" << courseCap << "\n\n";
    for (int i = 0; i < (int)lastPlan.size(); ++i) {
        fout << "第 " << i + 1 << " 学期：课程数 " << lastPlan[i].items.size()
             << "，学分 " << lastPlan[i].totalCredit << "\n";
        if (lastPlan[i].items.empty()) {
            fout << "  （无）\n";
        }
        for (int j = 0; j < (int)lastPlan[i].items.size(); ++j) {
            int idx = lastPlan[i].items[j];
            fout << "  " << courses[idx].id << "  "
                 << courses[idx].name << "  "
                 << courses[idx].credit << " 学分\n";
        }
    }
    cout << "教学计划已输出到文件 " << filename << "。\n";
}

/* 打印系统主菜单 */
static void printMenu() {
    cout << "\n----------------------------------------\n";
    cout << "教学计划编制系统\n";
    cout << "当前设置：课程" << courses.size()
         << "门，学期" << semesterCount
         << "个，学分上限" << creditCap
         << "，课程数上限" << courseCap << "\n";
    cout << "----------------------------------------\n";
    cout << "1. 读取课程进修目录文件\n";
    cout << "2. 手动输入课程信息\n";
    cout << "3. 显示所有课程及其先修关系\n";
    cout << "4. 编辑课程信息（增/删/改）\n";
    cout << "5. 设置学期参数（默认：8学期、15学分、5门课）\n";
    cout << "6. 按均匀分布策略生成教学计划\n";
    cout << "7. 按最短时间策略生成教学计划\n";
    cout << "8. 输出教学计划到文件\n";
    cout << "0. 退出系统\n";
}

/* 程序入口：循环接收用户菜单选择并调用对应功能 */
int main() {
    int syb;
    do {
        printMenu();
        cout << "请输入操作编号：";
        if (!(cin >> syb)) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "输入无效，请重新输入。\n";
            continue;
        }
        switch (syb) {
            case 1: readCoursefile(); break;
            case 2: manualInput(); break;
            case 3: showCourserelations(); break;
            case 4: editCourse(); break;
            case 5: setCourseparameter(); break;
            case 6: Evengenerate(); break;
            case 7: Shortestgenerate(); break;
            case 8: OutputScheduletofile(); break;
            case 0: cout << "系统已退出。\n"; break;
            default: cout << "未知操作，请输入0~8之间的数字。\n"; break;
        }
    } while (syb != 0);

    return 0;
}
