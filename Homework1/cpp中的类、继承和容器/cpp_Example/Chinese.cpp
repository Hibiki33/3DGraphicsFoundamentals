#include "Chinese.h"

Chinese::Chinese() {}

Chinese::Chinese(int age, char* name, char* province) {
    this->age = age;
    this->name = name;
    this->province = province;
}                                       

Chinese::~Chinese() { }
    
void Chinese::eat() {
    cout << "People need to eat." << endl;
    cout << "Chinese need to eat rice." << endl;
}

void Chinese::useChopsticks() {
    cout << "Chinese can use chopsticks." << endl;
}