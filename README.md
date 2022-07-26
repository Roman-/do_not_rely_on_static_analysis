# static analysis is feeble
The purpose of this repository is to collect C++ code examples containing bugs that can be seemingly easily detected with static analysis tools. Each example application either crash or may crash, and one might expect that the reason for the crash must be detected with static analysis. Yet none the tools from the list below has detected any problems in none of the programs from this repo.

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

# unspecified evaluation order
## level 1
Quite obvious and easily detected with `-Wunsequenced` clang diagnostics:
```cpp
void foo(int a, int b) {
    std::cout << a << ", " << b << std::endl;
}

int main(int argc, char *argv[]) {
    int i = 0;
    foo(++i, --i);
}
```

## level 2
Quite obvious but and also detected with `clang -std=c++14 -Wunsequenced`:
```cpp
int main(int argc, char *argv[]) {
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

int main(int argc, char *argv[]) {
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

int main(int argc, char *argv[]) {
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

int main(int argc, char *argv[]) {
    std::function<int()> f;
    maybe_initialize(f);
    return f();
}
```
