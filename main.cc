#include <unistd.h>
#include <stdlib.h>

#include <QGridLayout>
#include <QApplication>
#include <QDebug>
#include <QVariant>
#include <QVariantMap>
#include <QMap>
#include <QDataStream>
#include <QTime>
#include <QTimer>
#include <QQueue>
#include <QLabel>
#include <QListWidgetItem>
#include <QFileDialog>
#include <QtCrypto>
#include <QCryptographicHash>
 #include <QPainter>

#include "main.hh"
#include "NetSocket.hh"
#include "Peer.hh"
#include "PrivateMessageWindow.hh"
#include "SageTextEdit.hh"
#include "FileMetadata.hh"
#include "TrackedFileMetadata.hh"

#include "DownloadWindow.hh"

QString CHAT_TEXT = "ChatText";
QString ORIGIN = "Origin";
QString SEQNO = "SeqNo";
QString WANT = "Want";
QString DEST = "Dest";
QString HOP_LIMIT = "HopLimit";
QString NO_FORWARD = "-noforward";
QString LAST_IP = "LastIP";
QString LAST_PORT = "LastPort";
QString BLOCK_REQUEST = "BlockRequest";
QString BLOCK_REPLY = "BlockReply";
QString DATA = "Data";
QString SEARCH = "Search";
QString BUDGET = "Budget";
QString SEARCH_REPLY = "SearchReply";
QString MATCH_NAMES = "MatchNames";
QString MATCH_IDS = "MatchIDs";

////////////////////////////////////////////////////////////////
// ChatDialog
////////////////////////////////////////////////////////////////

//ChatDialog::InitDialog(QStringList args) {

//}

ChatDialog::ChatDialog(QStringList args)
{
	// configure no forward option
	noForward = false;
	foreach (QString arg, args) {
		if (arg == NO_FORWARD) {
			noForward = true;
			args.removeAll(arg);
			qDebug() << "Not forwarding";
		}
	}

	// Create a UDP network socket
	if (!sock.bind())
		exit(1);

	qsrand(QTime::currentTime().msec()+1);
	// Hack to construct unique origin identifier
	myOrigin = "SAGE_";
	myOrigin.append(QString::number((1+QTime::currentTime().msec())
			* qrand()%1000000));

	setWindowTitle(myOrigin + " @Port:" + QString::number(sock.getMyPort()));
	// Read-only text box where we display messages from everyone.
	// This widget expands both horizontally and vertically.
//	textview = new QTextEdit(this);
//	textview->setReadOnly(true);

	// Exercise 2: SageTextEdit subclasses QTextEdit for multi-line text entry.
	userInput = new SageTextEdit();

	fileSearch = new QLineEdit();
	QLabel* fileSearchLabel = new QLabel("Enter file to search for:", fileSearch);
	shareButton = new QPushButton("Share file", this);
	shareButton->setAutoDefault(false);
	dlButton = new QPushButton("Download file", this);
	dlButton->setAutoDefault(false);
	dlList = new QListWidget(this);
	QLabel* dlLabel = new QLabel("Click file to download:", dlList);

	peerEntry = new QLineEdit();
	QLabel* peerLabel = new QLabel("Add peer:", peerEntry);

	privateList = new QListWidget(this);
	QLabel* privateListLabel = new QLabel("Click ID to direct message:", privateList);

	// Lay out the widgets to appear in the main window.
	QGridLayout *layout = new QGridLayout();

	QHBoxLayout *peerEntryLayout = new QHBoxLayout();
	peerEntryLayout->addWidget(peerLabel);
	peerEntryLayout->addWidget(peerEntry);

	QVBoxLayout *privMsgLayout = new QVBoxLayout();
	privMsgLayout->addWidget(privateListLabel);
	privMsgLayout->addWidget(privateList);

	QVBoxLayout *textAndPeers = new QVBoxLayout();
	textAndPeers->addLayout(privMsgLayout);
	textAndPeers->addLayout(peerEntryLayout);

	QHBoxLayout *files = new QHBoxLayout();
	files->addWidget(dlButton);
	files->addWidget(shareButton);


//    QPixmap pixmap(100,100);
  //  pixmap.fill(QColor("transparent"));

//    QPainter painter(&pixmap);
//    painter.setBrush(QBrush(Qt::black));
//    painter.drawRect(20, 10, 100, 100);

//    QLabel* al = new QLabel;
 //   al->setPixmap(pixmap);




	QVBoxLayout *fileEntry = new QVBoxLayout();
	fileEntry->addWidget(dlLabel);
	fileEntry->addWidget(dlList);
	fileEntry->addWidget(fileSearchLabel);
	fileEntry->addWidget(fileSearch);
	fileEntry->addLayout(files);

	// resize
	layout->addLayout(textAndPeers, 1, 0);
	layout->addLayout(fileEntry, 1, 1);
//	layout->addWidget(al, 1, 2);
	layout->setColumnStretch(0, 1);
	layout->setColumnStretch(1, 2);
	layout->setColumnStretch(2, 2);
	setLayout(layout);

	seqNo = 1;
	downloading = false;
	nonSeqDL = false;

	wantedSeqNos = new QMap<QString, quint32>();
	wantedSeqNos->insert(myOrigin, seqNo);
	readMsgs = new QMap<quint32, QVariant>();

	for (int i = sock.getPortMin(); i <= sock.getPortMax(); i++)
		if (sock.getMyPort() != i)
			peers.append(Peer(QHostAddress(QHostAddress::LocalHost), i));

	processArgs(args);

	// Register a callback on the userInput's returnPressed signal
	// so that we can send the message entered by the user.
	connect(userInput, SIGNAL(sageReturnPressed()),
			this, SLOT(gotReturnPressed()));
	// Register incoming message so we can process accordingly
	connect(&sock, SIGNAL(readyRead()),
			this, SLOT(incomingMessage()));
	// Register newly IP/Port or DNS origin/port to message
	connect(peerEntry, SIGNAL(returnPressed()),
			this, SLOT(processPeer()));
	// Register peer clicked for private messaging so we will
	// open a private message window to communicate
	connect(privateList, SIGNAL(itemClicked(QListWidgetItem *)),
			this, SLOT(privateMessage(QListWidgetItem *)));
	// Register button click to open file selection dialog box
	connect(shareButton, SIGNAL(clicked()),
			this, SLOT(shareFile()));
	// Register button click to open file download box
	connect(dlButton, SIGNAL(clicked()),
			this, SLOT(startDownload()));
	// Register enter to start file search
	connect(fileSearch, SIGNAL(returnPressed()),
			this, SLOT(createSearchRequest()));
	connect(dlList, SIGNAL(itemClicked(QListWidgetItem *)),
			this, SLOT(requestSeeders(QListWidgetItem *)));

	timerQueue = new QQueue<QTimer *>();

	entropyTimer = new QTimer(this);
	connect(entropyTimer, SIGNAL(timeout()), this, SLOT(preventEntropy()));
	entropyTimer->start(10000);

	routeTimer = new QTimer(this);
	connect(routeTimer, SIGNAL(timeout()), this, SLOT(sendRoute()));
	routeTimer->start(60000);

	// Timer to broadcast list of tracked files
	broadcastTimer = new QTimer(this);
	connect(broadcastTimer, SIGNAL(timeout()), this, SLOT(sendBroadcast()));
	broadcastTimer->start(10000);


	sendRoute();

	// Exercise 1: We wish to have focus initially on the text entry box.
	userInput->setFocus();
}


