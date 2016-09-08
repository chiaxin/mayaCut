#include "dirEdit.h"
#include <QLineEdit>
#include <QMouseEvent>
#include <QDragEnterEvent>
#include <QProcess>
#include <QDropEvent>
#include <QMimeData>
#include <QFileInfo>
#include <QDebug>
#include <QDir>

DirEdit::DirEdit(QWidget * parent)
	: QLineEdit(parent)
{
	this->setDragEnabled(true);
	this->setAcceptDrops(true);
}

DirEdit::~DirEdit()
{
}

void DirEdit::mouseDoubleClickEvent(QMouseEvent * event)
{
	if (!this->text().isEmpty() && event->button() == Qt::LeftButton)
	{
		QString path = QDir::toNativeSeparators(this->text());
		qDebug() << path;
		QProcess::startDetached("explorer.exe", QStringList() << path);
		event->accept();
	}
	else
	{
		event->ignore();
	}
}

void DirEdit::dragEnterEvent(QDragEnterEvent * event)
{
	if (event->mimeData()->hasText())
	{
		event->accept();
	}
	else
	{
		event->accept();
	}
}

void DirEdit::dropEvent(QDropEvent * event)
{
	QString dropText = event->mimeData()->text().remove("file:///");
	QFileInfo fileinfo(dropText);
	if (fileinfo.isDir())
	{
		event->acceptProposedAction();
		qDebug() << dropText;
		this->setText(dropText);
	}
	else
	{
		event->ignore();
	}
}
