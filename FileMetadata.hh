#ifndef FILE_METADATA_HH
#define FILE_METADATA_HH

#include <QString>
#include <QByteArray>

class FileMetadata
{
public:
	// constructors
	FileMetadata();
	FileMetadata(QString, QByteArray);
	FileMetadata(QString, QString, int, QByteArray, QByteArray);

	// methods
	int findBytesIndex(QByteArray);
	QByteArray getNextRequest();

	// getters
	QString getFullName() const;
	QString getFileName() const;
	int getFileSize() const;
	QByteArray getBlocklistHash() const;
	QByteArray getMetaHash() const;
	QByteArray getFileBytes() const;

	QByteArray getLastRequested() const;

	// setters
	void addFileBytes(QByteArray);
	void setBlocklistHash(QByteArray);

	// operators
	inline bool operator==(const FileMetadata& rhs)
	{
		return (fileName == rhs.getFileName() &&
				fileSize == rhs.getFileSize());
	}

private:
	QString fullName;
	QString fileName;
	int fileSize;
	QByteArray blocklistHash;
	QByteArray metaHash;

	QByteArray fileBytes;
	QByteArray lastRequested;
};

#endif // FILE_METADATA_HH