//////////////////////////////////////////////////////////////
//	File Sharing
//////////////////////////////////////////////////////////////

void ChatDialog::shareFile()
{
	QFileDialog dialog(this);
	dialog.setFileMode(QFileDialog::ExistingFiles);
	dialog.setViewMode(QFileDialog::Detail);
	connect(&dialog, SIGNAL(filesSelected(QStringList)),
			this, SLOT(addFilesForSharing(QStringList)));
	dialog.exec();
}

void ChatDialog::addFilesForSharing(QStringList files)
{
	bool found;
	foreach(QString file, files) {
		found = 0;
		foreach (FileMetadata data, filesForDL) {
			if (data.getFileName() == file)
				found = 1;
		}
		if (!found) {
			QFileInfo info(file);
			QFile FILE(file);
			FILE.open(QFile::ReadOnly);

			QByteArray blocklistHash;
			QCA::Hash shaHash("sha1");

			int blockCount = 0;
			while(!FILE.atEnd()){
				shaHash.update(FILE.read(8192));		// 8192 = 8KB
				QByteArray byteBlock = shaHash.final().toByteArray();
				blocklistHash.append(byteBlock);
				shaHash.clear();						// clear buffer
				blockCount++;
			}
			FILE.close();

			shaHash.update(blocklistHash);
			QByteArray metaHash = shaHash.final().toByteArray();
			shaHash.clear();

			FileMetadata newFile(file, info.fileName(),
					info.size(), blocklistHash, metaHash);
			filesForDL.append(newFile);

			// Add file to tracked files (with self as first seeder)
			TrackedFileMetadata tracked(info.fileName(), blockCount, blocklistHash,
				metaHash, myOrigin);
			filesTracking.append(tracked);

			// Announce you now have file
			/////////// ADD : APPEND FILE TO UPLOADS DISPLAY
			sendBroadcast();

		}
	}
}

///////////////////////////////////////////////////////////////////
//  Tracking
///////////////////////////////////////////////////////////////////

void ChatDialog::sendBroadcast()
{
	QVariantMap msg;
	QVariantMap fileData;

	foreach(TrackedFileMetadata tracked, filesTracking) {
		fileData.insert(tracked.fileName, tracked.seederCount);
		qDebug() << tracked.fileName + QString::number(tracked.blockCount);
    }

	msg.insert("Broadcast", fileData);
	msg.insert(ORIGIN, myOrigin);

	if (!peers.isEmpty())
		forwardAll(msg);

}

void ChatDialog::readBroadcast(QVariantMap msg)
{
	QString origin = msg[ORIGIN].toString();
	QVariantMap fileData = msg["Broadcast"].toMap();

	QMapIterator<QString, QVariant> mapIter(fileData);

	while (mapIter.hasNext()) {
		mapIter.next();

		if (!availableFiles.contains(mapIter.key())) {
			availableFiles[mapIter.key()] = origin;
			QString display = mapIter.key() + "                     " + QString::number(mapIter.value().toInt());
			dlList->addItem(new QListWidgetItem(display));
		}
	}
}

void ChatDialog::requestSeeders(QListWidgetItem *clicked)
{
	QStringList l = (clicked->text()).split(" ");
	QString file = l[0];

	int index = dlList->row(clicked);
	dlList->takeItem(index);

	if (!availableFiles.contains(file)) {
		qDebug() << "AVF error";
		return;
	}

	QString target = availableFiles[file];

	QVariantMap request;
	request.insert(DEST, target);
	request.insert(ORIGIN, myOrigin);
	request.insert("SeedRequest", file);

	QPair<QHostAddress, quint16> dest = dsdv.value(target);
	send(serializeMsg(request), Peer(dest.first, dest.second));

//	requestTimer = new QTimer(this);
//	connect(requestTimer, SIGNAL(timeout()), this, SLOT(resendRequest()));
//	requestTimer->start(5000);
}

