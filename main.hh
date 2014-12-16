#ifndef PEERSTER_MAIN_HH
#define PEERSTER_MAIN_HH

#include <QDialog>
#include <QTextEdit>
#include <QLineEdit>
#include <QPushButton>
#include <QUdpSocket>
#include <QQueue>
#include <QHostInfo>
#include <QListWidget>
#include <QGridLayout>
#include <QLabel>
#include <QPaintEvent>

#include "NetSocket.hh"
#include "Peer.hh"
#include "FileMetadata.hh"
#include "TrackedFileMetadata.hh"

#include "ChordDHT.hh"
#include <QThread>    

class Sleeper : public QThread
{
public:
    static void usleep(unsigned long usecs){QThread::usleep(usecs);}
    static void msleep(unsigned long msecs){QThread::msleep(msecs);}
    static void sleep(unsigned long secs){QThread::sleep(secs);}
};

class Visualizer : public QWidget
{
	Q_OBJECT

public:
	Visualizer();
	void paintEvent(QPaintEvent *);
	void updateImage();

	int blockPos[100];

private:


public slots:


};


class InitDialog : public QDialog
{
	Q_OBJECT

public:
	InitDialog();

private:
	QGridLayout *layout;
	QLineEdit *peerEntry;
	QLabel * peerLabel;
	QLabel * progressLabel;
	QLabel * topLabel;
	QLineEdit *idEntry;
	QLabel* idLabel;
	QGridLayout* fullLayout;
	QPushButton *goButton;

public slots:
	void loadID();

};

class ChatDialog : public QDialog
{
	Q_OBJECT

public:
	ChatDialog(QStringList);

	void processArgs(QStringList);
	void send(QByteArray, Peer);
	void forwardAll(QVariantMap);
	void processMessage(QByteArray, QHostAddress, quint16);
	void readRumor(QVariantMap, QHostAddress, quint16);
	void checkStatus(QVariantMap, QHostAddress, quint16);
	void sendStatus(QHostAddress, quint16);

	void readBroadcast(QVariantMap msg);
	void readUploadNotice(QVariantMap msg);

	void pingRandomPeer();
	QByteArray serializeMsg(QVariantMap);
	void updateReadMsgs(QString, int, QVariant);
	QVariantMap getMyStatus();
	Peer getRandomPeer();
	void dumpPeerData();

	void addTimer();
	void killRequestTimer();

	void updateDSDV(QString, QHostAddress, quint16);

	// For BitTorrent dl
	void processBlockReply(QVariantMap msg);
	void sendBlockReply(QVariantMap msg);
	void sendBlockRequest(int blockNum, QString seeder, QByteArray fileID, QByteArray blockListHash);
	void sendBlockRequestToSeeders(QVariantMap msg);

	void sendRepReport() ;
	void readRepReport(QVariantMap msg);

public slots:
	void incomingMessage();
	void timedOutWaiting();
	void preventEntropy();
	void processPeer();
	void checkInfo(QHostInfo);
	void sendRoute();
	void sendBroadcast();

	void shareFile();
	void addFilesForSharing(QStringList);

	void requestSeeders(QListWidgetItem*);
	void replySeeders(QVariantMap msg);

private:
	QString myOrigin;
	NetSocket sock;
	QTextEdit *textview;
	QTextEdit *userInput;
	QPushButton *shareButton;
	QPushButton *dlButton;
	QLineEdit *fileSearch;
	QListWidget *dlList;
	QLineEdit *peerEntry;
	QListWidget *uploadList;
	quint32 seqNo;
	quint32 failedDLNum;
	bool downloading;

	// Maps of origin to wanted message
	QMap<QString, quint32> *wantedSeqNos;
	// Map of int to map of origin to message
	QMap<quint32, QVariant> *readMsgs;
	// List of peers communicated with
	QList<Peer> peers;

	QHash<QString, QPair<QHostAddress, quint16> > dsdv;

	// Files this node has made available for download
	QList<FileMetadata> filesForDL;
	// Files this node is waiting to download
	QList<FileMetadata> waitingToDL;

	// Files this node is tracking
	QList<TrackedFileMetadata> filesTracking;
	QVariantMap availableFiles;
	
	QHash<QString, QPair<QString, QByteArray> > wantToDL;
	QList<QString> foundForDL;

	QVariantMap lastSearch;
	QTimer *searchTimer;
	QList<QPair<QString, QByteArray> > searchResponses;

	FileMetadata currentDL;
	QVariantMap lastRequest;
	QTimer *requestTimer;

	QQueue<QTimer *> *timerQueue;
	QTimer *entropyTimer;
	QTimer *routeTimer;
	QTimer *broadcastTimer;

	QList<QStringList> maybePeers;

	// Chord table, initialized after name is decided
	ChordDHT *chord;

	QList<int> blocksLeft; //blocks left to download from file
	QMap<int, QByteArray> blocksAcquired; //file blocks acquired and saved
	Visualizer *visual;

	QString currentTracker;
	QVariantMap repReport;
	QVariantMap repTracking;
	bool excludeSeeders;
	QStringList topSeeders;
	int currentBlockCount;
};

#endif // PEERSTER_MAIN_HH
