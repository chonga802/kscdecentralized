#ifndef PRIVATE_MESSAGE_WINDOW_HH
#define PRIVATE_MESSAGE_WINDOW_HH

#include <QDialog>
#include <QString>

#include "SageTextEdit.hh"

// Window to handle private messaging
class PrivateMessageWindow : public QDialog
{
	Q_OBJECT

public:
	PrivateMessageWindow(QDialog *, QString);

signals:
	void messageEntered(QString, QString);

public slots:
	void textEntered();		// SageTextEnter signal will connect to this slot
						//	which will emit the messageEntered signal
						//	with the text that was entered

private:
	QString buddy;
	SageTextEdit* messageBox;
};

#endif // PRIVATE_MESSAGE_WINDOW_HH
