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

InitDialog::InitDialog() {

	layout = new QGridLayout();

	peerEntry = new QLineEdit();
	peerLabel = new QLabel("Join at:", peerEntry);

	idEntry = new QLineEdit();
	idLabel = new QLabel("Your ID:", idEntry);

	goButton = new QPushButton("Initialize BitTorrEnnant", this);

	layout->addWidget(idLabel, 1, 0);
	layout->addWidget(idEntry, 2, 0);
	layout->addWidget(peerLabel, 1, 1);
	layout->addWidget(peerEntry, 2, 1);

	fullLayout = new QGridLayout();
	fullLayout->addLayout(layout,0,0);
	fullLayout->addWidget(goButton,1,0);

	setLayout(fullLayout);
		qDebug() << "asdfasdf";

	connect(goButton, SIGNAL(clicked()), this, SLOT(loadID()));

}

void InitDialog::loadID() {

	QString id = idEntry->text();
	QString p = peerEntry->text();

	fullLayout->removeItem(layout);
	fullLayout->removeWidget(goButton);

	topLabel = new QLabel("Generating secure ID:");
	topLabel->setAlignment(Qt::AlignCenter);

	progressLabel = new QLabel(" ");
	progressLabel->setAlignment(Qt::AlignCenter);

	delete layout;
	delete goButton;
	delete idLabel;
	delete idEntry;
	delete peerLabel;
	delete peerEntry;

	fullLayout->addWidget(topLabel,0,0);
	fullLayout->addWidget(progressLabel,1,0);
	setLayout(fullLayout);
	QString newID;

	for (int counter = 0; ; counter++) {

		newID = id + "_" + QString::number(counter);

		QByteArray hash = QCryptographicHash::hash(newID.toUtf8(), QCryptographicHash::Md5);

		// Set size of bytes
		Q_ASSERT(hash.size() == 16);

		// Extract bytes
		QDataStream stream(hash);
		qint64 a, b;
		stream >> a >> b;

	//	qDebug() << id << "," << (((a ^ b) % 256) + 256) % 256;
		// return XOR of bits, mod 256 = pow(2, 8)
		int thingy = (((a ^ b) % (65535)) + 65535) % 65535;
	//	qDebug() << thingy;

		//if (counter%1000 == 0) {
		progressLabel->setText(newID);
		setLayout(fullLayout);
	//}
		if (rand() % 1000 == 0)
		QCoreApplication::processEvents();

		if (thingy == 42) {
			topLabel->setText("Success!");
			break;
		}
	}

	ChatDialog *dialog = new ChatDialog(QStringList(""));
	dialog->show();


}


