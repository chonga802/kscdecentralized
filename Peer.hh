#ifndef PEER_HH
#define PEER_HH

#include <QHostAddress>

class Peer
{
public:
	Peer();
	Peer(QHostAddress, quint16);

	QHostAddress getAddress() const;
	quint16 getPort() const;

	inline bool operator==(const Peer& rhs)
	{
		return (address == rhs.getAddress() &&
				port == rhs.getPort());
	}

private:
	QHostAddress address;
	quint16 port;
};

#endif // PEER_HH
