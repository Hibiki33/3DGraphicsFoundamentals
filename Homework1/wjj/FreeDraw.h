#pragma once

#include "Shape.h"

class FreeDraw : public Shape {


public:
	FreeDraw();
	~FreeDraw();

	void set_start(QPoint s);
	void set_end(QPoint e);

	QVector<QPoint> pointList;

	void Draw(QPainter& painter);
	int length(QPoint p1, QPoint p2);
};