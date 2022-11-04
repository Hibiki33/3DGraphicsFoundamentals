#include "FreeDraw.h" 
#include <vector>

FreeDraw::FreeDraw() { }
FreeDraw::~FreeDraw() { }


void FreeDraw::set_start(QPoint s)									//这里对Shape类的set_start()重写了
{																	//需要将Shape.h内的set_Start()设为virtual
	start = s;
	pointList.push_back(s);
}

void FreeDraw::set_end(QPoint e)
{
	end = e;
	pointList.push_back(e);
}

void FreeDraw::Draw(QPainter& painter) {
	for (int i = 0; i < pointList.size() - 1; i++) 
	{
		if (length(pointList[i], pointList[i + 1]) > 3)				//当两点间距离过大时绘制直线防止间断
		{
			painter.drawLine(pointList[i], pointList[i + 1]);
		}
		else														//否则绘制点
		{
			painter.drawPoint(pointList[i]);
		}	
	}
}

int FreeDraw::length(QPoint p1, QPoint p2)
{
	return (p1.x() - p2.x()) * (p1.x() - p2.x()) +
		(p1.y() - p2.y()) * (p1.y() - p2.y());
}
