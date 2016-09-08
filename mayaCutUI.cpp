// QClass
#include <QWidget>
#include <QLineEdit>
#include <QCheckBox>
#include <QPushButton>
#include <QGroupBox>
#include <QLabel>
#include <QList>
#include <QFont>
#include <QDebug>
#include <QDir>
#include <QString>
#include <QStringList>
#include <QFileDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSizePolicy>
#include <QFileInfo>
#include <QProcess>
#include <QMessageBox>
#include <QSettings>
#include <QBitArray>
#include <QByteArray>
#include <QPixmap>
#include <QIcon>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QCloseEvent>
#include <QFile>
#include "mayaCutUI.h"
#include "fileEdit.h"
#include "dirEdit.h"

const QString VERSION(" 0.0.2 Beta");
const QSize WIDGET_SIZE(360, 200);

// Mayapy programs default path
const QStringList MAYAPYS_DEF = {
	"C:/Program Files/Autodesk/Maya2014/bin/mayapy.exe",
	"C:/Program Files/Autodesk/Maya2015/bin/mayapy.exe",
	"C:/Program Files/Autodesk/Maya2016/bin/mayapy.exe",
	"C:/Program Files/Autodesk/Maya2017/bin/mayapy.exe"
};

// Custom python script for maya path(under the program's directory)
const char * PY_SCRIPT = "/python/mayaCut.py";

const char * MayaCutUI::OPTION_NAME[]
{
	"Freeze all",
	"Convert all instance",
	"Copy textures",
	"Delete all intermediate Object",
	"Set all vertex to zero"
};
const char * MayaCutUI::FLAGS[]
{
	"-f", "-i", "-c", "-d", "-z"
};

// UI Font
const QFont COMMON_FONT("Arial");

// Setting key names
const QString INIT_SETTING("./config.ini");
const QString KEY_OPTIONS("makeSimple/options");
const QString KEY_MAYAFILE("makeSimple/mayafile");
const QString KEY_DESTFILE("makeSimple/destfile");
const QString KEY_TEXTUREDIR("makeSimple/texturedir");
const QString KEY_MAYAVERIDX("makeSimple/mayaveridx");
const QString KEY_MAYAPYS("makeSimple/mayapyprogs");