void ChatDialog::replySeeders(QVariantMap msg)
{
	QVariantMap request;
	request.insert(DEST, msg[ORIGIN]);
	request.insert(ORIGIN, myOrigin);
	request.insert("SeedReply", msg["SeedRequest"]);
	TrackedFileMetadata *found;

	foreach(TrackedFileMetadata tracked, filesTracking) {
		if (tracked.fileName == msg["SeedRequest"].toString()) {
			request.insert("MetaFileID", tracked.metaHash);
			request.insert("BlockListHash", tracked.blocklistHash);
			request.insert("Seeders", tracked.seeders);
			request.insert("BlockCount", tracked.blockCount);
			found = &tracked;
		}
	}

	QPair<QHostAddress, quint16> dest = dsdv.value(msg[ORIGIN].toString());
	send(serializeMsg(request), Peer(dest.first, dest.second));

	found->seederCount++;
	found->seeders.append(msg[ORIGIN].toString());
}



///////////////////////////////////////////////////////////////////
// private messages and routing
///////////////////////////////////////////////////////////////////

void ChatDialog::sendPrivateMessage(QString text, QString target)
{
	QVariantMap msg;
	msg.insert(DEST, target);
	msg.insert(ORIGIN, myOrigin);
	msg.insert(CHAT_TEXT, text);
	msg.insert(HOP_LIMIT, 10);

	QPair<QHostAddress, quint16> peerData = dsdv.value(target);
	send(serializeMsg(msg), Peer(peerData.first, peerData.second));

	textview->append("<TO:" + target + "> " + text);

	userInput->clear();
}

void ChatDialog::privateMessage(QListWidgetItem *buddy)
{
	PrivateMessageWindow *privateMessageWindow =
			new PrivateMessageWindow(this, buddy->text());
	privateMessageWindow->setParent(this, Qt::Dialog);

	privateMessageWindow->show();
}

void ChatDialog::sendRoute()
{
	QVariantMap routeRumor;
	routeRumor.insert(ORIGIN, myOrigin);
	routeRumor.insert(SEQNO, seqNo++);

	wantedSeqNos->insert(myOrigin, seqNo);
	updateReadMsgs(myOrigin, seqNo-1, routeRumor);

	if (!peers.isEmpty())
		forwardAll(routeRumor);
}

////////////////////////////////////////////////////////////////
//PEER PROCESSING SECTION
//	for processing and adding new peers to communicate with
////////////////////////////////////////////////////////////////

void ChatDialog::processPeer()
{
	processArgs(QStringList(peerEntry->text()));
	peerEntry->clear();
}

void ChatDialog::processArgs(QStringList args)
{
	QHostAddress notIP = QHostAddress("");
	for (int i = 0; i < args.size(); i++) {
		// separate into ip/dns and port
		int colonCount = args.at(i).count(':');
		QString ip = args.at(i).section(':', 0, colonCount-1);
		QString port = args.at(i).section(':', colonCount, colonCount);
		// if no colon, invalid, drop it
		if (port == ip)
			return;

		QStringList peerPair;
		peerPair.append(ip);
		peerPair.append(port);

		maybePeers.append(peerPair);

		if (QHostAddress(ip) != notIP)
			peers.append(Peer(QHostAddress(ip), port.toInt()));
		else
			QHostInfo::lookupHost(ip, this, SLOT(checkInfo(QHostInfo)));
	}

}

void ChatDialog::checkInfo(QHostInfo host)
{
	if (host.error() != QHostInfo::NoError) {
		qDebug() << "DNS lookup failed:" << host.errorString();
		return;
	}
	foreach (const QHostAddress &address, host.addresses()) {
		//qDebug() << "Found address:" << address.toString() << host.hostName();
		for (int i = 0; i < maybePeers.size(); i++) {
			Peer newPeer = Peer(address, maybePeers.at(i).at(1).toInt());
			if (!peers.contains(newPeer) && 
					(maybePeers.at(i).at(0) == host.hostName()) &&
					(maybePeers.at(i).at(1).toInt() != 0))
				peers.append(Peer(address, maybePeers.at(i).at(1).toInt()));
		}
	}
}

////////////////////////////////////////////////////
//	SENDING NEW MESSAGES AND FORWARDING ALL
////////////////////////////////////////////////////
void ChatDialog::gotReturnPressed()
{
	// create message
	QVariantMap msg;
	msg.insert(CHAT_TEXT, userInput->toPlainText());
	msg.insert(ORIGIN, myOrigin);
	msg.insert(SEQNO, seqNo++);
	wantedSeqNos->insert(myOrigin, seqNo);
	updateReadMsgs(myOrigin, seqNo-1, QVariant(msg));

	send(serializeMsg(msg), getRandomPeer());

	textview->append(userInput->toPlainText());

	userInput->clear();
	addTimer();
}

void ChatDialog::send(QByteArray bytes, Peer peer)
{
	sock.writeDatagram(bytes.data(), bytes.size(),
			peer.getAddress(), peer.getPort());
}

void ChatDialog::forwardAll(QVariantMap msg)
{
	foreach(Peer peer, peers) {
		send(serializeMsg(msg), peer);
	}
}

