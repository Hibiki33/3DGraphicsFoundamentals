#include "minidraw.h"

#include <QToolBar>

MiniDraw::MiniDraw(QWidget* parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);
	view_widget_ = new ViewWidget();
	Creat_Action();
	Creat_ToolBar();
	

	setCentralWidget(view_widget_);
}

void MiniDraw::Creat_Action() {
	Action_About = new QAction(tr("&About"), this);
	connect(Action_About, &QAction::triggered, this, &MiniDraw::AboutBox);

	Action_Line = new QAction(tr("&Line"), this);
	connect(Action_Line, SIGNAL(triggered()), view_widget_, SLOT(setLine()));

	Action_Rect = new QAction(tr("&Rectangle"), this);
	connect(Action_Rect, &QAction::triggered, view_widget_, &ViewWidget::setRect);

	Action_Poly = new QAction(tr("&Polygon"), this);
	connect(Action_Poly, &QAction::triggered, view_widget_, &ViewWidget::setPoly);

/**
 * 
 * 连接函数 ：connect(参数1, 参数2, 参数3, 参数4)
 * 参数1 信号的发送者
 * 参数2 发送的信号（函数地址）
 * 参数3 信号的接受者
 * 参数4 处理的槽函数 （函数的地址）
 * 
 */
}

void MiniDraw::Creat_ToolBar() {
	pToolBar = addToolBar(tr("&Main"));
	pToolBar->setMovable(false);
	pToolBar->addAction(Action_Line);
	pToolBar->addAction(Action_Rect);
	pToolBar->addAction(Action_Poly);
	ToolBar_Spacer = new QWidget();
	ToolBar_Spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	pToolBar->addWidget(ToolBar_Spacer);
	pToolBar->addAction(Action_About);
}


void MiniDraw::AboutBox() {
	QMessageBox::about(this, tr("About"), tr("MiniDraw"));
}

MiniDraw::~MiniDraw() {}
