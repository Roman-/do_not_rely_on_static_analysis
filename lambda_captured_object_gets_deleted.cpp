#include <functional>
#include <iostream>

std::function<void()> print_hello_world;

void set_callback(std::function<void()> cb) {
    print_hello_world = cb;
}

void init() {
    std::string hello_world = "hello world!\n";
    set_callback([&](){std::cout << hello_world;});
}

int main(int argc, char *argv[]) {
    init();
    print_hello_world();
    return 0;
}
