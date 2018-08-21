#include <iostream>
#include <vector>
#include <deque>
#include <utility>
#include <list>

int main() {
    int count_ver;
    int count_let;
    int count_term;
    int cur = 0;
    int mi = 97;
    std::cin >> count_ver;
    std::cin >> count_term;
    std::cin >> count_let;
    std::vector<bool> is_terminal(count_ver, false);
    for (size_t ind = 0; ind < count_term; ++ind) {
        std::cin >> cur;
        is_terminal[cur] = true;
    }
    std::vector<std::vector<int>> automat(count_ver,
                    std::vector<int> (count_let, 0));
    std::vector<std::vector<std::list<int>>> automat_rev(count_ver,
                         std::vector<std::list<int>> (count_let));
    int from, to;
    char let;
    for (size_t ind = 0; ind < count_ver; ++ind) {
        for (size_t jind = 0; jind < count_let; ++jind) {
            std::cin >> from;
            std::cin >> let;
            std::cin >> to;
            automat[from][static_cast<int> (let) - mi] = to;
            automat_rev[to][static_cast<int> (let) - mi].push_back(from);
        }
    }
 /*   if (count_term == 0) {
        std::cout << 0;
        return 0;
    }*/
    std::deque<int> deq;
    deq.push_back(0);
    int ver;
    std::vector<bool> used(count_ver, false);
    used[0] = true;
    while (!deq.empty()) {
        ver = deq.front();
        deq.pop_front();
        for (size_t ind = 0; ind < count_let; ++ind) {
            if (!used[automat[ver][ind]]) {
                deq.push_back(automat[ver][ind]);
                used[automat[ver][ind]] = true;
            }
        }
    }
    bool is_dest = false;
    for (size_t ind = 0; ind < count_ver; ++ind) {
        if (used[ind] && is_terminal[ind]) {
            is_dest = true;
        }
    }
    if (!is_dest) {
        std::cout << 1;
        return 0;
    }
    std::deque<std::pair<int, int>> de;
    std::vector<std::vector<bool>> marked(count_ver,
            std::vector<bool> (count_ver, 0));
    for (size_t ind = 0; ind < count_ver; ++ind) {
        for (size_t jind = 0; jind < count_ver; ++jind) {
            if (!marked[ind][jind] &&
                    (is_terminal[ind] != is_terminal[jind])) {
                marked[ind][jind] = marked[jind][ind] = true;
                de.push_back(std::make_pair(ind, jind));
            }
        }
    }
    int uer = 0;
    while (!de.empty()) {
        ver = de.front().first;
        uer = de.front().second;
        de.pop_front();
        for (size_t ind = 0; ind < count_let; ++ind) {
            for (auto re: automat_rev[ver][ind]) {
                for (auto se: automat_rev[uer][ind]) {
                    if (!marked[re][se]) {
                        marked[re][se]=marked[se][re] = true;
                        de.push_back(std::make_pair(re, se));
                    }
                }
            }
        }
    }
    std::vector<int> component(count_ver, -1);
    for (size_t ind = 0; ind < count_ver; ++ind) {
        if (!marked[0][ind]) {
            component[ind] = 0;
        }
    }
    int component_count = 0;
    for (size_t ind = 0; ind < count_ver; ++ind) {
        if (used[ind] && (component[ind] == -1)) {
            component_count += 1;
            component[ind] = component_count;
            for (size_t jind = ind + 1;
                        jind < count_ver; ++jind) {
                if (!marked[ind][jind]) {
                    component[jind] = component_count;
                }
            }
        }
    }
    std::cout << component_count + 1;
    return 0;
}
