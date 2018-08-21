#include <iostream>
#include <string>
#include <vector>
#include <algorithm>


int main() {
    std::string str;
    std::cin >> str;
    std::vector<size_t> z_function(str.size());
    z_function[0] = str.size();
    size_t left = 0;
    size_t right = 0;
    for (size_t ind = 1; ind < str.size(); ++ind) {
        if (ind > right) {
            size_t index = 0;
            while (index != (str.size() - ind) && str[index] == str[ind+index]) {
                ++index;
            }
            z_function[ind] = index;
        } else {
            size_t index = std::min(right - ind + 1, z_function[ind - left]);
            while (index != (str.size() - ind) && str[index] == str[ind+index]) {
                ++index;
            }
            z_function[ind] = index;
        }
        if (ind + z_function[ind] - 1 > right) {
            left = ind, right = ind + z_function[ind] - 1;
        }
    }
    for (size_t ind = 0; ind < str.size(); ++ind) {
        std::cout << z_function[ind] << " ";
    }
}