ChatDialog::ChatDialog(QStringList args)
{
	// Create a UDP network socket
	if (!sock.bind())
		exit(1);

	qsrand(QTime::currentTime().msec()+1);
	// Hack to construct unique origin identifier
	myOrigin = "SAGE_";
	myOrigin.append(QString::number((1+QTime::currentTime().msec())
			* qrand()%1000000));

	chord = new ChordDHT(myOrigin);

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
//	dlButton = new QPushButton("Download file", this);
//	dlButton->setAutoDefault(false);
	dlList = new QListWidget(this);
	QLabel* dlLabel = new QLabel("Torrents available:", dlList);

	peerEntry = new QLineEdit();
	QLabel* peerLabel = new QLabel("Add peer:", peerEntry);

	privateList = new QListWidget(this);
	QLabel* privateListLabel = new QLabel("Seeding:", privateList);

	// Lay out the widgets to appear in the main window.
	QGridLayout *layout = new QGridLayout();

//	QHBoxLayout *peerEntryLayout = new QHBoxLayout();
//	peerEntryLayout->addWidget(peerLabel);
//	peerEntryLayout->addWidget(peerEntry);

	QVBoxLayout *privMsgLayout = new QVBoxLayout();
	privMsgLayout->addWidget(privateListLabel);
	privMsgLayout->addWidget(privateList);

	QVBoxLayout *textAndPeers = new QVBoxLayout();
	textAndPeers->addLayout(privMsgLayout);
	textAndPeers->addWidget(shareButton);
//	textAndPeers->addLayout(peerEntryLayout);

//	QHBoxLayout *files = new QHBoxLayout();
//	files->addWidget(dlButton);
//	files->addWidget(shareButton);


//    QPixmap pixmap(100,100);
//    pixmap.fill(QColor("transparent"));

//    QPainter painter(&pixmap);
//    painter.setBrush(QBrush(Qt::black));
 //   painter.drawRect(20, 10, 100, 100);

//    QLabel* al = new QLabel;
//    al->setPixmap(pixmap);


	QVBoxLayout *fileEntry = new QVBoxLayout();
	fileEntry->addWidget(dlLabel);
	fileEntry->addWidget(dlList);
//	fileEntry->addWidget(fileSearchLabel);
//	fileEntry->addWidget(fileSearch);
//	fileEntry->addLayout(files);

	// resize
	layout->addLayout(textAndPeers, 1, 0);
	layout->addLayout(fileEntry, 1, 1);
//	layout->addWidget(al, 1, 2);
	layout->setColumnStretch(0, 1);
	layout->setColumnStretch(1, 2);
//	layout->setColumnStretch(2, 2);
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
//	connect(peerEntry, SIGNAL(returnPressed()),
//			this, SLOT(processPeer()));
	// Register button click to open file selection dialog box
	connect(shareButton, SIGNAL(clicked()),
			this, SLOT(shareFile()));
	// Register button click to open file download box
//	connect(dlButton, SIGNAL(clicked()),
//			this, SLOT(startDownload()));
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

			QString fn = info.fileName();

			// Upload (for use as a seeder)
			FileMetadata newFile(file, fn,
					info.size(), blocklistHash, metaHash);
			filesForDL.append(newFile);

			// Forward to tracker / along chord table
			QString target = chord->getTracker(fn);

			QVariantMap msg;
			msg.insert("UploadNotice", fn);
			msg.insert("Seeder", myOrigin);
			msg.insert("BlockCount", blockCount);
			msg.insert("BlockListHash", blocklistHash);
			msg.insert("MetaHash", metaHash);
			msg.insert(DEST, target);

			QPair<QHostAddress, quint16> dest = dsdv.value(target);

			if (target == myOrigin) {
				readUploadNotice(msg);
			}
			else
				send(serializeMsg(msg), Peer(dest.first, dest.second));

			// Add file to tracked files (with self as first seeder)/
//			TrackedFileMetadata tracked(info.fileName(), blockCount, blocklistHash,
//				metaHash, myOrigin);
//			filesTracking.append(tracked);

			// Announce you now have file
			/////////// ADD : APPEND FILE TO UPLOADS DISPLAY
		}
	}
}

void ChatDialog::readUploadNotice(QVariantMap msg) {

	QString fn = msg["UploadNotice"].toString();
	QString target = chord->getTracker(fn);

	if (target == myOrigin) {
		QString seeder = msg["Seeder"].toString();
		int blockCount = msg["BlockCount"].toInt();
		QByteArray blocklistHash = msg["BlockListHash"].toByteArray();
		QByteArray metaHash = msg["MetaHash"].toByteArray();

		TrackedFileMetadata tracked(fn, blockCount, blocklistHash,
			metaHash, seeder);
		filesTracking.append(tracked);

		sendBroadcast();
	}
	else {
		msg[DEST] = target;
		QPair<QHostAddress, quint16> dest = dsdv.value(target);
		send(serializeMsg(msg), Peer(dest.first, dest.second));
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
		qDebug() << tracked.fileName + " " + QString::number(tracked.seederCount);
    }

	msg.insert("Broadcast", fileData);
	msg.insert(ORIGIN, myOrigin);

	if (!peers.isEmpty())
		forwardAll(msg);

	readBroadcast(msg);
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
			QString pad = " ";
			int padsize = 40 - mapIter.key().size();// - QString::number(mapIter.value().toInt()).size();

			for (int i = 0; i < padsize; i++) {
				pad = pad + " ";
			}
			pad = pad + "\t";
			QString display = QString::number(mapIter.value().toInt()) + "\t" + mapIter.key();
			dlList->addItem(new QListWidgetItem(display));
		}
	}
}

