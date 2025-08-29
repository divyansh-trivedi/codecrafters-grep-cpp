#include <bits/stdc++.h>
#include <filesystem>
using namespace std;
namespace fs = filesystem;

struct State {
    shared_ptr<State> out, out1;
    int c = -1;
    int lastlist = -1;
    vector<int> match_set;
    vector<int> capture_ids;
};

struct Fragment {
    shared_ptr<State> start;
    vector<shared_ptr<State>*> out;
};

enum {
    Split = 256, MatchAny, MatchWord, MatchDigit, MatchChoice, MatchAntiChoice,
    MatchStart, MatchEnd, Epsilon = 299, BackRefStart = 300, Matched = 1000
};

int capture_id_counter = 0;
int listid = 0;

vector<string> find_files_recursively(fs::path dir) {
    vector<string> files;

    if (!fs::is_directory(dir)) {
        return { dir };
    }

    for (auto& e : fs::recursive_directory_iterator(dir)) {
        if (!fs::is_directory(e)) {
            files.push_back(e.path());
        }
    }

    return files;
}

void capture_fragment(shared_ptr<State> s) {
    while (s && s->c >= 0) {
        if (s->c == Split && s->out1) {
            capture_fragment(s->out1);
        }

        if (find(s->capture_ids.begin(), s->capture_ids.end(), capture_id_counter) != s->capture_ids.end()) {
            break;
        }

        s->capture_ids.insert(s->capture_ids.begin(), capture_id_counter);
        s = s->out;
    }
}
Fragment parse(string_view&, Fragment, int);

Fragment parse_primary(string_view& rx) {
    auto state = make_shared<State>();
    Fragment frag { state, { &state->out } };

    char c = rx.front();
    rx.remove_prefix(1);

    switch (c) {
        case '.':
            state->c = MatchAny;
            break;

        case '^':
            state->c = MatchStart;
            break;

        case '$':
            state->c = MatchEnd;
            break;

        case '\\': {
            c = rx.front();
            rx.remove_prefix(1);

            if (c == 'd') state->c = MatchDigit;
            else if (c == 'w') state->c = MatchWord;
            else if (isdigit(c)) state->c = BackRefStart + c - '0';
            else state->c = c;
            break;
        }

        case '[': {
            bool neg = rx.front() == '^';
            if (neg) rx.remove_prefix(1);

            state->c = neg ? MatchAntiChoice : MatchChoice;

            while (rx.front() != ']') {
                state->match_set.push_back(rx.front());
                rx.remove_prefix(1);
            }
            rx.remove_prefix(1);
            break;
        }

        case '(':
            frag = parse_primary(rx);
            frag = parse(rx, frag, 0);

            if (rx.front() != ')') {
                throw runtime_error("Expected ')'");
            }

            rx.remove_prefix(1);
            capture_fragment(frag.start);
            capture_id_counter++;
            break;

        default:
            state->c = c;
    }

    if (rx.size()) {
        switch (rx.front()) {
            case '*': {
                auto s = make_shared<State>();
                s->c = Split;
                s->out = frag.start;

                for (auto o : frag.out) {
                    *o = s;
                }

                frag = { s, { &s->out1 } };
                rx.remove_prefix(1);
            } break;

            case '+': {
                auto s = make_shared<State>();
                s->c = Split;
                s->out = frag.start;

                for (auto o : frag.out) {
                    *o = s;
                }

                frag.out = { &s->out1 };
                rx.remove_prefix(1);
            } break;

            case '?': {
                auto s = make_shared<State>();
                s->c = Split;
                s->out = frag.start;

                frag.start = s;
                frag.out.push_back(&s->out1);
                rx.remove_prefix(1);
            } break;
        }
    }

    return frag;
}

Fragment parse(string_view& rx, Fragment lhs, int min_prec) {
    auto prec = [](char c) {
        return c == '|' ? 0 : (c == ']' || c == ')') ? -1 : 1;
    };

    char look = rx.front();

    while (rx.size() && prec(look) >= min_prec) {
        char op = look;

        if (op == '|') {
            rx.remove_prefix(1);
        }

        Fragment rhs = parse_primary(rx);

        if (rx.size()) {
            look = rx.front();
        }

        while (prec(look) > prec(op)) {
            rhs = parse(rx, rhs, prec(op) + 1);
            if (!rx.size()) break;
            look = rx.front();
        }

        if (op == '|') {
            auto s = make_shared<State>();
            s->c = Split;
            s->out = lhs.start;
            s->out1 = rhs.start;

            lhs.start = s;
            lhs.out.insert(lhs.out.end(), rhs.out.begin(), rhs.out.end());
        } else {
            for (auto o : lhs.out) {
                *o = rhs.start;
            }
            lhs.out = rhs.out;
        }

        if (!rx.size()) break;
    }

    return lhs;
}

shared_ptr<State> regex2nfa(string_view rx) {
    auto matched = make_shared<State>();
    matched->c = Matched;

    if (rx.empty()) return matched;

    auto frag = parse_primary(rx);
    auto nfa = parse(rx, frag, 0);

    for (auto o : nfa.out) {
        *o = matched;
    }

    return nfa.start;
}

