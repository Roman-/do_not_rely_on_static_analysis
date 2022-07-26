#include <functional>

int main(int argc, char *argv[]) {
    std::function<int()> f;
    return f();
}