////////////////////////////////////////////////////////////
//	PROCESS INCOMING MESSAGES
////////////////////////////////////////////////////////////
void ChatDialog::incomingMessage()
{
	while (sock.hasPendingDatagrams()) {
		if (!timerQueue->isEmpty())
			timerQueue->dequeue()->stop();

		QByteArray byteMessage;
		byteMessage.resize(sock.pendingDatagramSize());

        QHostAddress sender;
        quint16 senderPort;
		sock.readDatagram(byteMessage.data(), byteMessage.size(),
				&sender, &senderPort);

		processMessage(byteMessage, sender, senderPort);
	}
}

void ChatDialog::processMessage(QByteArray bytes, QHostAddress sender, quint16 senderPort)
{
	QVariantMap msg;
	QDataStream inStream(&bytes, QIODevice::ReadWrite);
	inStream >> msg;

	if (!peers.contains(Peer(sender, senderPort)))
		peers.append(Peer(sender, senderPort));

	// Status request
	if (msg.contains(WANT))
		checkStatus(msg.value(WANT).toMap(), sender, senderPort);
	// Broadcast
	else if (msg.contains("Broadcast"))
		readBroadcast(msg);
	// Invalid message
	else if (msg.size() == 0) {
		qDebug() << "EMPTY MAP FOUND FROM" << sender << senderPort;
	}
	// Directed messages
	else if (msg.contains(DEST)) {
		QString dest = msg.value(DEST).toString();

		if (dest == myOrigin) {
			if (msg.contains(CHAT_TEXT))
				printRumor(msg);
			else if (msg.contains(BLOCK_REQUEST))
				processBlockRequest(msg);
			else if (msg.contains(BLOCK_REPLY))
				processBlockReply(msg);
		//	else if (msg.contains(SEARCH_REPLY))
		//		processSearchReply(msg);
			else if (msg.contains("SeedRequest"))
				replySeeders(msg);
			else if (msg.contains("SeedReply"))
				beginTorrent(msg);
		}
		// must forward
		else if (msg.value(HOP_LIMIT).toInt() > 0) {
			if (noForward)
				return;
			msg.insert(HOP_LIMIT, msg.value(HOP_LIMIT).toInt()-1);
			QPair<QHostAddress, quint16> destData = dsdv.value(dest);

			send(serializeMsg(msg), Peer(destData.first, destData.second));
		}
	}
	else if (msg.contains(SEARCH)) {
		processSearchRequest(msg);
	}
	// Rumors
	else
		readRumor(msg, sender, senderPort);
}

//////////////////////////////////////////////////////////////
//	FILE SEARCH
//////////////////////////////////////////////////////////////

void ChatDialog::createSearchRequest()
{
	QString keywords = fileSearch->text();
	fileSearch->clear();

	QVariantMap search;
	search.insert(ORIGIN, myOrigin);
	search.insert(SEARCH, keywords);
	search.insert(BUDGET, quint32(2));

	lastSearch = search;
	qDebug() << lastSearch;
	foreach (Peer peer, peers)
		send(serializeMsg(search), peer);

	searchTimer = new QTimer(this);
	connect(searchTimer, SIGNAL(timeout()), this, SLOT(resendSearch()));
	searchTimer->start(1000);
}

void ChatDialog::resendSearch()
{
	if ((lastSearch.value(BUDGET).toUInt() > 99) || (searchResponses.size() > 9))
		searchTimer->stop();
	else {
		quint32 newBudget = lastSearch.value(BUDGET).toUInt();
		newBudget *= 2;
		lastSearch.insert(BUDGET, newBudget);
		foreach (Peer peer, peers)
			send(serializeMsg(lastSearch), peer);
	}
}

void ChatDialog::processSearchRequest(QVariantMap request)
{
	// drop own search requests
	if (request.value(ORIGIN).toString() == myOrigin)
		;
	else {
		// generate response if necessary
		QStringList keywords;
		keywords = request.value(SEARCH).toString().split(" ");

		QVariantList matchNames;
		QVariantList matchIDs;

		bool fileMatched;
		foreach(FileMetadata file, filesForDL) {
			fileMatched = 0;
			foreach(QString keyword, keywords) {
				if (file.getFileName().contains(keyword))
					fileMatched = 1;
			}
			if (fileMatched) {
				matchNames.append(file.getFileName());
				matchIDs.append(file.getMetaHash());
			}
		}
		// send response
		if (!matchNames.isEmpty()) {
			QVariantMap reply;
			reply.insert(DEST, request.value(ORIGIN).toString());
			reply.insert(ORIGIN, myOrigin);
			reply.insert(HOP_LIMIT, 20);
			reply.insert(SEARCH_REPLY, request.value(SEARCH).toString());
			reply.insert(MATCH_NAMES, matchNames);
			reply.insert(MATCH_IDS, matchIDs);
			QPair<QHostAddress, quint16> dest =
					dsdv.value(request.value(ORIGIN).toString());
			send(serializeMsg(reply), Peer(dest.first, dest.second));	
		}
		// forward search request
		quint32 budg = request.value(BUDGET).toUInt() - 1;
		if (budg > 0) {
			quint32 newBudg = budg / peers.size();
			quint32 extra = budg % peers.size();

			foreach (Peer neighbor, peers) {
				quint32 finalBudg = newBudg;
				if (extra > 0) {
					finalBudg++;
					extra--;
				}
				if (finalBudg > 0) {
					request.insert(BUDGET, finalBudg);
					send(serializeMsg(request), neighbor);
				}
			}
		}
	}
}
/*
void ChatDialog::processSearchReply(QVariantMap request)
{
	QVariantList matchNames = request.value(MATCH_NAMES).toList();
	QVariantList matchIds = request.value(MATCH_IDS).toList();
	QString orig = request.value(ORIGIN).toString();

	qDebug() << "MatchNames" << matchNames;
	qDebug() << "MatchIds" << matchIds;

	while (!matchNames.isEmpty()) {
		QString name = matchNames.takeFirst().toString();
		QByteArray id = matchIds.takeFirst().toByteArray();
		if (!wantToDL.contains(name)) {
			wantToDL.insert(name, QPair<QString, QByteArray>(orig, id));
			foundForDL.append(name);
			dlList->addItem(new QListWidgetItem(name));
		}
	}
}*/

