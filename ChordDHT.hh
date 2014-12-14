#ifndef CHORD_DHT_HH
#define CHORD_DHT_HH

/***************************************************************
*
*	ChordDHT.hh
*
*	API and class definitions for Chord distributed hash table
*
*	Chord protocol defines a DHT wherein each node in the
*	network contols all items which hash to a location between
*	the node itself and the preceeding node. In our (KSC)
*	implementation, this means that all relevant metadata for
*	a file (i.e. blocklist hash and list of seeders) will be
*	stored in the node immediately succeeding its location
*	within the hash table.
*
***************************************************************/

#include <QByteArray>
#include <QString>
#include <QMap>
#include <QPair>

/*
	//TEST CODE FOR MAIN

	ChordDHT fuckit("Sagey");

	fuckit.updateFingers("Friend!");
	fuckit.updateFingers("Enemy");
	fuckit.updateFingers("Boss!");
	fuckit.updateFingers("Rando!");
	fuckit.updateFingers("mothafucka");
	fuckit.updateFingers("t");

	fuckit.addFile("pray", QByteArray(), "Myself", QByteArray());

	qDebug() << "testing owned by" << fuckit.getTracker("testing", QByteArray());
	qDebug() << "Space Jam owned by" << fuckit.getTracker("Space Jam", QByteArray());
	qDebug() << "lolz owned by" << fuckit.getTracker("lolz", QByteArray());
	qDebug() << "pray owned by" << fuckit.getTracker("pray", QByteArray());
*/

class ChordDHT
{
public:

	/* Constructs new empty ChordDHT:
		fills all finger entriew with given id		*/
	ChordDHT(QString);

	/* Adds file to this nodes maps, should only be
		called the first time a file is uploaded	*/
	void addFile(QString, QByteArray, QString, QByteArray);

	/* Adds node as a seed for specified file		*/
	void addSeed(QString, QByteArray, QString);

	/* Updates finger table with peerID				*/
	void updateFingers(QString);

	/* Returns index of given string in DHT			*/
	int getLocation(QString);

	/* Returns either predecessor in finger table of
		file, or actual tracker if that is known

		NOTE: if value returned is not this node,
		must message that node to confirm status
		as tracker for given file					*/
	QString getTracker(QString, QByteArray);

	/* Returns seeds for given file					*/
	QStringList getSeeders(QString, QByteArray);

	/* Returns blocklist hash for given file		*/
	QByteArray getBytes(QString, QByteArray);

private:

	/* Node's index in table */
	int myLoc;

	/* Node's id */
	QString myID;

	/* Node's predecessor */
	int predLoc;;

	/* Finger table:
		each entry is the nearest successor to
		locations (my location) + 2^(finger #),
	   Initially all entries are self		*/
	QString finger[8];

	/* Key:		Pair of filename/metafile ID
	   Value:	List of known seeds for key	*/
	QMap<QPair<QString, QByteArray>, QStringList> fileSeedMap;

	/* Key:		Pair of filename/metafile ID
	   Value:	Blocklist hash for the file	*/
	QMap<QPair<QString, QByteArray>, QByteArray> fileHashMap;

};

#endif // CHORD_DHT_HH