#include "People.h"


People::People(int age, char* name) {
    this->age = age;
    this->name = name;
    cout << "pos1" << endl;
}                                        //重载构造方法使得可以进行一定操作

People::~People() { }

void People::eat() {
    cout << "People need to eat." << endl;
}