//////////////////////////////////////////////////////////////
//	FILE DOWNLOADING:
//		Process requests/replies
//		Timer behavior
//		Start download
//////////////////////////////////////////////////////////////

// Take seeder list and start non-sequential download
void ChatDialog::beginTorrent(QVariantMap msg) {


}

////// NON-SEQUENTIAL, MUTLI-PEER DOWNLOAD GOES HERE /////////
/*
// initiate non-sequential download
void ChatDialog::startNonSeqDL() {
	// Get bytes needed
	// fill blocks wanted with nums of blocks
	// fix nonSeqDL bool
	// call requestBlocks
	// start timer, link to requestBlocks
}

// slot called on timer to request remaining blocks
void ChatDialog::requestNonSeqBlocks() {
	if (dlBlocksWanted.isEmpty()) {
		finishNonSeqDL();
	}
	else {
		foreach (quint32 blockNum, dlBlocksWanted) {
			QString randSeeder = getRandomSeeder(seeders);
			QVariantMap request;
			request.insert(DEST, randSeeder);
			request.insert(ORIGIN, myOrigin);
			request.insert(HOP_LIMIT, 20);
			// need to add method to get bytes for num
			request.insert(BLOCK_REQUEST, currentDL.getHashBytes(blockNum)); 
		}
	}
}

// called only when doing non-seq dl, called by normal dl method
void ChatDialog::processNonSeqBlockResponse(QVariantMap response) {
	QByteArray data = response.value(DATA).toByteArray();
	QByteArray hashedData = QCA::Hash("sha1").hash(data).toByteArray();
	if (hashedData != reply.value(BLOCK_REPLY).toByteArray()) {
		qDebug() << "ERROR: data returned does not match hash";
		qDebug() << hashedData.toHex();
		qDebug() << reply.value(BLOCK_REPLY).toByteArray().toHex();
		failedDLNum++;
	}
	quint32 blockNum = currentDL.findBytesIndex(hashedData);
	dlBlocks.insert(blockNum, data);
	dlBlocksWanted.remove(blockNum);
}

void ChatDialog::finishNonSeqDL() {
	QByteArray allBytes;
	QByteArray blockBytes;
	quint32 blockNum;

	while (dlBlocks.contains(blockNum)) {
		blockBytes = dlBlocks.value(blockNum);
		allBytes.append(blockBytes);
		blockNum++;
	}

	qDebug() << "DONE WITH NON-SEQUENTIAL DOWNLOAD";
	QFile tempFile(currentDL.getFileName());
	tempFile.open(QIODevice::WriteOnly);
	tempFile.write(allBytes);
	tempFile.close();

	nonSeqDL = false;
	downloading = false;
}
*/
//////////////////////// NORMAL DOWNLOADS ///////////////////////


void ChatDialog::startDownload(QString target, QString hex)
{
	qDebug() << "target of dl:" << target << hex;
	qDebug() << "hex to dl: " << hex.toLatin1();
	if (!downloading) {
		QVariantMap request;
		request.insert(DEST, target);
		request.insert(ORIGIN, myOrigin);
		request.insert(HOP_LIMIT, 20);
		request.insert(BLOCK_REQUEST, QByteArray::fromHex(hex.toLatin1()));

		currentDL = FileMetadata("IDK_NAME" + qrand(),
				QByteArray::fromHex(hex.toLatin1()));
		lastRequest = request;

		QPair<QHostAddress, quint16> dest = dsdv.value(target);
		send(serializeMsg(request), Peer(dest.first, dest.second));
		downloading = true;
		failedDLNum = 0;

		requestTimer = new QTimer(this);
		connect(requestTimer, SIGNAL(timeout()), this, SLOT(resendRequest()));
		requestTimer->start(5000);
	}
	else {
		FileMetadata fileToDL = 
				FileMetadata(target, QByteArray::fromHex(hex.toLatin1()));
		waitingToDL.append(fileToDL);
	}
}

void ChatDialog::startDownload()
{
	DownloadWindow *downloadWindow = new DownloadWindow(this);
	downloadWindow->setParent(this, Qt::Dialog);

	downloadWindow->show();
}

