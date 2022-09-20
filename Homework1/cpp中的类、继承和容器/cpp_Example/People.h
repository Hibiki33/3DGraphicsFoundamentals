#pragma once
#include <iostream>
#include <string>
using namespace std;

class People {
public:
    int age;
    char* name;                 //父类的属性
public:
    People();                   //这是默认的构造方法
    People(int, char*);         //也可以重载构造方法
    ~People();
    virtual void eat();         //用virtual使得方法可以被子类重写
};

