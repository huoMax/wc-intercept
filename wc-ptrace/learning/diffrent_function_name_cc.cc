#include <iostream>

int test1(int a, int b) {
    return a+b;
}

int test2(int a) {
    return a;
}

class test3 {
public:
    test3() {
        a = 0;
        b = 0;
    }
    ~test3() {}

    int test1(int c, int d) {
        return c + d;
    }
private:
    int a, b;
};

int main(){
    test1(2, 3);
    test2(2);
    test3 t3;
    t3.test1(2, 3);
    return 0;
}