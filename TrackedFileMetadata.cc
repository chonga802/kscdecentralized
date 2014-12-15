#include "TrackedFileMetadata.hh"

#include <QDebug>


TrackedFileMetadata::TrackedFileMetadata(QString fileName, int blockCount,
							QByteArray blocklistHash, QByteArray metaHash,
							QString firstSeeder)
{
	this->fileName = fileName;
	this->blockCount = blockCount;
	this->blocklistHash = blocklistHash;
	this->metaHash = metaHash;
	this->seeders.append(firstSeeder);
	this->seederCount = 1;
}

TrackedFileMetadata::TrackedFileMetadata(QString fileName, int blockCount,
							QByteArray blocklistHash, QByteArray metaHash,
							QStringList seeders)
{
	this->fileName = fileName;
	this->blockCount = blockCount;
	this->blocklistHash = blocklistHash;
	this->metaHash = metaHash;
	this->seeders = seeders;
	this->seederCount = seeders.length();
}




