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

#include "protocol.h"

Connection::Connection( 
	boost::asio::io_service& aIOS, 
	boost::asio::io_service::strand& aStrand 
)
 : socket( aIOS ),
   strand( aStrand ),
   version( fv0_3 )
{
	// no code
}

void Connection::listen()
{
	// Create frame!
  frame_in.reset( new Frame( version ) );

	// start an asynchronous read until headers end
	boost::asio::async_read( socket,
		boost::asio::buffer( frame_in->data(), frame_in->getHeaderLength() ),
		boost::bind(
			&Connection::onReadHeader, 
			shared_from_this(),
			boost::asio::placeholders::error
		)
	);
}

void Connection::onReadHeader( const boost::system::error_code& error )
{
	// if not error, parse first line and headers
	if ( !error /* and decode header */ ) 
	{
		// TODO: change to read_some
		boost::asio::async_read( socket,
			boost::asio::buffer( frame_in->body(), frame_in->getDataLength() ),
			boost::bind(
				&Connection::onReadBody, 
				shared_from_this(),
				boost::asio::placeholders::error
			)
		);
	}
	else
	{
		// handle error
	}
}



void Connection::onReadBody( const boost::system::error_code& error )
{
	if ( !error )
	{
		// TODO: handle message mechanism
		listen();
	}
	else
	{
		// handle error
	}
}


void Connection::onWrite( const boost::system::error_code& error )
{
	// TODO: change to write_some
	// write requested data to socket asynchronously
	if ( !error )
	{
		frames_out.pop_front();
		if ( !frames_out.empty() )
		{
			boost::asio::async_write(
				socket,
				boost::asio::buffer( frames_out.front()->data(), frames_out.front()->getLength() ),
				boost::bind( 
					&Connection::onWrite, 
					shared_from_this(),
					boost::asio::placeholders::error
				)
			);
		}
	}
	else
	{
		// handle error
	}
}

boost::asio::ip::tcp::socket& Connection::getSocket()
{
	return socket;
}




