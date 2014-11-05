#include "PrivateMessageWindow.hh"

#include <QHBoxLayout>

////////////////////////////////////////////////////////////////
// PrivateMessageWindow
//   Window for entering private messages
////////////////////////////////////////////////////////////////

PrivateMessageWindow::PrivateMessageWindow(QDialog *myDialog, QString bud)
{
	buddy = bud;
	this->setWindowTitle("Private message to: " + buddy);

	QHBoxLayout *messageLayout = new QHBoxLayout(this);
	messageBox = new SageTextEdit();
	messageLayout->addWidget(messageBox);
	setLayout(messageLayout);
	setWindowFlags(Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::Dialog);

	connect(messageBox, SIGNAL(sageReturnPressed()),
			this, SLOT(textEntered()));
	connect(this, SIGNAL(messageEntered(QString, QString)),
			myDialog, SLOT(sendPrivateMessage(QString, QString)));
	messageBox->setFocus();
	this->show();
}

void PrivateMessageWindow::textEntered()
{
	emit messageEntered(messageBox->toPlainText(), buddy);
	this->close();
}
