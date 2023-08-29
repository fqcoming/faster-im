
#include <string>
#include <iostream>
using namespace std;


int main() {

    char x[1024] = {'h', 'e', 'l'};
    string s(x, 3);
    cout << s.size() << ":" << s << endl;
    x[0] = 'f';
    cout << s.size() << ":" << s << endl;
    cout << x << endl;

    return 0;
}















