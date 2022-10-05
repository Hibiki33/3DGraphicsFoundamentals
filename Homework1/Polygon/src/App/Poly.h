#pragma once
#include <vector>
#include "Shape.h"

class Poly : public Shape {
private:
	QVector<QLine> lineList;
	QPoint temp;


public:
	Poly();
	~Poly();

	void Draw(QPainter& painter);
	void set_Line(QLine l);
	void set_temp(QPoint t);
	QPoint get_temp();
};