void ChatDialog::processBlockRequest(QVariantMap request)
{
	QByteArray requestedBytes = request.value(BLOCK_REQUEST).toByteArray();
	QByteArray replyHashBytes, dataBytes;
	bool foundData = 0;
	qDebug() << "REQUESTED:" << requestedBytes.toHex();
	foreach (FileMetadata data, filesForDL) {
		// if metafile request, send block list hash
		if (data.getMetaHash() == requestedBytes) {
			qDebug() << "DATA MATCHES A METAHASH";
			replyHashBytes = data.getMetaHash();
			dataBytes = data.getBlocklistHash();
			foundData = 1;
			break;
		}
		// else if hash is found, get associated data
		else if (data.findBytesIndex(requestedBytes) != -1) {
			qDebug() << "DATA FOUND IN BLOCKLIST, INDEX:" <<
					data.findBytesIndex(requestedBytes);
			replyHashBytes = requestedBytes;

			QFile FILE(data.getFullName());
			FILE.open(QFile::ReadOnly);
			int blockNum = data.findBytesIndex(requestedBytes);
			while (blockNum > 0) {
				FILE.read(8192);
				blockNum--;
			}
			dataBytes = FILE.read(8192);
			FILE.close();
			foundData = 1;
			break;
		}
	}

	if (foundData) {
		qDebug() << "REPLY HASH:" << replyHashBytes.toHex();
		QVariantMap reply;
		reply.insert(DEST, request.value(ORIGIN).toString());
		reply.insert(ORIGIN, myOrigin);
		reply.insert(HOP_LIMIT, 20);
		reply.insert(BLOCK_REPLY, replyHashBytes);
		reply.insert(DATA, dataBytes);

		QPair<QHostAddress, quint16> dest =
				dsdv.value(request.value(ORIGIN).toString());
		send(serializeMsg(reply), Peer(dest.first, dest.second));
	}
	else
		qDebug() << "COULD NOT FIND REQUESTED DATA";
}

void ChatDialog::processBlockReply(QVariantMap reply)
{
	QByteArray data = reply.value(DATA).toByteArray();
	QByteArray hashedData = QCA::Hash("sha1").hash(data).toByteArray();
	if (hashedData != reply.value(BLOCK_REPLY).toByteArray()) {
		qDebug() << "ERROR: data returned does not match hash";
		qDebug() << hashedData.toHex();
		qDebug() << reply.value(BLOCK_REPLY).toByteArray().toHex();
		failedDLNum++;
	}
/*
	else if (nonSeqDL) {
		processNonSeqBlockResponse(reply);
	}
*/
	else {
		if (currentDL.getLastRequested() == reply.value(BLOCK_REPLY).toByteArray()) {
			killRequestTimer();
			if (currentDL.getMetaHash() == currentDL.getLastRequested()) {
				qDebug() << "METAFILE REQUESTED";
				currentDL.setBlocklistHash(reply.value(DATA).toByteArray());
			}
			else {
				qDebug() << "ADDING TO FILE";
				currentDL.addFileBytes(reply.value(DATA).toByteArray());
			}
			currentDL.getNextRequest();

			if (currentDL.getLastRequested() == QByteArray("")) {
				downloading = false;
				qDebug() << "DONE DOWNLOADING";
				QFile tempFile(currentDL.getFileName());
				tempFile.open(QIODevice::WriteOnly);
				tempFile.write(currentDL.getFileBytes());
				tempFile.close();
				startNextDownload();
			}
			else {
				qDebug() << "REQUESTING MORE DATA";
				QVariantMap request;
				request.insert(DEST, reply.value(ORIGIN).toString());
				request.insert(ORIGIN, myOrigin);
				request.insert(HOP_LIMIT, 20);
				request.insert(BLOCK_REQUEST, currentDL.getLastRequested());
				lastRequest = request;

				QPair<QHostAddress, quint16> dest =
						dsdv.value(reply.value(ORIGIN).toString());
				send(serializeMsg(request), Peer(dest.first, dest.second));

				requestTimer = new QTimer(this);
				connect(requestTimer, SIGNAL(timeout()), this, SLOT(resendRequest()));
				requestTimer->start(5000);
			}
		}
		// Incorrect request, drop it
		else {
			qDebug() << "REPLY DOES NOT MATCH LAST REQUEST";
			qDebug() << "  Requested:" << currentDL.getLastRequested();
			qDebug() << "  Received: " << reply.value(BLOCK_REPLY).toByteArray();
			failedDLNum++;
		}
	}
}

void ChatDialog::killRequestTimer()
{
	requestTimer->stop();
}

void ChatDialog::resendRequest()
{
	if (failedDLNum < 5) {
		qDebug() << "DOWNLOAD TIMED OUT, RESENDING";
		QPair<QHostAddress, quint16> dest =
				dsdv.value(lastRequest.value(DEST).toString());
		lastRequest.insert(HOP_LIMIT, lastRequest.value(HOP_LIMIT).toUInt() * 2);
		qDebug() << "RESENDING:" << lastRequest;
		send(serializeMsg(lastRequest), Peer(dest.first, dest.second));
	}
	else {
		killRequestTimer();
		failedDLNum = 0;
		QString target = lastRequest.value(DEST).toString();
		QByteArray id = currentDL.getMetaHash();
		QString fileName = currentDL.getFileName();
		wantToDL.insert(fileName, QPair<QString, QByteArray>(target, id));
		foundForDL.append(fileName);
//		dlList->addItem(new QListWidgetItem(fileName));

		startNextDownload();
	}
}

void ChatDialog::startNextDownload()
{
	//TODO: fill this in
	if (!waitingToDL.isEmpty()) {
		FileMetadata nextDL = waitingToDL.takeFirst();
	}
}

