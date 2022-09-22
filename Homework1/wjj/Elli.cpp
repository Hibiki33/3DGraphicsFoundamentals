#include "Elli.h"

Elli::Elli()
{
}

Elli::~Elli() 
{
}

void Elli::Draw(QPainter& painter) 
{
	//painter.drawEllipse(start.x(), start.y(), end.x() - start.x(), end.y() - start.y());
    double a = fabs(start.x() - end.x()) / 2;
    double b = fabs(start.y() - end.y()) / 2;
    double x = 0; 
    double y = b;
    double ox = (start.x() + end.x()) / 2;
    double oy = (start.y() + end.y()) / 2;
    painter.drawPoint(x + ox, y + oy);
    painter.drawPoint(-x + ox, y + oy);
    painter.drawPoint(x + ox, -y + oy);
    painter.drawPoint(-x + ox, -y + oy);

    double d1 = b * b + a * a * (-b + 0.25);
    while (b * b * (x + 1) < a * a * (y - 0.5))
    {
        if (d1 < 0)
        {
            d1 += b * b * (2 * x + 3);
        }
        else
        {
            d1 += b * b * (2 * x + 3) + a * a * (-2 * y + 2);
            y--;
        }
        x++;
        painter.drawPoint(x + ox, y + oy);
        painter.drawPoint(-x + ox, y + oy);
        painter.drawPoint(x + ox, -y + oy);
        painter.drawPoint(-x + ox, -y + oy);
    }
    double d2 = b * b * (x + 0.5) * (x + 0.5) + a * a * (y - 1) * (y - 1) - a * a * b * b;
    while (y > 0)
    {
        if (d2 < 0)
        {
            d2 += b * b * (2 * x + 2) + a * a * (-2 * y + 3);
            x++;
        }
        else
        {
            d2 += a * a * (-2 * y + 3);
        }
        y--;
        painter.drawPoint(x + ox, y + oy);
        painter.drawPoint(-x + ox, y + oy);
        painter.drawPoint(x + ox, -y + oy);
        painter.drawPoint(-x + ox, -y + oy);
    }
}