MayaCutUI::MayaCutUI(QWidget * parent) :
	QWidget(parent),
	p_mayaFileSource(new FileEdit),
	p_destination(new FileEdit),
	p_texturePath(new DirEdit),
	p_mayapy_progs(new QStringList),
	p_mayaVersions(new QComboBox()),
	options(QList<QCheckBox *>()),
	mayapy_use(MAYAPYS_DEF.at(0)),
	p_proc(new QProcess(this))
{
	this->setWindowTitle("Make Simple " + VERSION);
	this->setFont(COMMON_FONT);
	this->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	QPixmap titlePixmap(":/icons/title");
	QIcon titleIcon(titlePixmap);
	this->setWindowIcon(titleIcon);
	//
	QVBoxLayout * main_layout = new QVBoxLayout;
	QGroupBox * p_pathGroupBox = new QGroupBox("Paths (Double click to open directory)");
	QVBoxLayout * p_pathLayout = new QVBoxLayout(p_pathGroupBox);
	// Here is for test
	// FileEdit * p_fileEditTest = new FileEdit;
	// p_pathLayout->addWidget(p_fileEditTest);
	// test end
	p_pathLayout->addLayout(createBrowseField(
		p_mayaFileSource, "Maya File", &MayaCutUI::browseMayaFile
	));
	p_pathLayout->addLayout(createBrowseField(
		p_destination, "Destination", &MayaCutUI::browseDestinationFile
	));
	p_pathLayout->addLayout(createBrowseField(
		p_texturePath, "Texture Path", &MayaCutUI::browseTexturePath
	));
	QHBoxLayout * p_lowerLayout = new QHBoxLayout;
	QGroupBox * p_executeGroupBox = new QGroupBox("Execute");
	QVBoxLayout * p_executeLayout = new QVBoxLayout(p_executeGroupBox);
	// Buttons
	QPushButton * p_execute_button = new QPushButton;
	p_execute_button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	QPixmap pixmap(":/icons/execute");
	QIcon btnIcon(pixmap);
	p_execute_button->setIcon(btnIcon);
	p_execute_button->setIconSize(pixmap.rect().size()/2);
	//p_execute_button->setMinimumSize(QSize(90, 60));
	QPushButton * p_cancel_button = new QPushButton("Cancel");
	p_cancel_button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	p_cancel_button->setMinimumSize(QSize(90, 60));
	// ==
	p_executeLayout->addWidget(p_execute_button);
	p_executeLayout->addWidget(p_cancel_button);
	QGroupBox * p_optionGroupBox = new QGroupBox("Options");
	QVBoxLayout * p_optionLayout = new QVBoxLayout(p_optionGroupBox);
	p_lowerLayout->addWidget(p_optionGroupBox);
	p_lowerLayout->addWidget(p_executeGroupBox);
	p_optionLayout->addWidget(p_mayaVersions);
	for (unsigned idx = 0; idx < (sizeof(OPTION_NAME) / sizeof(char*)); idx++)
	{
		options.append(new QCheckBox(OPTION_NAME[idx]));
		QCheckBox * cb = options.at(idx);
		cb->setChecked(true);
		p_optionLayout->addWidget(cb);
	}
	p_mayaVersions->setMinimumSize(QSize(60, 28));
	p_mayaVersions->addItems(
		{ "Maya 2014", "Maya 2015", "Maya 2016", "Maya 2017" });
	main_layout->addWidget(p_pathGroupBox);
	main_layout->addLayout(p_lowerLayout);
	// Connect
	connect(p_execute_button, &QPushButton::clicked,
		this, &MayaCutUI::doIt);
	connect(p_cancel_button, &QPushButton::clicked,
		this, &QWidget::close);
	connect(p_mayaVersions,
		static_cast<void (QComboBox::*)(int)>(
		&QComboBox::currentIndexChanged),
		this, &MayaCutUI::mayaVersionChanged);
	// TEST 
	connect(p_mayaFileSource, &QLineEdit::textChanged, 
		this, &MayaCutUI::checkMayaVersionFromSource);
	//
	connect(p_proc, &QProcess::started, this, &MayaCutUI::whenProgramStart);
	connect(p_proc, 
		static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished), 
		this, &MayaCutUI::whenProgramFinished);
	this->setLayout(main_layout);
	// Resize main widget
	this->resize(WIDGET_SIZE);
	// Read settings from ini
	readSettings();
}

MayaCutUI::~MayaCutUI()
{

}

QHBoxLayout * MayaCutUI::createBrowseField(
	QLineEdit * p_lineEdit, QString name, void (MayaCutUI::*pf)())
{
	const int HEIGHT = 20;
	QHBoxLayout * hlayout = new QHBoxLayout;
	QLabel * label = new QLabel(name);
	// Label
	label->setAttribute(Qt::WA_DeleteOnClose);
	label->setFixedSize(QSize(90, HEIGHT));
	label->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	// LineEdit
	p_lineEdit->setMinimumSize(QSize(120, HEIGHT));
	p_lineEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	// Browse button
	QPushButton * button = new QPushButton("...");
	button->setAttribute(Qt::WA_DeleteOnClose);
	button->setMaximumSize(QSize(24, HEIGHT));
	button->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	// Add widget
	hlayout->addWidget(label);
	hlayout->addWidget(p_lineEdit);
	hlayout->addWidget(button);
	//
	connect(button, &QPushButton::clicked, this, pf);
	//
	return hlayout;
}

void MayaCutUI::browseMayaFile()
{
	QString filename = QFileDialog::getOpenFileName(
		this, "Select Maya File", "D:/", "Maya File (*.ma *.mb)"
	);
	if (!filename.isEmpty())
	{
		p_mayaFileSource->setText(filename);
	}
}

void MayaCutUI::browseDestinationFile()
{
	QString filename = QFileDialog::getSaveFileName(
		this, "Create Maya File", "D:/", "Maya File (*.ma *.mb)"
	);
	if (!filename.isEmpty())
	{
		p_destination->setText(filename);
	}
}

