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

class ChordDHT
{
public:

//////METHODS///////
	/* Constructs new empty ChordDHT:
		fills all finger entriew with given id		*/
	ChordDHT(QString);

	/* Updates finger table with peerID				*/
	bool updateFingers(QString);

	/* Returns index of given string in DHT			*/
	int getLocation(QString);

	/* Returns either predecessor in finger table of
		file, or actual tracker if that is known

		NOTE: if value returned is not this node,
		must message that node to confirm status
		as tracker for given file					*/
	QString getTracker(QString);

	/* returns true if in order in cycle, else false
		This means that when given A, B, C as args,
		returns true when:
		- A < B < C
		- C < A < B
		- B < C < A
		Else it returns false						*/
	bool assertOrder(int, int, int);

//////FIELDS//////
	/* Node's index in table */
	int myLoc;

	/* Node's predecessor */
	int predLoc;

	/* Node's id */
	QString myID;

	/* Predecessor's id */
	QString predID;

private:

	/* Finger table:
		each entry is the nearest successor to
		locations (my location) + 2^(finger #),
	   Initially all entries are self		*/
	QString finger[8];

};

#endif // CHORD_DHT_HH
