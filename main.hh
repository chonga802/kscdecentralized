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

#include "NetSocket.hh"
#include "Peer.hh"
#include "SageTextEdit.hh"
#include "FileMetadata.hh"

class ChatDialog : public QDialog
{
	Q_OBJECT

public:
	ChatDialog(QStringList);

	void processArgs(QStringList);
	void send(QByteArray, Peer);
	void forwardAll(QVariantMap);
	void processMessage(QByteArray, QHostAddress, quint16);
	void processBlockRequest(QVariantMap);
	void processBlockReply(QVariantMap);
	void readRumor(QVariantMap, QHostAddress, quint16);
	void printRumor(QVariantMap);
	void checkStatus(QVariantMap, QHostAddress, quint16);
	void sendStatus(QHostAddress, quint16);
	void processSearchRequest(QVariantMap);
	void processSearchReply(QVariantMap);
	void startNextDownload();

	void pingRandomPeer();
	QByteArray serializeMsg(QVariantMap);
	void updateReadMsgs(QString, int, QVariant);
	QVariantMap getMyStatus();
	Peer getRandomPeer();
	void dumpPeerData();
	void dumpReadMsgs();

	void addTimer();
	void killRequestTimer();

	void updateDSDV(QString, QHostAddress, quint16);

	// For non-seq downloads
	void startNonSeqDL();
	void processNonSeqBlockResponse(QVariantMap response);
	void finishNonSeqDL();

public slots:
	void gotReturnPressed();
	void incomingMessage();
	void timedOutWaiting();
	void preventEntropy();
	void processPeer();
	void checkInfo(QHostInfo);
	void sendRoute();

	void shareFile();
	void addFilesForSharing(QStringList);
	void resendRequest();

	void privateMessage(QListWidgetItem*);
	void sendPrivateMessage(QString msg, QString buddy);

	void createSearchRequest();
	void resendSearch();

	void startDownload();
	void startDownload(QString, QString);
	void initiateDownload(QListWidgetItem*);

	// For non-seq downloads
//	void requestNonSeqBlocks();

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
	QListWidget *privateList;
	quint32 seqNo;
	quint32 failedDLNum;
	bool noForward;
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

	QHash<QString, QPair<QString, QByteArray> > wantToDL;
	QList<QString> foundForDL;

	// for non-seq dl
	QList<quint32> dlBlocksWanted;
	QMap<quint32, QByteArray> dlBlocks;
	bool nonSeqDL;
	QStringList seeders;

	QVariantMap lastSearch;
	QTimer *searchTimer;
	QList<QPair<QString, QByteArray> > searchResponses;

	FileMetadata currentDL;
	QVariantMap lastRequest;
	QTimer *requestTimer;

	QQueue<QTimer *> *timerQueue;
	QTimer *entropyTimer;
	QTimer *routeTimer;

	QList<QStringList> maybePeers;
};

#endif // PEERSTER_MAIN_HH