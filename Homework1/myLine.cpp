#include "myLine.h"

myLine::myLine() { }

myLine::~myLine() { }

void myLine::Draw(QPainter& painter)
{
	//painter.drawLine(start, end);
	myDrawLine(start, end, painter);
}

void myLine::myDrawLine(QPoint s, QPoint e, QPainter& painter)
{
	int x = s.x();
	int y = s.y();

	int w = e.x() - x;
	int h = e.y() - y;

	int dx1 = w < 0 ? -1 : (w > 0 ? 1 : 0);
	int dy1 = h < 0 ? -1 : (h > 0 ? 1 : 0);

	int dx2 = w < 0 ? -1 : (w > 0 ? 1 : 0);
	int dy2 = 0;

	int fastStep = abs(w);
	int slowStep = abs(h);
	if (fastStep <= slowStep) {
		fastStep = abs(h);
		slowStep = abs(w);

		dx2 = 0;
		dy2 = h < 0 ? -1 : (h > 0 ? 1 : 0);
	}
	int numerator = fastStep >> 1;

	for (int i = 0; i <= fastStep; i++) {
		painter.drawPoint(x, y);
		numerator += slowStep;
		if (numerator >= fastStep) {
			numerator -= fastStep;
			x += dx1;
			y += dy1;
		}
		else {
			x += dx2;
			y += dy2;
		}
	}
}
