/*  tpserver abstract connection class
 *
 *  Copyright (C) 2010 Kornel Kisielewicz and the Thousand Parsec Project
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include "connection.h"

#include <boost/make_shared.hpp>
#include <boost/bind.hpp>

Connection::Connection( 
	boost::asio::io_service& aIOS, 
	boost::asio::io_service::strand& aStrand 
)
 : mSocket( aIOS ),
   mStrand( aStrand )
{
	// no code
}

void Connection::read( boost::asio::mutable_buffers_1 aBuffer, WriteCallback onComplete )
{
	// normal operation
	mSocket.async_read_some( 
		aBuffer,
		mStrand.wrap(
			boost::bind( 
				onComplete,
				boost::asio::placeholders::error,
				boost::asio::placeholders::bytes_transferred
			)
		)
	);
}


void Connection::write( boost::asio::const_buffers_1 aBuffer, WriteCallback onComplete )
{
	// write requested data to socket asynchronously
	boost::asio::async_write( 
		mSocket, 
		aBuffer,
		mStrand.wrap(
			boost::bind( 
				onComplete,
				boost::asio::placeholders::error,
				boost::asio::placeholders::bytes_transferred
			)
		)
	);
}

boost::asio::ip::tcp::socket& Connection::getSocket()
{
	return mSocket;
}