void ChatDialog::requestSeeders(QListWidgetItem *clicked)
{
	QStringList l = (clicked->text()).split("\t");
	QString file = l[1];

	int index = dlList->row(clicked);
	dlList->takeItem(index);

	// Get initial tracker from chord table
	QString target = chord->getTracker(file);

	// If we are not tracker, request data
	if (target != myOrigin) {

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

	// If we are tracker, just pass the data internally
	else {

		TrackedFileMetadata *found;
		bool blergh = false;
		int t = -1;
		foreach(TrackedFileMetadata meta, filesTracking) {
			t++;
			if (meta.fileName == file) {
				found = &meta;
				blergh = true;
				break;
			}
		}
		if (!blergh) {
			qDebug() << "info not in trackedfilemetadataasfes";
			return;
		}


		QVariantMap reply;
		reply.insert(DEST, target);
		reply.insert(ORIGIN, myOrigin);
		reply.insert("SeedReply", file);
		reply.insert("MetaFileID", found->metaHash);
		reply.insert("BlockListHash", found->blocklistHash);
		reply.insert("Seeders", found->seeders);
		reply.insert("BlockCount", found->blockCount);

		qDebug() << "I HAVE THE SEEDERS ALREADYYYY";
		qDebug() << "Passing to meeee";
	//	qDebug() << reply;
	//	sendBlockRequestToSeeders(reply);


	// If we are not seeding already, add us to list of seeders
		qDebug() << "Updating Seed List";
		QStringList intermediary = found->seeders;
		qDebug() << intermediary;
		if (!intermediary.contains(myOrigin)) {
			intermediary.append(myOrigin);
			TrackedFileMetadata moreSeeds(found->fileName,
				found->blockCount,
				found->blocklistHash,
				found->metaHash,
				intermediary[0]);
			for (int i = 1; i < intermediary.length(); i++) {
				moreSeeds.seeders.append(intermediary[i]);
			}
			moreSeeds.seederCount = intermediary.length();
			filesTracking.replace(t, moreSeeds);
			qDebug() << "File " + filesTracking[t].fileName + " has new seeds " << filesTracking[t].seeders;
		}

	}
}

void ChatDialog::replySeeders(QVariantMap msg)
{
	QString file = msg["SeedRequest"].toString();

	QString requestor = msg[ORIGIN].toString();

	// If I have the data for this file
	if ((chord->getTracker(file)) == myOrigin) {

		TrackedFileMetadata *found;
		bool blergh = false;
		int t = -1;
		foreach(TrackedFileMetadata meta, filesTracking) {
			t++;
			if (meta.fileName == file) {
				found = &meta;
				blergh = true;
				break;
			}
		}
		if (!blergh) {
			qDebug() << "info not in trackedfilemetadataasfes";
			return;
		}

		QVariantMap reply;
		reply.insert(DEST, requestor);
		reply.insert(ORIGIN, myOrigin);
		reply.insert("SeedReply", file);
		reply.insert("MetaFileID", found->metaHash);
		reply.insert("BlockListHash", found->blocklistHash);
		reply.insert("Seeders", found->seeders);
		reply.insert("BlockCount", found->blockCount);

		// And send it back to who requested it
		QPair<QHostAddress, quint16> dest = dsdv.value(requestor);
		send(serializeMsg(reply), Peer(dest.first, dest.second));

		// TODO: Then add them as a seeder, if they are not already
		
		qDebug() << "Sending along data ";
	//	qDebug() << reply;

		QStringList intermediary = found->seeders;
		if (!intermediary.contains(requestor)) {
			intermediary.append(requestor);

			TrackedFileMetadata moreSeeds(found->fileName,
				found->blockCount,
				found->blocklistHash,
				found->metaHash,
				intermediary[0]);

			for (int i = 1; i < intermediary.length(); i++) {
				moreSeeds.seeders.append(intermediary[i]);
			}
			moreSeeds.seederCount = intermediary.length();

			filesTracking.replace(t, moreSeeds);

			qDebug() << "File " + filesTracking[t].fileName + " has new seeds " << filesTracking[t].seeders;
		}
	}
	// Otherwise
	else {
		// Forward the message to the next tracker
		//	Do not change origin, as will eventually
		//	need to return to them. Just update destination
		msg.insert(DEST, chord->getTracker(file));

		QPair<QHostAddress, quint16> dest = dsdv.value(msg[ORIGIN].toString());
		send(serializeMsg(msg), Peer(dest.first, dest.second));
	}
}


///////////////////////////////////////////////////////////////////
// private messages and routing
///////////////////////////////////////////////////////////////////

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
			if (msg.contains(BLOCK_REQUEST))
				sendBlockReply(msg);
			else if (msg.contains(BLOCK_REPLY))
				processBlockReply(msg);
			else if (msg.contains("SeedRequest"))
				replySeeders(msg);
			else if (msg.contains("UploadNotice"))
				readUploadNotice(msg);
		}
		// must forward
		else if (msg.value(HOP_LIMIT).toInt() > 0) {
			msg.insert(HOP_LIMIT, msg.value(HOP_LIMIT).toInt()-1);
			QPair<QHostAddress, quint16> destData = dsdv.value(dest);

			send(serializeMsg(msg), Peer(destData.first, destData.second));
		}
	}
	// Rumors
	else
		readRumor(msg, sender, senderPort);
}

