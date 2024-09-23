# Do not rely on static analysis
C++ code examples with bugs that should seemingly be easily detected with static analysis tools, yet they are not.

# tools
The list of static analysis tools tested to detect bugs in each of the examples:
- [cppcheck](https://cppcheck.sourceforge.io/)
- [PVS‑Studio](https://pvs-studio.com/)
- clang-tidy (while searching the [list of diagnostics](https://clang.llvm.org/extra/clang-tidy/checks/list.html) for words related to the specific bug)
- clang diagnostics (e.g. [-Wunsequenced](https://clang.llvm.org/docs/DiagnosticsReference.html#wunsequenced) for unspecified_evaluation_order.cpp)

# build
```
mkdir build && cd build
cmake ..
make
```

# unordered map invalidation
As of September 2023, no static analysis tools are able to detect this:
```cpp
int main() {
    std::unordered_map<int, int> my_map = {{1, 10}, {2, 20}, {3, 30}};

    for (const auto& p : my_map) {
        if (p.first == 2) {
            my_map.erase(p.first);
        }
    }
    std::cout << my_map.size() << std::endl;
    return 0;
}
```
No clang-tidy checks provided for this issue. Cppcheck also show no warnings, and [PVS‑Studio](https://pvs-studio.com/) team mailed me that their [V789](https://pvs-studio.ru/ru/docs/warnings/v789/) diagnostic needs a fix to detect this.

# unspecified evaluation order
## level 1
Quite obvious and is detected with some tools, including `-Wunsequenced` clang diagnostics:
```cpp
void foo(int a, int b) {
    std::cout << a << ", " << b << std::endl;
}

int main() {
    int i = 0;
    foo(++i, --i);
}
```

## level 2
Quite obvious and only detected with `clang -std=c++14 -Wunsequenced`:
```cpp
int main() {
    std::map<int, int> m;
    int i = 0;
    m[++i] = --i;
    return 0;
}
```

## level 3
Isn't so obvious and no detections:
```cpp
#include <unordered_map>
#include <cassert>
#include <memory>
#include <iostream>

struct Object {
};

struct Wrapper {
    std::unique_ptr<Object> object;
    std::string description;
};

class ObjectRegistry {
public:
    void add_object(Wrapper& w) {
        objects_[w.object.get()] = Wrapper{std::move(w.object), "added from " + w.description};

        // this assertion may or may not fail depending on the order of evaluation in the line above, which is unspecified
        assert(objects_.begin()->first != nullptr);
    }

private:
    // In our application, we need to quickly access the wrapper by raw pointer of its object
    std::unordered_map<Object*, Wrapper> objects_;
};

int main() {
    ObjectRegistry registry;
    Wrapper w{std::make_unique<Object>(), "first object"};
    registry.add_object(w);
    return 0;
}
```

# call to uninitialized std::function
This is self-explanatory:
```cpp
#include <functional>

int main() {
    std::function<int()> f;
    return f();
}
```

If you managed to find a static analysis tool that detects a problem here, try with the example where the initialization logic of `f` is non-trivial:
```cpp
void maybe_initialize(std::function<int()>& f) {
    if (rand()%2)
        f = [](){return 42;};
}

int main() {
    std::function<int()> f;
    maybe_initialize(f);
    return f();
}
```
# lambda captured object gets deleted
## level 1
This may print "hello world" or just crash.
PVS-Studio gives V1047 (Lifetime of the lambda is greater than lifetime of the local variable 'hello_world' captured by reference)
```cpp
int main() {
    std::function<void()> print_hello_world;
    {
        std::string hello_world = "hello world!\n";
        print_hello_world = [&](){std::cout << hello_world;};
    }
    print_hello_world();
    return 0;
}
```

## level 2
But here's an example that more closely resembles the real bug found in one of the projects. Neither PVS-Studio nor cppcheck detect the issue.
```cpp
std::function<void()> print_hello_world;

void set_callback(std::function<void()> cb) {
    print_hello_world = cb;
}

void init() {
    std::string hello_world = "hello world!\n";
    set_callback([&](){std::cout << hello_world;});
}

int main() {
    init();
    print_hello_world();
    return 0;
}
```

# pop from queue
Calling `pop()` on the empty queue:
## Level 1
```cpp
int main() {
    std::queue<int> q;
    q.push(42); // q has one element
    q.pop(); // q has no elements
    q.pop(); // UB
    return 0;
}
```

## Level 2
```cpp
void pop_until_3(std::queue<int>& q) {
    while (!q.empty() && q.front() != 3) {
        q.pop();
    }
    if (q.empty()) {
        std::cout << "Number 3 not found, returning...\n";
        // FORGOT TO RETURN HERE
    }
    std::cout << "Found number " << q.front() << "! Let's pop it, too\n";
    q.pop();
    std::cout << "Now queue size is " << q.size() << ".\n";
}

int main() {
    std::queue<int> my_queue;
    my_queue.push(1);
    my_queue.push(2);
    my_queue.push(3);
    my_queue.push(2);
    my_queue.push(1);

    pop_until_3(my_queue); // Found number 3! Let's pop it, too
    pop_until_3(my_queue); // Number 3 not found -> segfault
    return 0;
}
```