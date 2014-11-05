#include "NetSocket.hh"

#include <QHostInfo>

#include <unistd.h>
#include <stdlib.h>

///////////////////////////////////////////////////
// NetSocket class
///////////////////////////////////////////////////

NetSocket::NetSocket()
{
	// Pick a range of four UDP ports to try to allocate by default,
	// computed based on my Unix user ID.
	// This makes it trivial for up to four Peerster instances per user
	// to find each other on the same host,
	// barring UDP port conflicts with other applications
	// (which are quite possible).
	// We use the range from 32768 to 49151 for this purpose.
	myPortMin = 32768 + (getuid() % 4096)*4;
	myPortMax = myPortMin + 3;

	QHostInfo info = QHostInfo::fromName(QHostInfo::localHostName());
	QList<QHostAddress> l= info.addresses();
	myIP = l[0].toString();
	//qDebug() << "My IP address:" << myIP;
}

bool NetSocket::bind()
{
	// Try to bind to each of the range myPortMin..myPortMax in turn.
	for (int p = this->myPortMin; p <= this->myPortMax; p++) {
		if (QUdpSocket::bind(p)) {
			boundPort = p;
		//	qDebug() << "bound to UDP port " << p;
			return true;
		}
	}

	qDebug() << "Oops, no ports in my default range " << myPortMin
			<< "-" << myPortMax << " available";
	return false;
}

int NetSocket::getPortMin()
{ return this->myPortMin;}

int NetSocket::getPortMax()
{ return this->myPortMax;}

int NetSocket::getMyPort()
{ return this->boundPort;}

QString NetSocket::getMyIP()
{ return this->myIP;}


