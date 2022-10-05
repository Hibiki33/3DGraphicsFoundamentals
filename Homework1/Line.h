#pragma once

#include "Shape.h"

class myLine : public Shape {
public:
	myLine();
	~myLine();

	void Draw(QPainter& painter);
	void myDrawLine(QPoint s, QPoint e, QPainter& painter);
};