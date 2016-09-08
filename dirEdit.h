#ifndef _DIR_EDIT_H
#define _DIR_EDIT_H
#include <QLineEdit>
#include <QMouseEvent>
#include <QDragEnterEvent>
#include <QDropEvent>

class DirEdit :public QLineEdit
{
	Q_OBJECT
public:
	explicit DirEdit(QWidget * parent = 0);
	virtual ~DirEdit();
	void mouseDoubleClickEvent(QMouseEvent *) override;
	void dragEnterEvent(QDragEnterEvent *) override;
	void dropEvent(QDropEvent *) override;
private:
};
#endif
