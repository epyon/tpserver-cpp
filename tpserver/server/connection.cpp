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
#include "logging.h"

Connection::Connection( 
	boost::asio::io_service& aIOS, 
	boost::asio::io_service::strand& aStrand 
)
 : socket( aIOS ),
   strand( aStrand ),
   version( fv0_4 ),
   pad_spaces( false )
{
  LOG_DEBUG("Connection created");
	// no code
}

void Connection::listen()
{
	// Create frame!
  frame_in.reset( new InputFrame( version ) );

  LOG_DEBUG("Connection : reading header (%d bytes)", frame_in->getHeaderLength() );

	// start an asynchronous read until headers end
	boost::asio::async_read( socket,
		boost::asio::buffer( frame_in->getHeaderBuffer(), frame_in->getHeaderLength() ),
		boost::bind(
			&Connection::onReadHeader, 
			shared_from_this(),
			boost::asio::placeholders::error
		)
	);
}

void Connection::send()
{
  LOG_DEBUG("Connection::send()");
	// TODO: change to write_some
	// write requested data to socket asynchronously
  if ( !frames_out.empty() )
  {
    boost::asio::async_write(
      socket,
      boost::asio::buffer( frames_out.front()->getPacket() ),
      boost::bind( 
        &Connection::onWrite, 
        shared_from_this(),
        boost::asio::placeholders::error
      )
    );
  }
}

void Connection::onReadHeader( const boost::system::error_code& error )
{
  LOG_DEBUG("Connection::onReadHeader()");
	// if not error, parse first line and headers
	if ( !error && frame_in->decodeHeader() ) 
	{
		// TODO: change to read_some
		boost::asio::async_read( socket,
			boost::asio::buffer( frame_in->getBodyBuffer(), frame_in->getDataLength() ),
			boost::bind(
				&Connection::onReadBody, 
				shared_from_this(),
				boost::asio::placeholders::error
			)
		);
	}
	else
	{
    if (error) 
    {
      LOG_ERROR("Connection::onReadHeader failed -- %s!", error.message().c_str() );
		  // handle error
    }
    else
    {
      LOG_ERROR("Connection::onReadHeader failed -- header not decoded properly!");
		  // handle error
    }
	}
}



void Connection::onReadBody( const boost::system::error_code& error )
{
  LOG_DEBUG("Connection::onReadBody()");
	if ( !error )
	{
		handleFrame();
		listen();
	}
	else
	{
    LOG_ERROR("Connection::onReadBody failed -- %s!", error.message().c_str() );
		// handle error
	}
}

void Connection::onWrite( const boost::system::error_code& error )
{
  LOG_DEBUG("Connection::onWrite()");
	// TODO: change to write_some
	// write requested data to socket asynchronously
	if ( !error )
	{
    // remove sent frame
		frames_out.pop_front();

    // try sending more
    send();
	}
	else
	{
    LOG_ERROR("Connection::onWrite failed -- %s!", error.message().c_str() );
		// handle error
	}
}

boost::asio::ip::tcp::socket& Connection::getSocket()
{
	return socket;
}

Connection::~Connection()
{
  LOG_DEBUG("Connection destroyed");
}


