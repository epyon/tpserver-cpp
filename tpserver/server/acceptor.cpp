/*  tpserver connection acceptor class
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

#include "acceptor.h"

#include <boost/bind.hpp>

Acceptor::Acceptor( boost::asio::io_service& aIOS, const std::string& aAddress, const std::string& aPort, Creator aCreator )
  : mAcceptor( aIOS ),
    mCreator( aCreator )
{
    // create a resolver, and resolve the address
    boost::asio::ip::tcp::resolver iResolver( aIOS );
    boost::asio::ip::tcp::resolver::query iQuery( aAddress, aPort );
    boost::asio::ip::tcp::endpoint iResult = *iResolver.resolve( iQuery );

    // prepare the acceptor
    mAcceptor.open( iResult.protocol() );
    mAcceptor.set_option( boost::asio::ip::tcp::acceptor::reuse_address(true) );
    mAcceptor.bind( iResult );

    // start listening
    mAcceptor.listen();

    // create first connection
    mConnection = mCreator();

    // asynchronously accept on our first worker, if succeeded, run accept
    mAcceptor.async_accept( 
        mConnection->getSocket(), 
        boost::bind( &Acceptor::accept, this, boost::asio::placeholders::error ) 
        );
    // note that this wont happen yet, because there's no io_service running
}

/// Accept handler
void Acceptor::accept( const boost::system::error_code& aError )
{
    // if the accept had an error, forget it
    if ( aError ) return;

    // process the socket
    mConnection->listen();

    // forget the connection, create a new one
    mConnection = mCreator();

    // ... and asynchronously wait for an accept again
    mAcceptor.async_accept( 
        mConnection->getSocket(), 
        boost::bind( &Acceptor::accept, this, boost::asio::placeholders::error ) 
        );

}