////////////////////////////RUMORS////////////////////////////
void ChatDialog::readRumor(QVariantMap rumor, QHostAddress sender, quint16 senderPort)
{
	QString rumorOrigin = rumor.value(ORIGIN).toString();
	quint32 rumorSeqNo = rumor.value(SEQNO).toInt();
	int rumorSize = rumor.size();

	//qDebug() << "RUMOR FOUND from: " << rumorOrigin << " #" << rumorSeqNo;
	//qDebug() << "  At addr, port: " << sender << ", " << senderPort;

	if (rumorSeqNo < 1 || rumorOrigin == QString("")) {
		qDebug() << "INVALID RUMOR: " << rumor;
		return;
	}

	QHostAddress priorAddress = sender;
	quint16 priorPort = senderPort;

	if (rumor.contains(LAST_IP)) {
		quint32 temp = rumor.value(LAST_IP).toUInt();
		priorAddress = QHostAddress(temp);
		priorPort = rumor.value(LAST_PORT).toUInt();
	}

	rumor.insert(LAST_IP, sender.toIPv4Address());
	if (sender == QHostAddress(QHostAddress::LocalHost))
		rumor.insert(LAST_IP, QHostAddress(sock.getMyIP()).toIPv4Address());
	rumor.insert(LAST_PORT, senderPort);

	// new message origin
	if (!wantedSeqNos->contains(rumorOrigin)) {
		if ((rumorSize == 2 || rumorSize == 4) && !dsdv.contains(rumorOrigin))
			forwardAll(rumor);
		updateDSDV(rumorOrigin, priorAddress, priorPort);
		// if first message, add to maps, print it
		if (rumorSeqNo == 1) {
			updateReadMsgs(rumorOrigin, rumorSeqNo, QVariant(rumor));

			wantedSeqNos->insert(rumorOrigin, rumorSeqNo+1);
			if (rumor.contains(CHAT_TEXT))
				printRumor(rumor);
			sendStatus(sender, senderPort);
		}
		// otherwise, request first message
		else {
			QVariantMap myStatus = getMyStatus();
			myStatus.insert(rumorOrigin, 1);
			QVariantMap myWants;
			myWants.insert(WANT, myStatus);
			send(serializeMsg(myWants), Peer(sender, senderPort));
		}
	}
	// old message origin
	else {
		// If we've seen rumor and have later ones, send what they need
		if (wantedSeqNos->value(rumorOrigin) >= rumorSeqNo+1) {
			if (wantedSeqNos->value(rumorOrigin) == rumorSeqNo+1) {
				sendStatus(sender, senderPort);
				// prioritize messages from original sender in dsdv table
				if (rumorSize==3)
					updateDSDV(rumorOrigin, sender, senderPort);
			}
			else if (!noForward) {
				QVariantMap msg = readMsgs->value(rumorSeqNo+1).toMap()
						.value(rumorOrigin).toMap();
				send(serializeMsg(msg), Peer(sender, senderPort));
				addTimer();
			}
			else
				qDebug() << "I'm ignoring your needs";
		}
		// If we haven't seen it
		else {
			updateDSDV(rumorOrigin, priorAddress, priorPort);
		//	qDebug() << "\tand I haven't seen it";

			// If it is what we want, add it to the map and print
			if (rumorSeqNo == wantedSeqNos->value(rumorOrigin)) {
				//qDebug() << "\t adding message";
				if (rumor.contains(CHAT_TEXT))
					printRumor(rumor);

				updateReadMsgs(rumorOrigin, rumorSeqNo, QVariant(rumor));
				wantedSeqNos->insert(rumorOrigin, rumorSeqNo+1);

				if (rumorSize == 2 || rumorSize == 4)
					forwardAll(rumor);
			}
			sendStatus(sender, senderPort);
		}
	}
}

void ChatDialog::printRumor(QVariantMap map)
{
	QString message = "";
	if (map.size() == 4)
		message.append("<PRIVATE>");
	message.append(map.value(ORIGIN).toString() + " #" 
			+ map.value(SEQNO).toString() + ": " + map.value(CHAT_TEXT).toString());
	textview->append(message);
}

////////////////////////STATUSES/////////////////////////
void ChatDialog::checkStatus(QVariantMap wants, QHostAddress sender, quint16 senderPort)
{
	if (noForward)
		return;

	QMapIterator<QString, QVariant> wantIter(wants);
	//qDebug() << "STATUS RECEIVED from:" << sender << senderPort;
	quint32 wantSeqNo;
	QString wantPeer;
	bool messaged = false;
	
	while (wantIter.hasNext()) {
		wantIter.next();
		wantSeqNo = wantIter.value().toInt();
		wantPeer = wantIter.key();
		if (wantedSeqNos->contains(wantPeer)) {
			quint32 mySeqNo = wantedSeqNos->value(wantPeer);
			// we have the same messages, do nothing
			if (mySeqNo == wantSeqNo) {
				;
			}
			// they are missing what we have, send to them
			else if (mySeqNo > wantSeqNo) {
				QVariantMap msg = readMsgs->value(wantSeqNo).toMap()
						.value(wantPeer).toMap();
				//qDebug() << "\tSending missing message" << msg;
				send(serializeMsg(msg), Peer(sender, senderPort));
				messaged = true;
				addTimer();
			}
			// they have what we don't, send status request
			else if (mySeqNo < wantSeqNo) {
				//qDebug() << "\tthey have what I want";
				sendStatus(sender, senderPort);
				messaged = true;
			}
		}
		// they have contact we don't, send status request
		else {
			//qDebug() << "I do not have this contact: " << wantIter.key();
			wantedSeqNos->insert(wantIter.key(), 1);
			sendStatus(sender, senderPort);
			messaged = true;
		}
	}
	// after going through all their wants, see if we have
	// any peers they do not and send them if so
	if (!messaged) {
		QMapIterator<QString, quint32> myWantIter(*wantedSeqNos);
		while (myWantIter.hasNext()) {
			myWantIter.next();
			if (!wants.contains(myWantIter.key())) {
				//qDebug() << "THEY DO NOT HAVE THIS PEER" << myWantIter.key();
				QVariantMap oldMsg = readMsgs->value(1).toMap().
						value(myWantIter.key()).toMap();
				if (!oldMsg.isEmpty()) {
					send(serializeMsg(oldMsg), Peer(sender, senderPort));
					messaged = true;
				}
			}
		}
	}
	// if our wants match perfectly, flip a coin and pick a new
	// peer to rumormonger with
	if (!messaged) {
		pingRandomPeer();
	}
}

