#include "FileMetadata.hh"

#include <QDebug>

FileMetadata::FileMetadata()
{}

FileMetadata::FileMetadata(QString fileName, QByteArray metaHash)
{
	this->fileName = fileName;
	this->metaHash = metaHash;

	this->lastRequested = metaHash;
}

FileMetadata::FileMetadata(QString fullName, QString fileName, int fileSize,
							QByteArray blocklistHash, QByteArray metaHash)
{
	this->fullName = fullName;
	this->fileName = fileName;
	this->fileSize = fileSize;
	this->blocklistHash = blocklistHash;
	this->metaHash = metaHash;
}

int FileMetadata::findBytesIndex(QByteArray wanted)
{
	int place = blocklistHash.indexOf(wanted);
	qDebug() << "INDEX FOUND:" << place;
	qDebug() << "File id hash:" << metaHash.toHex();
	qDebug() << "BL hash dump:" << blocklistHash.toHex();
	if (place == -1)
		return place;
	place /= 20;
	return place;
}

QByteArray FileMetadata::getNextRequest()
{
	if (lastRequested.isEmpty()) {
		lastRequested = metaHash;
		return metaHash;
	}
	else if (lastRequested == metaHash) {
		lastRequested = QByteArray(blocklistHash, 20);
		return lastRequested;
	}
	else {
		blocklistHash.remove(0, 20);
		lastRequested = QByteArray(blocklistHash, 20);
		if (blocklistHash.size() == 0) {
			lastRequested = QByteArray("");
			return QByteArray("");
		}
		qDebug() << "BLOCKLIST:" << blocklistHash.toHex();
		return lastRequested;
	}
}

void FileMetadata::addFileBytes(QByteArray data)
{
	fileBytes.append(data);
}

void FileMetadata::setBlocklistHash(QByteArray data)
{
	this->blocklistHash = data;
}

QString FileMetadata::getFullName() const
{ return this->fullName;}

QString FileMetadata::getFileName() const
{ return this->fileName;}

int FileMetadata::getFileSize() const
{ return this->fileSize;}

QByteArray FileMetadata::getBlocklistHash() const
{ return this->blocklistHash;}

QByteArray FileMetadata::getMetaHash() const
{ return this->metaHash;}

QByteArray FileMetadata::getLastRequested() const
{ return this->lastRequested;}

QByteArray FileMetadata::getFileBytes() const
{ return this->fileBytes;}
