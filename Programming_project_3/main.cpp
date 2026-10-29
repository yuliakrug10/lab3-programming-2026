#include <iostream>
#include <thread>
#include <vector>
#include <chrono>
#include <syncstream>
#include <latch>

using namespace std;

struct Task {
    char name;
    int id;
};

latch La(7), Lb(8), Lc(8), Ld(9), Le(8), Lg(5), Lh(4), Li(8);

void f(char name, int num, int thread_id) {
    this_thread::sleep_for(1s);
    osyncstream(cout) << "(Thread " << thread_id << ") from list " << name << " completed action - " << num << ".\n";
}

void wait_dependencies(char node) {
    switch (node) {
    case 'b': case 'c': case 'd': La.wait(); break;
    case 'e': Lb.wait(); break;
    case 'g': case 'h': Lc.wait(); break;
    case 'i': Ld.wait(); break;
    case 'j': Le.wait(); Lg.wait(); break;
    case 'k': Lh.wait(); Li.wait(); break;
    default: break;
    }
}

void signal_completion(char node) {
    switch (node) {
    case 'a': La.count_down(); break;
    case 'b': Lb.count_down(); break;
    case 'c': Lc.count_down(); break;
    case 'd': Ld.count_down(); break;
    case 'e': Le.count_down(); break;
    case 'g': Lg.count_down(); break;
    case 'h': Lh.count_down(); break;
    case 'i': Li.count_down(); break;
    default: break;
    }
}

void worker(int thread_id, const vector<Task>& tasks) {
    for (const Task& t : tasks) {
        wait_dependencies(t.name);
        f(t.name, t.id, thread_id);
        signal_completion(t.name);
    }
}

int main() {
    const int nt = 7;

    const vector<vector<Task>> plan = {
        {{'a',1},{'b',1},{'b',8},{'c',7},{'d',6},{'e',4},{'g',3},{'i',1},{'i',8},{'j',7}},
        {{'a',2},{'b',2},{'c',1},{'c',8},{'d',7},{'e',5},{'g',4},{'i',2},{'j',1},{'j',8}},
        {{'a',3},{'b',3},{'c',2},{'d',1},{'d',8},{'e',6},{'g',5},{'i',3},{'j',2},{'k',1}},
        {{'a',4},{'b',4},{'c',3},{'d',2},{'d',9},{'e',7},{'h',1},{'i',4},{'j',3},{'k',2}},
        {{'a',5},{'b',5},{'c',4},{'d',3},{'e',1},{'e',8},{'h',2},{'i',5},{'j',4},{'k',3}},
        {{'a',6},{'b',6},{'c',5},{'d',4},{'e',2},{'g',1},{'h',3},{'i',6},{'j',5},{'k',4}},
        {{'a',7},{'b',7},{'c',6},{'d',5},{'e',3},{'g',2},{'h',4},{'i',7},{'j',6}},
    };

    cout << "calculation started\n";
    auto start_time = chrono::steady_clock::now();

    {
        vector<jthread> threads;
        for (int i = 0; i < nt; ++i) {
            threads.emplace_back(worker, i + 1, cref(plan[i]));
        }
    }

    auto end_time = chrono::steady_clock::now();
    auto duration = chrono::duration_cast<chrono::seconds>(end_time - start_time).count();

    cout << "calculation complete, duration = " << duration << " seconds\n";

    return 0;
}