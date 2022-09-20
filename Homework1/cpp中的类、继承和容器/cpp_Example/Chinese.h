#pragma once

#include "People.h"

class Chinese : public People {
public:
    char* province;             //子类有自己的属性
public:
    Chinese();
    Chinese(int, char*, char*);
    ~Chinese();
    void eat();                 //子类可以重写父类的方法
    void useChopsticks();       //子类可以有自己的方法
};