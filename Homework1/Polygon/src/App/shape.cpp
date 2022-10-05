#include "Shape.h"
#include <qdebug.h>

Shape::Shape()
{
}

Shape::~Shape()
{
}

void Shape::set_start(QPoint s)
{
	start = s;
}

void Shape::set_end(QPoint e)
{
	//qDebug() << e << endl;
	end = e;
}
