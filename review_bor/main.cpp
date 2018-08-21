#include <iostream>
#include <sstream>
#include <stdio.h>
#include <vector>
#include <map>
#include <algorithm>
#include <string>
#include <queue>
int code(char c) {
    return c - 97;
}

int main() {
    int count_letters = 26;
    std::string alphabeth = "abcdefghijklmnopqrstuvwxyz";
    std::string p_string;
    std::string t_string;
    std::getline(std::cin, p_string);
    std::getline(std::cin, t_string);
    if (p_string.size() > t_string.size()) {
        std::cout << 0;
        return 0;
    }
    std::vector<std::string> patterns;
    std::vector<int> patterns_index;
    std::vector<int> pattern_length;
    size_t prev_pos = 0;
    std::string pattern;
    std::vector<int> count(t_string.size(), 0);
    for (size_t pos = 0; pos <  p_string.size(); ++pos) {
        if (p_string[pos] == '?') {
            pattern = p_string.substr(prev_pos, pos-prev_pos);
            if (pattern.size() != 0) {
                patterns.emplace_back(pattern);
                patterns_index.emplace_back(prev_pos);
                pattern_length.emplace_back(pattern.size());
            }
            prev_pos = pos + 1;
        }
    }
    if (p_string[p_string.size() - 1] != '?' && prev_pos != p_string.size()) {
        pattern = p_string.substr(prev_pos, p_string.size() - prev_pos);
        if (pattern.size() != 0) {
            patterns.emplace_back(pattern);
            patterns_index.emplace_back(prev_pos);
            pattern_length.emplace_back(pattern.size());
        }
    }
    for (size_t index_pattern = 0; index_pattern < patterns.size(); ++index_pattern) {
        std::string current_pattern = patterns[index_pattern];
        int enter_index = patterns_index[index_pattern];
        int bor_size = current_pattern.size() + 1;
        std::vector<std::vector<int>> to(bor_size, std::vector<int>(alphabeth.size(), -1));
        for (size_t index = 0; index < current_pattern.size(); ++index) {
            to[index][code(current_pattern[index])] = index + 1;
        }
        std::vector<int> suffix_reference(current_pattern.size() + 1);
        suffix_reference[0] = -1;
        for (int vertice = 0; vertice < bor_size; ++vertice) {
            for (size_t index = 0; index < to[vertice].size(); ++index) {
                int to_vertice = to[vertice][index];
                if (to_vertice != -1) {
                    int suffix = suffix_reference[vertice];
                    while (suffix != -1 && to[suffix][index] == -1) {
                        suffix = suffix_reference[suffix];
                    }
                    if (suffix != -1) {
                        suffix_reference[to_vertice] = to[suffix][index];
                    }
                }
            }
        }
        std::vector<std::vector<int>> suffix_letter =
                std::vector<std::vector<int>>(bor_size,
                                              std::vector<int>(count_letters, 0));
        suffix_reference[0] = 0;
        std::queue<int> qu;
        qu.push(0);
        std::vector<bool> checked(bor_size, false);
        int current_verice = 0;
        checked[0] = true;
        while (!qu.empty()) {
            current_verice = qu.front();
            qu.pop();
            suffix_letter[current_verice] =
                    suffix_letter[suffix_reference[current_verice]];
            for (char letter: alphabeth) {
                if (to[current_verice][code(letter)] != -1) {
                    suffix_letter[current_verice][code(letter)] = current_verice;
                }
            }
            for (int ind = 0; ind < count_letters; ++ind) {
                if (to[current_verice][ind] != -1 &&
                        !checked[to[current_verice][ind]]) {
                    qu.push(to[current_verice][ind]);
                    checked[to[current_verice][ind]] = true;
                }
            }
        }
        current_verice = 0;
        std::vector<bool> enter(t_string.size(), 0);
        for (size_t position = 0; position < t_string.size(); ++position) {
            if (to[current_verice][code(t_string[position])] != -1) {
                ++current_verice;
            } else {
                if (current_verice == bor_size - 1) {
                    enter[position - current_pattern.size()] = true;
                }
                current_verice = suffix_letter[current_verice][code(t_string[position])];
                if (to[current_verice][code(t_string[position])] != -1) {
                    current_verice = to[current_verice][code(t_string[position])];
                }
            }
        }
        if (current_verice == bor_size - 1) {
            enter[t_string.size() - current_pattern.size()] = true;
        }
        for (size_t index = enter_index; index < t_string.size(); ++index) {
            if (enter[index]) {
                ++count[index - enter_index];
            }
        }
    }
    int sum = 0;
    for (size_t index = 0; index < count.size(); ++index) {
        if (count[index] == patterns.size()) {
            ++sum;
        }
    }
    std::cout << sum << "\n";
    for (size_t index = 0; index < count.size(); ++index) {
        if (count[index] == patterns.size()) {
            std::cout << index << " ";
        }
    }
    return 0;
}
