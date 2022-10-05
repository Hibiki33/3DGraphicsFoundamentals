#include "viewwidget.h"
#include <qdebug.h>

ViewWidget::ViewWidget(QWidget* parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	draw_status_ = false;
	shape_ = NULL;
	type_ = Shape::kDefault;
	isDrawingPolygon = false;
}

ViewWidget::~ViewWidget()
{
}

void ViewWidget::setLine()
{
	type_ = Shape::kLine;
}

void ViewWidget::setRect()
{
	type_ = Shape::kRect;
}

void ViewWidget::setPoly()
{
	type_ = Shape::kPoly;
}

void ViewWidget::mousePressEvent(QMouseEvent* event)
{

	if (Qt::LeftButton == event->button() && !isDrawingPolygon)
	{
		switch (type_)
		{
		case Shape::kLine:
			shape_ = new Line();
			break;
		case Shape::kDefault:
			break;

		case Shape::kRect:
			shape_ = new Rect();
			break;
		case Shape::kPoly:
			shape_ = new Poly();
			break;
		}
		if (shape_ != NULL)
		{
			//qDebug() << "2" << endl;
			draw_status_ = true;
			start_point_ = end_point_ = event->pos();
			shape_->set_start(start_point_);
			shape_->set_end(end_point_);
		}
		
		if (type_ = Shape::kPoly)
		{
			isDrawingPolygon = true;
			((Poly*)shape_)->set_temp(event->pos());
			setMouseTracking(true);
		}
		
	}
	
	else if (Qt::LeftButton == event->button() && isDrawingPolygon)
	{
		//qDebug() << (shape_ == NULL) << endl;
		if (shape_ != NULL && draw_status_) 
		{
			//qDebug() << "3" << endl;
			end_point_ = event->pos();
			shape_->set_end(end_point_);
			//qDebug() << "4" << endl;
			((Poly*)shape_)->set_Line(QLine(((Poly*)shape_)->get_temp(), end_point_));

			if (pow(start_point_.x() - end_point_.x(), 2) + pow(start_point_.y() - end_point_.y(), 2) < 25)
			{
				((Poly*)shape_)->set_Line(QLine(start_point_, end_point_));
				draw_status_ = false;
				isDrawingPolygon = false;
				shape_list_.push_back(shape_);
				shape_ = NULL;
				setMouseTracking(false);
			}
			else
			{
				((Poly*)shape_)->set_temp(end_point_);
			}
		}
	}
	
	update();
}

void ViewWidget::mouseMoveEvent(QMouseEvent* event)
{
	
	if (draw_status_ && shape_ != NULL)
	{

		end_point_ = event->pos();
		//qDebug() << end_point_ << endl;
		shape_->set_end(end_point_);
	}
}

void ViewWidget::mouseReleaseEvent(QMouseEvent* event)
{
	if (shape_ != NULL && !isDrawingPolygon)
	{
		draw_status_ = false;
		shape_list_.push_back(shape_);
		shape_ = NULL;
	}
}

void ViewWidget::paintEvent(QPaintEvent*)
{
	QPainter painter(this);

	for (int i = 0; i < shape_list_.size(); i++)
	{
		shape_list_[i]->Draw(painter);
	}

	if (shape_ != NULL) {
		shape_->Draw(painter);
	}

	update();
}