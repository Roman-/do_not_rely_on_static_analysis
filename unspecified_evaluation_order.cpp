// related: https://stackoverflow.com/questions/73114323/can-unspecified-order-of-evaluation-be-detected-with-static-analysis
#include <unordered_map>
#include <cassert>
#include <memory>
#include <iostream>

struct Object {
    explicit Object(int data) : data_{data} { };
    int data_;
};

struct Wrapper {
    std::unique_ptr<Object> object;
    std::string description;
};

class ObjectRegistry {
    // In our application, we need to quickly access the wrapper by raw pointer of its object
    std::unordered_map<Object*, Wrapper> objects_;

public:
    void add_object(Wrapper& w) {
        objects_[w.object.get()] = Wrapper{std::move(w.object), "added from " + w.description};
        // ^^if "w.object.get()" is evaluated after "std::move(w.object)" ...
    }

    void print_objects_data() {
        for (const auto& pair : objects_) {
            std::cout << pair.first->data_ << '\n'; // ... then you get a segfault here because pair.first is nullptr
        }
    }
};

int main(int argc, char *argv[]) {
    ObjectRegistry registry;
    Wrapper w{std::make_unique<Object>(42), "first object"};
    registry.add_object(w);
    registry.print_objects_data();
    return 0;
}