//////////////////////////////////////////////////////////////
//	FILE DOWNLOADING:
//		Process requests/replies
//		Timer behavior
//		Start download
//////////////////////////////////////////////////////////////

void ChatDialog::sendBlockRequestToSeeders(QVariantMap msg){

	QByteArray fileID = msg.value("MetaFileID").toByteArray();
	QByteArray blockListHash = msg.value("BlockListHash").toByteArray();
	QStringList seeders = msg.value("Seeders").toStringList();
	int blockCount = msg.value("BlockCount").toInt();
	//fill blocksLeft
	for(int i = 0; i < blockCount; i++){
		blocksLeft.append(i);
	}
	//request blocks from seeders until you have all of them
	int numBlocksLeft = blocksLeft.length();
	//for every seeder request different block
	for(int i = 0; i < seeders.size(); i++){
		int blockNum = blocksLeft.at(i % numBlocksLeft);
		//send block request to seeder
		sendBlockRequest(blockNum, seeders.at(i), fileID, blockListHash);
	}
	//set up timer in case all seeders time out

}

void ChatDialog::sendBlockRequest(int blockNum, QString seeder, QByteArray fileID, QByteArray blockListHash)
{
	//construct block request
	QVariantMap blockRequest;
	blockRequest.insert("Dest", seeder);
	blockRequest.insert("Origin", myOrigin);
	blockRequest.insert("BlockNum", blockNum);
	blockRequest.insert("MetaFileID", fileID);
	blockRequest.insert("BlockListHash", blockListHash);
	//send block request to target seeder
	QPair<QHostAddress, quint16> dest = dsdv.value(seeder);
	//qDebug() << "sending block request of" << blockRequest << "to" << dest.first << "," << dest.second;
	send(serializeMsg(blockRequest), Peer(dest.first, dest.second));	
}

