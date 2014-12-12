#ifndef CHORD_DHT_HH
#define CHORD_DHT_HH

#include <QByteArray>
#include <QString>
#include <QMap>
#include <QPair>

class ChordDHT
{
public:

	/* Initializes new empty ChordDHT:
		fills all finger entriew with given id
		should only be called once at startup		*/
	void initDHT(QString);

	/* Returns either predecessor in finger table of
		file, or actual tracker if that is known

		NOTE: if value returned is not this node,
		must message that node to confirm status
		as tracker for given file					*/
	QString getTracker(QString, QByteArray);

	/* Adds file to this nodes maps					*/
	void addFile(QString, QByteArray, QString, QByteArray);

	/* Adds node as a seed for specified file		*/
	void addSeed(QString, QByteArray, QString);

	/* Returns seeds for given file					*/
	QStringList getSeeders(QString, QByteArray);

	/* Returns index of given string in DHT			*/
	int getLocation(QString);

	/* Returns blocklist hash for given file		*/
	QByteArray getBytes(QString, QByteArray);

private:

	/* Node's location in table */
	int myLoc;

	/* Node's id */
	QString myID;

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
