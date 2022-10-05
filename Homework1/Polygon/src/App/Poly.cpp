#include "Poly.h" 
#include <vector>
#include <qdebug.h>

Poly::Poly() 
{
}

Poly::~Poly()
{
}

void Poly::Draw(QPainter& painter) 
{
    painter.drawLine(temp, end);
    int n = lineList.size();
    for (int i = 0; i < n; i++) {
        painter.drawLine(lineList[i]);
    }
}

void Poly::set_Line(QLine l)
{
    lineList.push_back(l);
}

void Poly::set_temp(QPoint t)
{
    temp = t;
}

QPoint Poly::get_temp()
{
    return temp;
}