void MayaCutUI::browseTexturePath()
{
	QString dirname = QFileDialog::getExistingDirectory(
		this, "Select Copy Texture Path", "D:/"
	);
	if (!dirname.isEmpty())
	{
		p_texturePath->setText(dirname);
	}
}

void MayaCutUI::mayaVersionChanged(int version_idx)
{
	mayapy_use = p_mayapy_progs->at(version_idx);
	qDebug() << "Mayapy use :" << mayapy_use;
}

void MayaCutUI::doIt()
{
	// Check mayapy program is correct
	if (!checkMayapy())
	{
		return;
	}
	// Check the source file is exist and not empty
	QString source = p_mayaFileSource->text();
	QString dest = p_destination->text();
	if (source.isEmpty() || dest.isEmpty())
	{
		QMessageBox::warning(this, "Warning",
			"The maya file can't be empty", QMessageBox::Ok);
		return;
	}
	QFileInfo fileinfo(source);
	if (!fileinfo.exists())
	{
		QMessageBox::warning(this, "Warning",
			"The maya file was not found", QMessageBox::Ok);
		return;
	}
	fileinfo.setFile(dest);
	if (fileinfo.exists())
	{
		if (!fileinfo.isWritable())
		{
			QMessageBox::warning(this, "Warning",
				"The destination file can't be write", QMessageBox::Ok);
			return;
		}
	}
	QStringList arguments;
	arguments.append(QDir::currentPath() + PY_SCRIPT);
	arguments.append(source);
	arguments.append(dest);
	for (unsigned idx = 0; idx < (sizeof(FLAGS) / sizeof(char*)); idx++) 
	{
		if (options.at(idx)->isChecked())
		{
			arguments.append(FLAGS[idx]);
		}
	}
	p_proc->setProgram(mayapy_use);
	p_proc->setArguments(arguments);
	p_proc->start();
	p_proc->waitForFinished();
}