void ChatDialog::sendBlockReply(QVariantMap msg){
	QByteArray fileID = msg.value("MetaFileID").toByteArray();
	QByteArray blockListHash = msg.value("BlockListHash").toByteArray();
	int blockNum = msg.value("BlockNum").toInt();
	QString pathName;
	//find fileName that matches fileID
	for(int i = 0; i < filesForDL.size(); i++){
		FileMetadata f = filesForDL.at(i);
		if(f.getMetaHash()==fileID){
			pathName = f.getFullName();
			msg.insert("FileName",f.getFileName());
			break;
		}
	}
    QByteArray blocklist;
    QCryptographicHash crypto(QCryptographicHash::Sha1);
	QFile file(pathName);
	file.open(QFile::ReadOnly);
	QByteArray data;
	//divide file into 8KB blocks
	for(int i = 0; i < blockNum+1; i++){
	  	data = file.read(8192);
	}
	msg.insert("FileBlock", data);
	//send file block to block requester;
	QString origin = msg.value("Origin").toString();
	msg.insert("Dest", origin);
	msg.insert("Origin", myOrigin);
	QPair<QHostAddress, quint16> dest = dsdv.value(origin);
	send(serializeMsg(msg), Peer(dest.first, dest.second));	
}

void ChatDialog::processBlockReply(QVariantMap msg){
	QByteArray fileID = msg.value("MetaFileID").toByteArray();
	QByteArray blockListHash = msg.value("BlockListHash").toByteArray();
	QString seeder = msg.value("Origin").toString();
	QByteArray data = msg.value("FileBlock").toByteArray();
	int blockNum = msg.value("BlockNum").toInt();
	if(blocksLeft.contains(blockNum)){
		//remove blockNum from blocksLeft
		blocksLeft.removeAll(blockNum);
		//store file in blocksAcquired
		blocksAcquired.insert(blockNum, data);
		//if still have blocks left you need, send block request to seeder
		int numBlocksLeft = blocksLeft.length();
		if(numBlocksLeft > 0){
			int blockNum = blocksLeft.at(rand() % numBlocksLeft);
			//send block request to seeder
			sendBlockRequest(blockNum, seeder, fileID, blockListHash);
		}
		//else, save the new file you have acquired to desktop
		else{
			//concatenate file blocks in blocksAcquired into single block
			QByteArray fileBlocks;
			for(int i = 0; i < blocksAcquired.size(); i++){
				QByteArray block = blocksAcquired.value(i);
				fileBlocks.append(block);
			}
			//save file
			QString fileName = msg.value("FileName").toString();
			QFile *savedFile = new QFile(fileName);
			savedFile->open(QFile::WriteOnly);
			savedFile->write(fileBlocks);
			savedFile->close();
		}
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
			else
				qDebug() << "I'm ignoring your needs";
		}
		// If we haven't seen it
		else {
			updateDSDV(rumorOrigin, priorAddress, priorPort);
		//	qDebug() << "\tand I haven't seen it";

			// If it is what we want, add it to the map and print
			if (rumorSeqNo == wantedSeqNos->value(rumorOrigin)) {

				updateReadMsgs(rumorOrigin, rumorSeqNo, QVariant(rumor));
				wantedSeqNos->insert(rumorOrigin, rumorSeqNo+1);

				if (rumorSize == 2 || rumorSize == 4)
					forwardAll(rumor);
			}
			sendStatus(sender, senderPort);
		}
	}
}

////////////////////////STATUSES/////////////////////////
void ChatDialog::checkStatus(QVariantMap wants, QHostAddress sender, quint16 senderPort)
{
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
			// they are missing what we have, send to them
			if (mySeqNo > wantSeqNo) {
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
	if (qrand() % 2 == 1) {
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
		chord->updateFingers(origin);
	}
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
//	InitDialog *dialog = new InitDialog();
	ChatDialog *dialog = new ChatDialog(args);
	
	dialog->show();

	// Enter the Qt main loop; everything else is event driven
	return app.exec();
}
