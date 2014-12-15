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
	predID = myID;

	two = 1;
	for (i = 0; i < 8; i++) {
		finger[i] = myID;
		qDebug() << "\tFinger" << i << "is" << id << "starts at" << (myLoc + two) % 256;
		two *= 2;
	}
}

bool ChordDHT::updateFingers(QString peerID) {
	int peerLoc, fingerLoc, startLoc, d, i;

	// get location of new peer in table
	peerLoc = getLocation(peerID);

	if (peerLoc == myLoc) {
		qDebug() << "NODE COLLISION" << myID << "," << peerID;
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

	// check if new peer is your immediate predecessor
	if (predLoc == myLoc) {
		predLoc = peerLoc;
		predID = myID;
	}
	else if (assertOrder(predLoc, peerLoc, myLoc)) {
		predLoc = peerLoc;
		predID = peerID;
	}

	// Display state
	qDebug() << "New state of" << myID;
	qDebug() << "\tpredLoc is" << predLoc;
	for (i = 0; i < 8; i++) {
		qDebug() << "\tFinger" << i << ":" << finger[i];
	}

	// If new predecessor, then must pass it files, return true
	if (peerLoc == predLoc) {
		return true;
	}
	return false;
}

int ChordDHT::getLocation(QString id) {

	//Generate hash
	QByteArray hash = QCryptographicHash::hash(id.toUtf8(), QCryptographicHash::Sha1);

	// Set size of bytes
	Q_ASSERT(hash.size() == 8);

	// Extract bytes
	QDataStream stream(hash);
	qint64 a, b;
	stream >> a >> b;

	// return XOR of bits, mod 256 = pow(2, 8)
	return (((a ^ b) % 256) + 256) % 256;

}

QString ChordDHT::getTracker(QString name) {

	qDebug() << "FINDING:" << name;

	int fileLoc, fin, step;

	/* Check if I have the file, return my ID if so */
	if (assertOrder(myLoc, predLoc, getLocation(name))) {
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
