#ifndef TRACKED_FILE_METADATA_HH
#define TRACKED_FILE_METADATA_HH

#include <QString>
#include <QStringList>
#include <QByteArray>

class TrackedFileMetadata
{
public:
	// constructors
	TrackedFileMetadata(QString, int, QByteArray, QByteArray, QString);

	// methods


	// getters




	// setters



public:
	QString fileName;
	int blockCount;
	QByteArray blocklistHash;
	QByteArray metaHash;

	QStringList seeders;
	int seederCount;
};

#endif // TRACKED_FILE_METADATA_HH
