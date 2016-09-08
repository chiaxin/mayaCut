#ifndef _MAYA_CUT_H
#define _MAYA_CUT_H
#include <QWidget>
#include <QLineEdit>
#include <QCheckBox>
#include <QComboBox>
#include <QLabel>
#include <QList>
#include <QString>
#include <QProcess>
#include <QStringList>
#include <QCloseEvent>
#include <QHBoxLayout>
#include "fileEdit.h"
#include "dirEdit.h"

class MayaCutUI :public QWidget
{
	Q_OBJECT	
	static const char * OPTION_NAME[];
	static const char * FLAGS[];
public:
	// Constructor
	MayaCutUI(QWidget * parent = 0);
	// Destructor
	virtual ~MayaCutUI();
private:
	// Functions
	void doIt();
	void browseMayaFile();
	void browseDestinationFile();
	void browseTexturePath();
	void writeSettings();
	void readSettings();
	bool checkMayapy();
	void mayaVersionChanged(int);
	void checkMayaVersionFromSource();
	void whenProgramStart();
	void whenProgramFinished(int, QProcess::ExitStatus);
	// Build Functions
	QHBoxLayout * createBrowseField(
		QLineEdit *, QString, void (MayaCutUI::*)());
	// Events
	void closeEvent(QCloseEvent *);
	// Attributes
	QString mayapy_use;
	QStringList * p_mayapy_progs;
	// Process
	QProcess * p_proc;
	// Elements
	QComboBox * p_mayaVersions;
	FileEdit * p_mayaFileSource;
	FileEdit * p_destination;
	DirEdit * p_texturePath;
	QList<QCheckBox *> options;
};
#endif