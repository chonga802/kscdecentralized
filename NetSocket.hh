#ifndef NET_SOCKET_HH
#define NET_SOCKET_HH

#include <QString>
#include <QUdpSocket>

class NetSocket : public QUdpSocket
{
	Q_OBJECT

public:
	NetSocket();

	// Bind this socket to a Peerster-specific default port.
	bool bind();

	int getPortMin();
	int getPortMax();
	int getMyPort();
	QString getMyIP();

private:
	int myPortMin, myPortMax, boundPort;
	QString myIP;
};

#endif // NET_SOCKET_HH
