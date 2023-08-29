#include <iostream>
#include <unordered_map>

using namespace std;

int main(int argc, char *argv[]) {
    std::unordered_map<int, int> my_map = {{1, 10}, {2, 20}, {3, 30}};

    for (const auto& p : my_map) {
        if (p.first == 2) {
            my_map.erase(p.first);
        }
    }
    std::cout << my_map.size() << std::endl;
    return 0;
}
