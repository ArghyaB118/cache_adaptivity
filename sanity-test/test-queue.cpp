#include <iostream>
#include <string>
// #include <ranges>
#include <queue>

using namespace std;

class comp {
public:
    bool operator () (const int& a, const int& b) const { return (a > b); } 
};

void myPrint (vector<int>& v) {
    for (auto& e : v)
        cout << e << " ";
    cout << endl;
}

int main (int argc, char *argv[]) {
    std::priority_queue<int, vector<int>, greater<int>> pq;
    for (auto i = 4; i >= 1; i--)
        pq.push(i);
    vector<int> v = {3, 1, 4, 1, 5, 9};
    comp c;
    std::make_heap(v.begin(), v.end(), c);
    
    myPrint (v);

    // for (int i = 0; i < 1; i++)
    pop_heap(v.begin(), v.end(), c); v.pop_back();

    myPrint (v);

    vector<int> u = {11, 14, 12};
    v.push_back (11);

    myPrint (v);

    push_heap (v.begin(), v.end(), c);
    myPrint (v);

    // pq.push_range(v);
    while(!pq.empty()) {
        cout << pq.top() << endl;
        pq.pop();
    }
    return 0;
}
