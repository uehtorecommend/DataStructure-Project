/* Include the complete program so tests exercise the production functions. */
#define main youth_demo_main
#include "../DataStructure-Project/实验12-校园十大优秀青年评比.c"
#undef main
#include <assert.h>
#include <stdlib.h>

static Candidate basic(const char *name)
{
    return makeCandidate(name, "class", "major", "grade", "achievement");
}

static void boundary(int field, int length)
{
    const size_t capacities[] = {NAME_CAPACITY, CLASS_CAPACITY, MAJOR_CAPACITY,
                                 GRADE_CAPACITY, ACHIEVEMENT_CAPACITY};
    const char *args[] = {"name", "class", "major", "grade", "achievement"};
    char text[4096];
    assert(length >= 0 && length < (int)sizeof(text));
    memset(text, 'x', (size_t)length);
    text[length] = '\0';
    args[field] = text;
    Candidate c = makeCandidate(args[0], args[1], args[2], args[3], args[4]);
    HashTable table, before;
    initTable(&table);
    before = table;
    int accepted = (size_t)length < capacities[field] && (field != 0 || length > 0);
    assert(c.used == accepted);
    assert(nominate(&table, c) == accepted);
    if (accepted) {
        const char *actual[] = {c.name, c.className, c.major, c.grade, c.achievement};
        for (int i = 0; i < 5; ++i) {
            assert(strcmp(actual[i], args[i]) == 0);
        }
        int count;
        int pos = findCandidate(&table, args[0], &count);
        assert(pos >= 0 && count >= 1);
        assert(vote(&table, args[0]) == 1);
        assert(table.data[pos].votes == 1);
    } else {
        assert(c.name[0] == '\0' && c.className[0] == '\0' && c.major[0] == '\0');
        assert(c.grade[0] == '\0' && c.achievement[0] == '\0' && c.votes == 0);
        assert(memcmp(&before, &table, sizeof(table)) == 0);
    }
}

static void invalid(void)
{
    HashTable table, before;
    initTable(&table);
    before = table;
    for (int i = 0; i < 5; ++i) {
        const char *args[] = {"name", "class", "major", "grade", "achievement"};
        args[i] = NULL;
        Candidate c = makeCandidate(args[0], args[1], args[2], args[3], args[4]);
        assert(c.used == 0 && !nominate(&table, c));
        c = basic("valid");
        char *fields[] = {c.name, c.className, c.major, c.grade, c.achievement};
        size_t sizes[] = {sizeof(c.name), sizeof(c.className), sizeof(c.major),
                          sizeof(c.grade), sizeof(c.achievement)};
        memset(fields[i], 'x', sizes[i]);
        assert(!nominate(&table, c));
        char *raw = malloc(sizes[i]);
        assert(raw != NULL);
        memset(raw, 'x', sizes[i]);
        args[i] = raw;
        c = makeCandidate(args[0], args[1], args[2], args[3], args[4]);
        assert(c.used == 0 && !nominate(&table, c));
        free(raw);
        assert(memcmp(&before, &table, sizeof(table)) == 0);
    }
    /* Exact-sized nonterminated input: bounded checks must not read past it. */
    char unterminated[NAME_CAPACITY];
    memset(unterminated, 'x', sizeof(unterminated));
    const char *bad[] = {NULL, "", unterminated,
                        "abcdefghijklmnopqrstuvwxyz0123456789"};
    for (size_t i = 0; i < sizeof(bad) / sizeof(bad[0]); ++i) {
        int count = 123;
        assert(hashName(bad[i]) == -1);
        assert(findCandidate(&table, bad[i], &count) == -1 && count == 0);
        assert(!vote(&table, bad[i]));
        showCandidate(&table, bad[i]);
        assert(memcmp(&before, &table, sizeof(table)) == 0);
    }
    Candidate c = basic("valid");
    c.votes = -1;
    assert(!nominate(&table, c));
    assert(!nominate(NULL, basic("valid")));
    assert(!vote(NULL, "valid"));
    assert(findCandidate(NULL, "valid", NULL) == -1);
}

