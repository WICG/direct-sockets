#include <vector>
#include <iostream>

using namespace std;

int main() {
  vector<int> v;
  v.emplace(v.end(), 1);
  cout << v.size();
}
