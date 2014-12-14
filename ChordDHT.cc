#include "ChordDHT.hh"

#include <QDebug>
#include <QStringList>
#include <QCryptographicHash>

ChordDHT::ChordDHT(QString id) {
	qDebug() << "Creating Chord table for" << id;
	int i, two;

	myID = id;
	myLoc = getLocation(id);
	qDebug() << "My location in table is" << myLoc;

	/* initially, predecessor is self - only one in network */
	predLoc = myLoc;

	two = 1;
	for (i = 0; i < 8; i++) {
		finger[i] = myID;
		qDebug() << "\tFinger" << i << "is" << id << "starts at" << (myLoc + two) % 256;
		two *= 2;
	}
}

/* seems to be working */
void ChordDHT::addFile(QString name,
		QByteArray metaID,
		QString seed,
		QByteArray bytes) {

	if (fileMetaMap.contains(name))
		qDebug() << "ERROR: File already inserted, overwriting";

	fileSeedMap.insert(name, QStringList(seed));
	fileHashMap.insert(name, bytes);
	fileMetaMap.insert(name, metaID);
}

/* working */
void ChordDHT::addSeed(QString name, QString seed) {

	QStringList seeds;

	// extract all current seeds for file
	seeds = fileSeedMap.value(name);

	// append new seed to list
	seeds.append(seed);

	// reinsert
	fileSeedMap.insert(name, seeds);
}

/* TODO: TEST THE SHIT OUT OF THIS
*
*	- peer updating appears to work correctly
*	- as does predecessor calculation
*
*	- need to do sharing files with predecessor
*/
bool ChordDHT::updateFingers(QString peerID) {
	int peerLoc, fingerLoc, startLoc, d, i;

	// get location of new peer in table
	peerLoc = getLocation(peerID);

//	qDebug() << "Checking fingers for new friend" << peerID << "at" << peerLoc;

	if (peerLoc == myLoc) {
		qDebug() << "NODE COLLISION: #FUCKED";
		qDebug() << "\t" << myID << "," << peerID;
	}

	d = 1;
	// for each finger
	for (i = 0; i < 8; i++) {
		// get where finger's arc starts
		startLoc = (myLoc + d) % 256;
		// find current successor
		fingerLoc = getLocation(finger[i]);

		qDebug() << "finger" << i << ":" << startLoc << ", new:" << peerLoc << ", old:" << fingerLoc;

		/*
		* if start location and new peer collide, obviously peer
		*	needs to occupy this spot in finger table, and it will
		*	certainly not appear in any other fingers, so break
		*/ 
		if (startLoc == peerLoc) {
			finger[i] = peerID;
		}
		/* if finger < new peer < old finger successor, update */
		else if (assertOrder(startLoc, peerLoc, fingerLoc))
			finger[i] = peerID;

		/* update start location */
		d *= 2;
	}

	if (predLoc == myLoc)
		predLoc = peerLoc;
	// check if new peer is your immediate predecessor
	else if (assertOrder(predLoc, peerLoc, myLoc))
		predLoc = peerLoc;


	qDebug() << "New state of" << myID;
	qDebug() << "\tpredLoc is" << predLoc;
	for (i = 0; i < 8; i++) {
		qDebug() << "\tFinger" << i << ":" << finger[i];
	}

	// if new predecessor, then must pass it files, return true
	if (peerLoc == predLoc) {
		return true;
	}
	return false;
}

/* seems to be working consistently, should test x-platform */
int ChordDHT::getLocation(QString id) {

// Adopted from
// http://stackoverflow.com/questions/22000005/built-in-64-bit-hash-function-for-qstring

	//Generate hash
	QByteArray hash = QCryptographicHash::hash(id.toUtf8(), QCryptographicHash::Sha1);

	// Set size of bytes
	Q_ASSERT(hash.size() == 8);

	// Extract bytes
	QDataStream stream(hash);
	qint64 a, b;
	stream >> a >> b;

//	qDebug() << id << "," << (((a ^ b) % 256) + 256) % 256;
	// return XOR of bits, mod 256 = pow(2, 8)
	return (((a ^ b) % 256) + 256) % 256;

}


int ChordDHT::getNumSeeds(QString name) {

	if(!fileMetaMap.contains(name)) {
		qDebug() << "ERROR: File" << name << "not found.";
		return 0;
	}
	else
		return fileSeedMap.value(name).size();
}


int ChordDHT::getNumBlocks(QString name) {

	if(!fileHashMap.contains(name)) {
		qDebug() << "ERROR: File" << name << "not found.";
		return 0;
	}
	else {
		QByteArray blockList = fileHashMap.value(name);
		return blockList.size() / 20;
	}
}

/* TODO: TEST THIS THOROUGHLY
*
*	seems okay right now? tested on a couple cases
*
*/
QString ChordDHT::getTracker(QString name) {

	qDebug() << "FINDING:" << name;

	int fileLoc, fin, step;

	/* Check if I have the file, return my ID if so */
	if (fileHashMap.contains(name)) {
		qDebug() << "\tI'VE GOT IT";
		return myID;
	}
	else {
		qDebug() << "\t I DON'T HAVE IT";
		fileLoc = getLocation(name);
		qDebug() << "IT'S AT" << fileLoc;
		// get distance between current location and target
		fileLoc = ((fileLoc + 256) - myLoc) % 256;

		// find nearest finger, start at furthest and work down
		fin = 7;
		step = 128;
		while (fileLoc < step) {
			step /= 2;
			fin--;
		}
		return finger[fin];
	}
}


QStringList ChordDHT::getSeeders(QString name) {

	// Check if we have file, return if so
	if(fileSeedMap.contains(name))
		return fileSeedMap.value(name);

	// else throw error, return empty list
	else
		qDebug() << "ERROR: file" << name << "not found.";

	/* questionable return, should think about this */
	return QStringList("");
}


QByteArray ChordDHT::getBytes(QString name) {

	if(fileHashMap.contains(name))
		return fileHashMap.value(name);

	else
		qDebug() << "ERROR: file" << name << "not found.";

	return NULL;
}


QByteArray ChordDHT::getMeta(QString name) {

	if(fileMetaMap.contains(name))
		return fileMetaMap.value(name);

	else
		qDebug() << "ERROR: file" << name << "not found.";

	return NULL;
}


bool ChordDHT::assertOrder(int a, int b, int c) {

	if ((a < b) && (b < c))
		return true;
	else if ((b < c) && (c < a))
		return true;
	else if ((c < a) && (a < b))
		return true;
	else
		return false;
}
