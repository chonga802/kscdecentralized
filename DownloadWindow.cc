#include "DownloadWindow.hh"

#include <QVBoxLayout>
#include <QLabel>

////////////////////////////////////////////////////////////////
// DownloadWindow
//   Window for entering files to download
////////////////////////////////////////////////////////////////

DownloadWindow::DownloadWindow(QDialog *myDialog)
{
	this->setWindowTitle("Please specify file to download");

	QVBoxLayout *messageLayout = new QVBoxLayout(this);
	targetEntry = new QLineEdit(this);
	QLabel* targetLabel = new QLabel("Enter target peer:", targetEntry);
	fileEntry = new QLineEdit(this);
	QLabel* fileLabel = new QLabel("Enter file to download:", fileEntry);
	doneButton = new QPushButton("DOWNLOAD", this);

	messageLayout->addWidget(targetLabel);
	messageLayout->addWidget(targetEntry);
	messageLayout->addWidget(fileLabel);
	messageLayout->addWidget(fileEntry);
	messageLayout->addWidget(doneButton);
	setLayout(messageLayout);
	setWindowFlags(Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::Dialog);

	connect(doneButton, SIGNAL(clicked()),
			this, SLOT(fileEntered()));
	connect(this, SIGNAL(downloadEntered(QString, QString)),
			myDialog, SLOT(startDownload(QString, QString)));
	targetEntry->setFocus();
	this->show();
}

void DownloadWindow::fileEntered()
{
	emit downloadEntered(targetEntry->text(), fileEntry->text());
	this->close();
}
