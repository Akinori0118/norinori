#include <bits/stdc++.h>
using namespace std;

static std::tm parseDate(const string& s) {
    std::tm tm{};
    if (s.size() != 10 || s[4] != '-' || s[7] != '-') {
        throw runtime_error("日付は YYYY-MM-DD 形式で入力してください");
    }
    tm.tm_year = stoi(s.substr(0,4)) - 1900;
    tm.tm_mon  = stoi(s.substr(5,2)) - 1;
    tm.tm_mday = stoi(s.substr(8,2));
    tm.tm_hour = 0; tm.tm_min = 0; tm.tm_sec = 0;

    std::tm tmp = tm;
    time_t t = mktime(&tmp);
    if (t == -1) throw runtime_error("無効な日付です");

    if (tmp.tm_year != tm.tm_year || tmp.tm_mon != tm.tm_mon || tmp.tm_mday != tm.tm_mday)
        throw runtime_error("存在しない日付です");
    return tm;
}


static string dateToString(const std::tm& tm) {
    ostringstream oss;
    oss << (tm.tm_year + 1900) << '-'
        << setw(2) << setfill('0') << (tm.tm_mon + 1) << '-'
        << setw(2) << setfill('0') << tm.tm_mday;
    return oss.str();
}

static long long daysRemaining(const std::tm& due) {
    using namespace std::chrono;

    auto now      = system_clock::now();
    time_t now_t  = system_clock::to_time_t(now);
    std::tm nowtm = *std::localtime(&now_t);
    nowtm.tm_hour = 0; nowtm.tm_min = 0; nowtm.tm_sec = 0;
    auto today_tp = system_clock::from_time_t(mktime(&nowtm));

    std::tm duetm = due;
    duetm.tm_hour = 0; duetm.tm_min = 0; duetm.tm_sec = 0;
    auto due_tp   = system_clock::from_time_t(mktime(&duetm));

    auto diff = duration_cast<hours>(due_tp - today_tp).count();
    return diff / 24;
}

class Task {
    string title;
    std::tm due{};
    bool done;

public:
    Task() : done(false) {}
    Task(string t, const std::tm& d) : title(std::move(t)), due(d), done(false) {}

    const string& getTitle() const { return title; }
    string getDueString() const { return dateToString(due); }
    bool isDone() const { return done; }
    void markDone() { done = true; }

    long long remainingDays() const { return daysRemaining(due); }

    string serialize() const {

        string safe = title;
        for (char& c : safe) if (c == ',') c = ' ';
        ostringstream oss;
        oss << safe << "," << getDueString() << "," << (done ? 1 : 0);
        return oss.str();
    }

    static Task deserialize(const string& line) {
        vector<string> cols;
        string cur; std::stringstream ss(line);
        while (getline(ss, cur, ',')) cols.push_back(cur);
        if (cols.size() != 3) throw runtime_error("破損した行: " + line);
        std::tm d = parseDate(cols[1]);
        Task t(cols[0], d);
        t.done = (cols[2] == "1");
        return t;
    }
};

class TodoList {
    vector<Task> tasks;
    const string savePath = "tasks.csv";

public:
    void add(const string& title, const std::tm& due) {
        tasks.emplace_back(title, due);
    }

    bool markDone(size_t index) {
        if (index == 0 || index > tasks.size()) return false;
        tasks[index - 1].markDone();
        return true;
    }

    void list() const {
        if (tasks.empty()) {
            cout << "（タスクはありません）\n";
            return;
        }
        cout << "No  状態  期限        残り日数  タイトル\n";
        cout << "-----------------------------------------------\n";
        for (size_t i = 0; i < tasks.size(); ++i) {
            const Task& t = tasks[i];
            long long rem = t.remainingDays();
            cout << setw(2) << (i+1) << "  "
                 << (t.isDone() ? "[済]" : "[未]")
                 << "  " << t.getDueString() << "  "
                 << setw(6) << rem << "  "
                 << t.getTitle() << "\n";
        }
    }

    void load() {
        tasks.clear();
        ifstream ifs(savePath);
        if (!ifs) return;
        string line;
        while (getline(ifs, line)) {
            if (line.empty()) continue;
            try {
                tasks.push_back(Task::deserialize(line));
            } catch (...) {

            }
        }
    }

    void save() const {
        ofstream ofs(savePath);
        for (auto& t : tasks) ofs << t.serialize() << "\n";
    }
};

static void printMenu() {
    cout << "\n==== TODO リスト ====\n"
         << "1) 追加\n"
         << "2) 一覧\n"
         << "3) 完了にする\n"
         << "4) 終了（自動保存）\n"
         << "選択: ";
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    TodoList todo;
    todo.load();

    while (true) {
        printMenu();
        int cmd; if (!(cin >> cmd)) break;
        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        try {
            if (cmd == 1) {
                cout << "タイトル: ";
                string title; getline(cin, title);
                cout << "期限（YYYY-MM-DD）: ";
                string ds; getline(cin, ds);
                std::tm due = parseDate(ds);
                todo.add(title, due);
                cout << "追加しました。\n";
            } else if (cmd == 2) {
                todo.list();
            } else if (cmd == 3) {
                cout << "完了にする番号: ";
                size_t idx; cin >> idx;
                if (todo.markDone(idx)) cout << "完了にしました。\n";
                else cout << "番号が不正です。\n";
            } else if (cmd == 4) {
                todo.save();
                cout << "保存して終了します。\n";
                break;
            } else {
                cout << "1〜4を選んでください。\n";
            }
        } catch (const exception& e) {
            cout << "エラー: " << e.what() << "\n";
        }
    }
    return 0;
}

