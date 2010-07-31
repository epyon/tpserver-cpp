#ifndef CONNECTION_H
#define CONNECTION_H

/* tpserver abstract connection base class.
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

#include <boost/function.hpp>
#include <boost/asio.hpp>
#include <boost/noncopyable.hpp>

class Connection : 
  private boost::noncopyable
{
  public:
    /// Callback for return with boost error code
    typedef boost::function< void (const boost::system::error_code& aError) > Callback;

    /// Write Callback for return with boost error code and amount of data processed
    typedef boost::function< void (const boost::system::error_code& aError, std::size_t aAmount ) > WriteCallback;

  public:

    /**
     * Constructor.
     *
     * @param aIOS Boost io_service on which the connection operates
     * @param aStrand the thread on which the connection operates
     */
    Connection( 
        boost::asio::io_service& aIOS, 
        boost::asio::io_service::strand& aStrand 
        );

    /**
     * Asynchronously write an asio buffer to the connection.
     *
     * @param onComplete Callback called on completing the operation, or error.
     */
    void write( boost::asio::const_buffers_1 aBuffer, WriteCallback onComplete );

    /**
     * Asynchronously read to an asio buffer on the connection.
     *
     * @param onComplete Callback called on completing the operation, or error.
     */
    void read( boost::asio::mutable_buffers_1 aBuffer, WriteCallback onComplete );

    /**
     * Returns the socket associated with this connection.
     *
     * @returns TCP/IP socket reference
     */
    boost::asio::ip::tcp::socket& getSocket();

  protected:

    /// Connection socket
    boost::asio::ip::tcp::socket mSocket;

    /// Assures non-concurrency of connection handling
    boost::asio::io_service::strand& mStrand;

    /// Buffer for asynchronous read of requests
    boost::asio::streambuf mStreamBuffer;

}; // class Connection

#endif // CONNECTION_H
