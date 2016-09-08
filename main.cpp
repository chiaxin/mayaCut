#include <QApplication>
#include <QWidget>
#include <QVBoxLayout>
#include <QDebug>
#include <QLabel>
#include <QPixmap>
#include <QCursor>
#include <QPushButton>
#include "mayaCutUI.h"

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);
	MayaCutUI makeSimpleUI;
	makeSimpleUI.show();
	return app.exec();
}
