#include <iostream>
#include <vector>
#include <map>
#include <algorithm>
#include <string>
#include <queue>

int code(char c) {
    return c - 97;
}

int main() {
    int n_count;
    std::cin >> n_count;
    int count_strings;
    std::cin >> count_strings;
    int letters;
    std::cin >> letters;
    if (count_strings == 0) {
        int64_t sum = 1;
        for (size_t ind = 0; ind < n_count; ++ind) {
            sum *= letters;
            sum = sum % 1000000007;
        }
        std::cout << sum;
        return 0;
    }
    std::vector<std::string> strings(count_strings);
    for (auto& item: strings) {
        std::cin >> item;
    }
    int length = 1000;
    std::vector<bool> is_end(length*count_strings, false);
    std::vector<std::vector<int>> to =
       std::vector<std::vector<int>> (length * count_strings + 1,
       std::vector<int>(letters, -1));
    std::vector<int> links(count_strings * length);
    std::vector<int> que(count_strings * length);
    int size_bor = 1;
    for (size_t index = 0; index < count_strings; ++index) {
        int ver = 0;
        for (auto ca: strings[index]) {
            if (to[ver][code(ca)] == -1) {
                to[ver][code(ca)] = size_bor++;
            }
            ver = to[ver][code(ca)];
        }
        is_end[ver] = true;
    }
    links[0] = -1;
    int start = 0, finish = 1;
    que[0] = 0;
    while (start < finish) {
        int ver = que[start++];
        for (size_t  ind = 0; ind < to[ver].size(); ++ind) {
            if (to[ver][ind] != -1) {
                int u_ver = to[ver][ind];
                int ch = ind;
                int j_ind = links[ver];
                while (j_ind != -1 && to[j_ind][ch] == -1) {
                    j_ind = links[j_ind];
                }
                if (j_ind != -1) {
                    links[u_ver] = to[j_ind][ch];
                }
                que[finish++] = u_ver;
            }
        }
    }
    std::vector<std::vector<int>> links_letter =
            std::vector<std::vector<int>> (size_bor,
                                 std::vector<int>(letters, 0));
    links[0] = 0;
    std::queue<int> qu;
    std::string alphabeth = "abcdefghijklmnopqrstuvwxyz";
    alphabeth = alphabeth.substr(0, letters);
    qu.push(0);
    std::vector<bool> checked(size_bor, false);
    int current = 0;
    checked[0] = true;
    while (!qu.empty()) {
        current = qu.front();
        qu.pop();
        links_letter[current] = links_letter[links[current]];
        for (char ch: alphabeth) {
            if (to[current][code(ch)] != -1) {
            if (to[links_letter[current][code(ch)]][code(ch)] != -1 &&
                        is_end[to[links_letter[current][code(ch)]][code(ch)]]) {
                    is_end[to[current][code(ch)]] = true;
                }
                links_letter[current][code(ch)] = current;
            }
        }
        for (int ind = 0; ind < to[current].size(); ++ind) {
            if (to[current][ind] != -1 && !checked[to[current][ind]]) {
                qu.push(to[current][ind]);
                checked[to[current][ind]] = true;
            }
        }
    }
    std::vector<std::vector<int64_t >> dp(n_count + 1,
                                     std::vector<int64_t>(size_bor, 0));
    dp[0][0] = 1;
    for (size_t len = 0; len < n_count; ++len) {
        for (size_t ind = 0; ind < size_bor; ++ind) {
            if (!is_end[ind]) {
                for (auto ch: alphabeth) {
                    int ver = links_letter[ind][code(ch)];
                    if (ver == 0 && to[0][code(ch)] == -1) {
                        dp[len + 1][0] += dp[len][ind];
                        dp[len + 1][0] = dp[len + 1][0] % 1000000007;
                    } else if (to[ver][code(ch)] != -1 &&
                            !is_end[to[ver][code(ch)]]) {
                        dp[len + 1][to[ver][code(ch)]] += dp[len][ind];
                        dp[len + 1][to[ver][code(ch)]] =
                                dp[len + 1][to[ver][code(ch)]] % 1000000007;
                    }
                }
            }
        }
    }
    long long sum = 0;
    for (size_t ind = 0; ind < size_bor; ++ind) {
        sum = (sum %1000000007 +  dp[n_count][ind]%1000000007)%1000000007;
    }
    std::cout << sum;
    return 0;
}
