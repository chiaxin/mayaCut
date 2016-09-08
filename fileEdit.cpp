#include "fileEdit.h"
#include <QDropEvent>
#include <QDragEnterEvent>
#include <QFileInfo>
#include <QMimeData>
#include <QString>
#include <QDebug>
#include <QDir>
#include <QProcess>

FileEdit::FileEdit(QWidget * parent) :
	QLineEdit(parent)
{
	// Set drag and drop enable
	this->setAcceptDrops(true);
	this->setDragEnabled(true);
	// Connect
	connect(this, &QLineEdit::textEdited, [&](const QString & text) 
	{
		this->setText(QDir::toNativeSeparators(text));
	});
}

FileEdit::~FileEdit()
{
}
void FileEdit::dragEnterEvent(QDragEnterEvent * event)
{
	if (event->mimeData()->hasText())
	{
		qDebug() << "Drag Enter Event Launched";
		event->accept();
	}
	else
	{
		event->ignore();
	}
}

void FileEdit::mouseDoubleClickEvent(QMouseEvent * event)
{
	if (event->button() == Qt::LeftButton)
	{
		QFileInfo fileinfo(this->text());
		QString path = QDir::toNativeSeparators(fileinfo.absoluteDir().path());
		qDebug() << path;
		QProcess::startDetached("explorer.exe", QStringList() << path);
		event->accept();
	}
	else
	{
		event->ignore();
	}
}
void FileEdit::dropEvent(QDropEvent * event)
{
	if (event->mimeData()->hasText())
	{
		event->acceptProposedAction();
		QString dropText = event->mimeData()->text().remove("file:///");
		qDebug() << dropText;
		QFileInfo fileinfo(dropText);
		// If dropped text is file, and ends with ".ma" or ".mb"
		if( fileinfo.isFile() &&
			(dropText.endsWith(".ma", Qt::CaseInsensitive)
			|| dropText.endsWith(".mb", Qt::CaseInsensitive)))
		{
			this->setText(dropText);
			event->accept();
		}
		else
		{
			qDebug() << "This file is not maya file!";
			event->ignore();
		}
	}
	else
	{
		qDebug() << "Failed!";
		event->ignore();
	}
}
