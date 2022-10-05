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
 * ���Ӻ��� ��connect(����1, ����2, ����3, ����4)
 * ����1 �źŵķ�����
 * ����2 ���͵��źţ�������ַ��
 * ����3 �źŵĽ�����
 * ����4 ����Ĳۺ��� �������ĵ�ַ��
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