struct CaptureInfo {
    vector<string> groups;
    map<int, int> active;
    int idx = 0;
};

using List = vector<pair<CaptureInfo, shared_ptr<State>>>;

bool ismatch(const List& l) {
    return any_of(l.begin(), l.end(), [](auto& p) {
        return p.second->c == Matched;
    });
}

void addstate(shared_ptr<State> s, CaptureInfo& cap, List& l) {
    if (s->lastlist == listid) return;

    s->lastlist = listid;

    if (s->c == Split) {
        addstate(s->out, cap, l);
        addstate(s->out1, cap, l);
        return;
    }

    l.push_back({ cap, s });
}

void startlist(shared_ptr<State> s, List& l) {
    listid++;
    CaptureInfo cap{};
    addstate(s, cap, l);
}

void capture(char c, CaptureInfo& caps, shared_ptr<State> s) {
    for (int id : s->capture_ids) {
        if (!caps.active.count(id)) {
            caps.active[id] = caps.groups.size();
            caps.groups.emplace_back();
        }
        caps.groups[caps.active[id]] += c;
    }
}

void match_step(List& cl, char c, List& nl) {
    listid++;
    nl.clear();

    for (auto [cap, s] : cl) {
        switch (s->c) {
            case MatchAny:
                capture(c, cap, s);
                addstate(s->out, cap, nl);
                break;

            case MatchDigit:
                if (isdigit(c)) {
                    capture(c, cap, s);
                    addstate(s->out, cap, nl);
                }
                break;

            case MatchWord:
                if (isalnum(c) || c == '_') {
                    capture(c, cap, s);
                    addstate(s->out, cap, nl);
                }
                break;

            case MatchChoice:
                if (find(s->match_set.begin(), s->match_set.end(), c) != s->match_set.end()) {
                    capture(c, cap, s);
                    addstate(s->out, cap, nl);
                }
                break;

            case MatchAntiChoice:
                if (find(s->match_set.begin(), s->match_set.end(), c) == s->match_set.end()) {
                    capture(c, cap, s);
                    addstate(s->out, cap, nl);
                }
                break;

            default:
                if (s->c == c) {
                    capture(c, cap, s);
                    addstate(s->out, cap, nl);
                } else if (s->c > BackRefStart &&
                           cap.groups[s->c - BackRefStart - 1][cap.idx] == c) {
                    cap.idx++;
                    capture(c, cap, s);

                    if (cap.idx >= cap.groups[s->c - BackRefStart - 1].size()) {
                        cap.idx = 0;
                        addstate(s->out, cap, nl);
                    } else {
                        addstate(s, cap, nl);
                    }
                }
        }
    }
}

int matchEpsilonNFA(shared_ptr<State> start, string_view text) {
    List cl, nl;
    startlist(start, cl);

    bool started = false;

    if (start->c == MatchStart) {
        listid++;
        nl.clear();
        CaptureInfo cap{};
        addstate(start->out, cap, nl);
        swap(cl, nl);
        started = true;
    }

restart:
    for (char c : text) {
        match_step(cl, c, nl);
        swap(cl, nl);

        if (!started && cl.empty()) {
            startlist(start, cl);
            text.remove_prefix(1);
            goto restart;
        }

        if (ismatch(cl)) {
            return 1;
        }
    }

    for (auto [cap, s] : cl) {
        if (s->c == MatchEnd) {
            listid++;
            nl.clear();
            addstate(s->out, cap, nl);
            swap(cl, nl);
        }
    }

    return ismatch(cl);
}

int main(int argc, char* argv[]) {
    cout << unitbuf;
    cerr << unitbuf;

    if (argc < 3) {
        cerr << "Expected at least 3 arguments\n";
        return 1;
    }

    bool foundE = false;
    bool recursive = false;
    string pattern;
    vector<string> files;

    for (int i = 1; i < argc; i++) {
        string arg = argv[i];

        if (arg[0] == '-') {
            if (arg[1] == 'E') {
                foundE = true;
            } else if (arg[1] == 'r') {
                recursive = true;
            } else {
                throw runtime_error("Bad flag");
            }
        } else if (pattern.empty()) {
            pattern = arg;
        } else {
            files.push_back(arg);
        }
    }

    if (!foundE) {
        cerr << "Expected -E\n";
        return 1;
    }

    if (recursive) {
        vector<string> tmp = files;
        files.clear();

        for (auto& d : tmp) {
            auto df = find_files_recursively(d);
            files.insert(files.end(), df.begin(), df.end());
        }
    }

    string line;
    bool matched = false;

    if (files.empty()) {
        while (getline(cin, line)) {
            auto start = regex2nfa(pattern);
            if (matchEpsilonNFA(start, line)) {
                cout << line << endl;
                matched = true;
            }
        }
    } else {
        for (auto& f : files) {
            ifstream fin(f);

            while (getline(fin, line)) {
                auto start = regex2nfa(pattern);
                if (matchEpsilonNFA(start, line)) {
                    if (files.size() > 1) {
                        cout << f << ":";
                    }
                    cout << line << endl;
                    matched = true;
                }
            }
        }
    }

    return !matched;
}