static void flow(void)
{
    HashTable table, before;
    initTable(&table);
    assert(nominate(&table, basic("Alice")));
    assert(nominate(&table, basic("Bob")));
    before = table;
    assert(!nominate(&table, makeCandidate("Alice", "other", "other", "", "")));
    assert(memcmp(&before, &table, sizeof(table)) == 0);
    assert(vote(&table, "Alice") && vote(&table, "Bob") && vote(&table, "Bob"));
    assert(!vote(&table, "missing"));
    int a = findCandidate(&table, "Alice", NULL);
    int b = findCandidate(&table, "Bob", NULL);
    assert(table.data[a].votes == 1 && table.data[b].votes == 2);
    showCandidate(&table, "Bob");
    showAllVotes(&table);
    showRanking(&table);
    table.data[a].votes = INT_MAX;
    assert(!vote(&table, "Alice") && table.data[a].votes == INT_MAX);
}

static void collisions(void)
{
    HashTable table;
    initTable(&table);
    char names[HASH_SIZE][NAME_CAPACITY];
    int found = 0;
    /* Force a cluster starting at the last slot, including wraparound. */
    for (int i = 0; found < HASH_SIZE; ++i) {
        char name[NAME_CAPACITY];
        snprintf(name, sizeof(name), "collision%d", i);
        if (hashName(name) == HASH_SIZE - 1) {
            strcpy(names[found], name);
            assert(nominate(&table, basic(name)));
            ++found;
        }
    }
    for (int i = 0; i < HASH_SIZE; ++i) {
        int count;
        int pos = findCandidate(&table, names[i], &count);
        assert(pos == (HASH_SIZE - 1 + i) % HASH_SIZE && count == i + 1);
        assert(vote(&table, names[i]) && table.data[pos].votes == 1);
    }
    HashTable before = table;
    assert(!nominate(&table, basic(names[HASH_SIZE - 1])));
    assert(!nominate(&table, basic("extra")));
    int count;
    assert(findCandidate(&table, "missing", &count) == -1 && count == HASH_SIZE);
    assert(!vote(&table, "missing"));
    assert(memcmp(&before, &table, sizeof(table)) == 0);
}

static void identity(void)
{
    HashTable table;
    initTable(&table);
    char first[NAME_CAPACITY], second[NAME_CAPACITY], longName[NAME_CAPACITY + 2];
    memset(first, 'a', sizeof(first));
    first[NAME_CAPACITY - 1] = '\0';
    strcpy(second, first);
    second[NAME_CAPACITY - 2] = 'b';
    strcpy(longName, first);
    strcat(longName, "xy");
    assert(nominate(&table, basic(first)) && nominate(&table, basic(second)));
    HashTable before = table;
    assert(!nominate(&table, basic(longName)) && !vote(&table, longName));
    assert(memcmp(&before, &table, sizeof(table)) == 0);
    assert(vote(&table, first) && vote(&table, second));
    char utf8[NAME_CAPACITY + 4] = "";
    for (int i = 0; i < 9; ++i) strcat(utf8, "中");
    strcat(utf8, "ab"); /* 29 UTF-8 bytes: accepted intact. */
    assert(strlen(utf8) == 29 && nominate(&table, basic(utf8)));
    assert(vote(&table, utf8));
    strcat(utf8, "中");
    assert(!nominate(&table, basic(utf8)));
}

int main(int argc, char **argv)
{
    assert(argc >= 2);
    if (strcmp(argv[1], "boundary") == 0) {
        assert(argc == 4);
        boundary(atoi(argv[2]), atoi(argv[3]));
    } else if (strcmp(argv[1], "invalid") == 0) invalid();
    else if (strcmp(argv[1], "flow") == 0) flow();
    else if (strcmp(argv[1], "collisions") == 0) collisions();
    else if (strcmp(argv[1], "identity") == 0) identity();
    else assert(0);
    return 0;
}
