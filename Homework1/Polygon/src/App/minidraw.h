#pragma once

#include <ui_minidraw.h>
#include <viewwidget.h>

#include <QtWidgets/QMainWindow>
#include <qmessagebox.h>

class MiniDraw : public QMainWindow {
	Q_OBJECT

public:
	MiniDraw(QWidget* parent = 0);
	~MiniDraw();

	QMenu* pMenu;
	QToolBar* pToolBar;
	QAction* Action_Line;
	QAction* Action_Rect;
	QAction* Action_Poly;
	QWidget* ToolBar_Spacer;
	QAction* Action_About;

	void Creat_ToolBar();
	void Creat_Action();
	void AboutBox();

private:
	Ui::MiniDrawClass ui;
	ViewWidget* view_widget_;
};