void ChatDialog::sendStatus(QHostAddress addr, quint16 port)
{
	// fill msg map
	QVariantMap msg;
	msg.insert(WANT, getMyStatus());

	QByteArray bytes;
	QDataStream outStream(&bytes, QIODevice::WriteOnly);
	outStream << msg;

	send(bytes, Peer(addr, port));
}

////////////////////////////////////////////////////
//ENTROPY PREVENTION AND TIMEOUTS
////////////////////////////////////////////////////
void ChatDialog::addTimer()
{
	QTimer *timer = new QTimer(this);
	connect(timer, SIGNAL(timeout()), this, SLOT(timedOutWaiting()));
	timer->start(1000);
	timerQueue->enqueue(timer);
}

void ChatDialog::preventEntropy()
{
	pingRandomPeer();
	addTimer();
}

void ChatDialog::timedOutWaiting()
{
	timerQueue->dequeue()->stop();
	pingRandomPeer();
}

void ChatDialog::pingRandomPeer()
{
	if (qrand()%2 == 1) {
		Peer rando = getRandomPeer();
		sendStatus(rando.getAddress(), rando.getPort());
	}
}

////////////////////////////////////////////////////////
// UTILITIES
//   Getters, serialization, map updating.
////////////////////////////////////////////////////////

QByteArray ChatDialog::serializeMsg(QVariantMap msg)
{
	QByteArray bytes;
	QDataStream outStream(&bytes, QIODevice::WriteOnly);
	outStream << msg;
	return bytes;
}

void ChatDialog::updateDSDV(QString origin, QHostAddress sender, quint16 senderPort)
{
	if (origin != myOrigin) {
		dsdv.insert(origin, QPair<QHostAddress, quint16>(sender, senderPort));

		// iterate through list of private peers, if origin is not present
		// then add it to the end of the list
		// code taken from stackoverflow at:
		// http://stackoverflow.com/questions/5496827/in-qlistwidget-how-do-
		//		i-check-if-qlistwidgetitem-already-exists-based-on-its-dat
		bool found = false;
		for (int i = 0; i < privateList->count(); i++)
			if (privateList->item(i)->data(Qt::DisplayRole).toString() == origin) {
				found = true;
				break;
			}
		if (!found)
			privateList->addItem(new QListWidgetItem(origin));
	}
	//qDebug() << "Updated dsdv for:" << origin;
	//qDebug() << dsdv.value(origin);
}

void ChatDialog::updateReadMsgs(QString peer, int num, QVariant msg)
{
	QVariantMap peerMessage;
	// if entry already exists, get the map
	if (readMsgs->contains(num))
		peerMessage = readMsgs->value(num).toMap();

	peerMessage.insert(peer, msg);
	readMsgs->insert(num, peerMessage);
}

QVariantMap ChatDialog::getMyStatus()
{
	QVariantMap myStatus;
	QMapIterator<QString, quint32> numIter(*wantedSeqNos);
	while (numIter.hasNext()) {
		numIter.next();
		myStatus.insert(numIter.key(), numIter.value());
	}
	return myStatus;
}

Peer ChatDialog::getRandomPeer()
{
	int r = qrand() % peers.size();
	foreach (Peer t, peers) {
		if (r == 0)
			return t;
		r--;
	}
	return Peer();
}

/////////////////////////////////////////////////////
// TESTING UTILITIES
/////////////////////////////////////////////////////

void ChatDialog::dumpReadMsgs()
{
	qDebug() << "Dumping read msgs";
	QMapIterator<quint32, QVariant> iter(*readMsgs);
	while (iter.hasNext()) {
		iter.next();
		qDebug() << "\t Dumping messages for: " << iter.key();
		QMapIterator<QString, QVariant> peerIter(iter.value().toMap());
		while (peerIter.hasNext()) {
			peerIter.next();
			qDebug() << "\t\tPeer: " << peerIter.key();
			qDebug() << "\t\t msg: " << peerIter.value();
		}
	}
}

void ChatDialog::dumpPeerData()
{
	qDebug() << "PEERDUMP";
	qDebug() << " wanted seqNos for peers are: " << wantedSeqNos->size();
	QMapIterator<QString, quint32> numIter(*wantedSeqNos);
	while (numIter.hasNext()) {
		numIter.next();
		qDebug() << "\t" << numIter.key() << ", " << numIter.value();
	}
	qDebug() << " Peers address/port pairs are:" << peers.size();
	Peer temp;
	foreach (temp, peers)
		qDebug() << "\t" << temp.getAddress() << temp.getPort();
}

////////////////////////////////////////////////////////
////////////////////////////////////////////////////////
////////////////////////////////////////////////////////

int main(int argc, char **argv)
{
	// Initialize Qt toolkit
	QCA::Initializer qcainit;
	QApplication app(argc,argv);

	QStringList args = QCoreApplication::arguments();
	args.removeFirst();

	// Create an initial chat dialog window
	ChatDialog dialog(args);
	dialog.show();

	// Enter the Qt main loop; everything else is event driven
	return app.exec();
}
