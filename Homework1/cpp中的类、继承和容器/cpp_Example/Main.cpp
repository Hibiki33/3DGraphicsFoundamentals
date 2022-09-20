#pragma once

#include <iostream>
#include <vector>
#include "People.h"
#include "Chinese.h"
using namespace std;

int main() {
    cout << "start" << endl;

    vector<People> list;                                //这是一个容器，或者说是数组

    People a = People(24, "litiansuo");
    list.push_back(a);                                  //向数组中添加People类的一个实例a

    Chinese b = Chinese(18, "sunxiaochuan", "sichuan");
    b.useChopsticks();
    cout << b.province << endl;

    People c = b;                                       //对b进行上转为People
    b.useChopsticks();                                  //上转后b仍然可以使用子类的方法
    list.push_back(c);                                  //将b添加到list中

    for (People x : list) {
        cout << x.age << x.name << endl;
        x.eat();
    }
    return 0;
}
