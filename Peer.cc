#include "Peer.hh"

///////////////////////////////////////////////////////
// Peer class
///////////////////////////////////////////////////////

// empty constructor for convenience
Peer::Peer()
{}

Peer::Peer(QHostAddress address, quint16 port)
{
	this->address = address;
	this->port = port;
}

QHostAddress Peer::getAddress() const
{ return this->address;}

quint16 Peer::getPort() const
{ return this->port;}
