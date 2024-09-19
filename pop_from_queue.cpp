#include <iostream>
#include <queue>

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

int main(int argc, char *argv[]) {
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
