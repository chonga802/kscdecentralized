#include "ChordDHT.hh"

#include <QDebug>
#include <QStringList>
#include <QCryptographicHash>

void ChordDHT::initDHT(QString id) {
	int i;

	myID = id;
	myLoc = getLocation(id);

	for (i = 0; i < 8; i++) {
		finger[i] = myID;
	}
}

/*
	WARNING TO SAGE: TEST THIS THOROUGHLY
	TEST
	TEST
	TEST
	TEST
	TEST
	TEST
	TEST
	TEST
	TEST
	PLEASE
*/
QString ChordDHT::getTracker(QString name, QByteArray metaID) {
	QPair<QString, QByteArray> file = QPair<QString, QByteArray>(name, metaID);

	/* Check if I have the file, return my ID if so */
	if (fileHashMap.contains(file)) {
		return myID;
	}
	else {
		int fileLoc = getLocation(name);
		// get distance between current location and target
		fileLoc = (fileLoc - (256 + myLoc)) % 256;

		// find nearest finger
		int fin = 0;
		int step = 1;
		while (fileLoc > step) {
			step *= 2;
			fin++;
		}
		if (fileLoc == step)
			return finger[fin];
		else
			return finger[fin - 1];
	}
}


void ChordDHT::addFile(QString name, QByteArray metaID, QString seed, QByteArray bytes) {
	QPair<QString, QByteArray> file = QPair<QString, QByteArray>(name, metaID);

	if (fileHashMap.contains(file))
		qDebug() << "ERROR: File already inserted, overwriting";

	fileSeedMap.insert(file, QStringList(seed));
	fileHashMap.insert(file, bytes);
}


void ChordDHT::addSeed(QString name, QByteArray metaID, QString seed) {
	QPair<QString, QByteArray> file = QPair<QString, QByteArray>(name, metaID);

	// extract all current seeds for file
	QStringList seeds = fileSeedMap.value(file);

	// append new seed to list
	seeds.append(seed);

	// reinsert
	fileSeedMap.insert(file, seeds);
}


QStringList ChordDHT::getSeeders(QString name, QByteArray metaID) {

	if(fileSeedMap.contains(QPair<QString, QByteArray>(name, metaID)))
		return fileSeedMap.value(QPair<QString, QByteArray>(name, metaID));

	else
		qDebug() << "ERROR: file" << name << "with id" << metaID.toHex() << "not found.";

	return QStringList("");
}


int ChordDHT::getLocation(QString id) {
	/* FUCK WE'RE GONNA HAVE TO MAKE A HASH FUNCTION DAMMIT
		here's some sample code:
http://stackoverflow.com/questions/22000005/built-in-64-bit-hash-function-for-qstring

	should use QString::toUtf8() instead of the utf16 call
	current test code below

	int collisions[256];
	for (int i = 0; i < 256; i++)
		collisions[i] = 0;

	for (int i = 0; i < 30; i++) {
QString str = QString::number(i);
QByteArray hash = QCryptographicHash::hash(
    QByteArray::fromRawData((const char*)str.toUtf8(), str.length()*2),
    QCryptographicHash::Sha1
  );
  Q_ASSERT(hash.size() == 8);
  QDataStream stream(hash);
  qint64 a, b;
  stream >> a >> b;

	collisions[(((a ^ b) % 256)  + 256) % 256]++;
  //qDebug() << "Testing for 'Test'" << (((a ^ b) % 256)  + 256) % 256;

}
	int fuck;
	fuck = 0;
	for (int i = 0; i < 256; i++) {
		if (collisions[i] > 1)
			fuck += collisions[i] - 1;
	}
	qDebug() << "Collisions:" << fuck;

	 */
	return 0;
}


QByteArray ChordDHT::getBytes(QString name, QByteArray metaID) {

	if(fileHashMap.contains(QPair<QString, QByteArray>(name, metaID)))
		return fileHashMap.value(QPair<QString, QByteArray>(name, metaID));

	else
		qDebug() << "ERROR: file" << name << "with id" << metaID.toHex() << "not found.";

	return NULL;
}

