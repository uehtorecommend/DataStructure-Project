#define main teaching_menu_main
#include "../DataStructure-Project/实验2-教学计划编制.cpp"
#undef main

int main() {
    int n, mode;
    cin >> n >> semesterCount >> creditCap >> courseCap >> mode;
    for (int i = 0; i < n; ++i) {
        Course c;
        int k;
        cin >> c.id >> c.credit >> k;
        c.name = c.id;
        while (k--) { string pre; cin >> pre; c.pre.push_back(pre); }
        courses.push_back(c);
    }
    if (mode == 1 || mode == 4) Evengenerate(); else Shortestgenerate(mode == 3 ? 0 : 200000);
    cout << "RESULT " << lastPlan.size() << '\n';
    for (int s = 0; s < (int)lastPlan.size(); ++s)
        for (int j = 0; j < (int)lastPlan[s].items.size(); ++j)
            cout << "ASSIGN " << lastPlan[s].items[j] << ' ' << s << '\n';
    if (mode == 2 || mode == 4) OutputScheduletofile();
    if (mode == 3) {
        Graph g; vector<int> order, earliest;
        if (prepareTopo(g, order, earliest)) {
            vector<int> latest = calculateLatestSemester(g, order), assigned(n, 0);
            vector<SemesterPlan> plan(semesterCount);
            int budget = 0;
            cout << "BUDGET " << searchSchedule(order, earliest, latest, assigned, plan, 0, budget) << '\n';
        }
    }
}
