#ifndef _FILE_EDIT_H
#define _FILE_EDIT_H
#include <QLineEdit>
#include <QDropEvent>
#include <QMouseEvent>
#include <QDragEnterEvent>

class FileEdit :public QLineEdit
{
	Q_OBJECT
public:
	explicit FileEdit(QWidget * parent = 0);
	virtual ~FileEdit();
	virtual void mouseDoubleClickEvent(QMouseEvent *) override;
	virtual void dragEnterEvent(QDragEnterEvent *) override;
	virtual void dropEvent(QDropEvent *) override;
private:
};

#endif
