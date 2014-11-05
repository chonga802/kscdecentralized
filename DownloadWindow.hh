#ifndef DOWNLOAD_WINDOW_HH
#define DOWNLOAD_WINDOW_HH

#include <QDialog>
#include <QString>
#include <QLineEdit>
#include <QPushButton>

// Window to handle private messaging
class DownloadWindow : public QDialog
{
	Q_OBJECT

public:
	DownloadWindow(QDialog *);

signals:
	void downloadEntered(QString, QString);

public slots:
	void fileEntered();	// SageTextEnter signal will connect to this slot
						//	which will emit the downloadEntered signal
						//	with the info that was entered

private:
	QLineEdit* targetEntry;
	QLineEdit* fileEntry;
	QPushButton* doneButton;
};

#endif // DOWNLOAD_WINDOW_HH