void MayaCutUI::closeEvent(QCloseEvent * event)
{
	QMessageBox::StandardButton user_push
		= QMessageBox::information(this, "Save Changes",
			"Do you want to save currect?",
			QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
	switch (user_push)
	{
	case QMessageBox::Yes:
		writeSettings();
		event->accept();
		break;
	case QMessageBox::No:
		event->accept();
		break;
	case QMessageBox::Cancel:
		event->ignore();
		break;
	}
}

void MayaCutUI::writeSettings()
{
	QSettings settings(INIT_SETTING, QSettings::IniFormat, this);
	settings.setValue(KEY_MAYAFILE, p_mayaFileSource->text());
	settings.setValue(KEY_DESTFILE, p_destination->text());
	settings.setValue(KEY_TEXTUREDIR, p_texturePath->text());
	// Store all checked into bitArray from each checkbox
	QBitArray isCheckedArray(sizeof(OPTION_NAME) / sizeof(char*));
	for (unsigned idx = 0; 
		idx < (sizeof(OPTION_NAME) / sizeof(char*)); 
		idx++) {
		isCheckedArray.setBit(idx, options.at(idx)->isChecked());
	}
	settings.setValue(KEY_OPTIONS, isCheckedArray);
	settings.setValue(KEY_MAYAVERIDX, p_mayaVersions->currentIndex());
	settings.setValue(KEY_MAYAPYS, *p_mayapy_progs);
}

void MayaCutUI::readSettings()
{
	QSettings settings(INIT_SETTING, QSettings::IniFormat, this);
	p_mayaFileSource->setText(
		settings.value(KEY_MAYAFILE, "").toString());
	p_destination->setText(
		settings.value(KEY_DESTFILE, "").toString());
	p_texturePath->setText(
		settings.value(KEY_TEXTUREDIR, "").toString());
	// mayapy version
	*p_mayapy_progs 
		= settings.value(KEY_MAYAPYS, MAYAPYS_DEF).toStringList();
	int mayaver_idx = settings.value(KEY_MAYAVERIDX, 0).toInt();
	mayapy_use = p_mayapy_progs->at(mayaver_idx);
	p_mayaVersions->setCurrentIndex(mayaver_idx);
	// read option's ini data
	QBitArray defaultBitArray(sizeof(OPTION_NAME) / sizeof(char*));
	defaultBitArray.fill(true);
	QBitArray isCheckedArray 
		= settings.value(KEY_OPTIONS, defaultBitArray).toBitArray();
	for (unsigned idx = 0; 
		idx < (sizeof(OPTION_NAME) / sizeof(char*)); 
		idx++) {
		options.at(idx)->setChecked(isCheckedArray.at(idx));
	}
}

bool MayaCutUI::checkMayapy()
{
	QString prog_name(mayapy_use);
	QFileInfo mayapy_fileinfo(prog_name);
	// If mayapy is valid in first, return true.
	if (mayapy_fileinfo.isExecutable())
	{
		return true;
	}
	// Otherwise, make user select mayapy.exe in loop.
	// We need select "mayapy.exe" correctly
	const QString version(p_mayaVersions->currentText());
	const int mayaVerIdx = p_mayaVersions->currentIndex();
	while (!mayapy_fileinfo.isExecutable()
		|| mayapy_fileinfo.baseName() != "mayapy")
	{
		prog_name = QFileDialog::getOpenFileName(
			this, "Find mayapy for " + version, "C:/", "exe (*.exe)");
		if (prog_name.isEmpty())
		{
			break;
		}
		mayapy_fileinfo.setFile(prog_name);
	}
	// If specify correctly, set mayapy path to current version.
	if (!prog_name.isEmpty())
	{
		(*p_mayapy_progs)[mayaVerIdx] = prog_name;
		mayapy_use = prog_name;
	}
	else
	{
		return false;
	}
	return true;
}

void MayaCutUI::checkMayaVersionFromSource()
{
	QString text = p_mayaFileSource->text();
	QFile * p_file = new QFile(text);
	if (!p_file->open(QIODevice::ReadOnly))
	{
		qDebug() << "Can't open this file";
		delete p_file;
		return;
	}
	// Create a regex to match maya version 20xx ( 4 digits )
	QRegularExpression re(R"(\d\d\d\d)");
	QRegularExpressionMatch rematch;
	// Maya file is ascii format
	if (text.endsWith(".ma"))
	{
		QString first_line = p_file->readLine();
		qDebug() << first_line;
		rematch = re.match(first_line);
	}
	// Maya file is binary format
	else if (text.endsWith(".mb"))
	{
		// Here is not implement
	}
	if (rematch.hasMatch())
	{
		qDebug() << "Now it has match!";
		qDebug() << rematch.captured();
		QString version = rematch.captured();
		if (version == "2014")
		{
			p_mayaVersions->setCurrentIndex(0);
		}
		else if (version == "2015")
		{
			p_mayaVersions->setCurrentIndex(1);
		}
		else if (version == "2016")
		{
			p_mayaVersions->setCurrentIndex(2);
		}
		else if (version == "2017")
		{
			p_mayaVersions->setCurrentIndex(3);
		}
		else
		{
			qDebug() << "Invalid maya version matched!";
		}
	}
	else
	{
		qDebug() << "Not match!";
	}
	p_file->close();
	delete p_file; p_file = nullptr;
}

void MayaCutUI::whenProgramStart()
{
	qDebug() << "Mayapy is running...";
}

void MayaCutUI::whenProgramFinished(
	int exitCode, QProcess::ExitStatus status)
{
	qDebug() << "Mayapy is finished!!";
	qDebug() << "Exit Code : " << exitCode;
	qDebug() << "Status : " << (status == QProcess::NormalExit ? "Normal" : "Crash");
	if (status == QProcess::NormalExit)
	{
		// Normal finished
		switch (exitCode)
		{
		case 0:
			QMessageBox::information(this, "Save Completed",
				QFileInfo(p_destination->text()).baseName() + " has been saved",
				QMessageBox::Ok);
			break;
		}
	}
	else
	{
		QMessageBox::warning(this, "Error", "Some error occurred, Process was failed.",
			QMessageBox::Ok);
	}
}
