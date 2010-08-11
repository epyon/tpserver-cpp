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

#include <deque>

#include <boost/asio.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

#include <tpserver/server/inputframe.h>
#include <tpserver/server/outputframe.h>

class Connection : 
  public boost::noncopyable,
  public boost::enable_shared_from_this< Connection >
{
public:
	/// Boost shared pointer
	typedef boost::shared_ptr< Connection > Ptr;

  /// Connection type
  enum Type {
    PLAYER,
    ADMIN
  };
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
	 * Start reading on connection
	 *
	 * Once enough data is read, onReadHeader is called.
	 */
	void listen();

    /**
     * Returns the socket associated with this connection.
     *
     * @returns TCP/IP socket reference
     */
    boost::asio::ip::tcp::socket& getSocket();

  protected: // methods

    /**
     * Gets called when enough data is read on the connection to decode a header
     */
	void onReadHeader( const boost::system::error_code& error );

    /**
     * Asynchronously reads the rest of the frame on the connection.
     */
    void onReadBody( const boost::system::error_code& error );

	  /**
     * Asynchronously write an asio buffer to the connection.
     */
    void onWrite( const boost::system::error_code& error );

    /** 
     * The working method -- processes the received message
     */
    virtual void handleFrame() = 0;

protected: // fields

    /// Connection socket
    boost::asio::ip::tcp::socket socket;

    /// Assures non-concurrency of connection handling
    boost::asio::io_service::strand& strand;

	/// Frame being read
	OutputFrame::Ptr frame_in;

	/// Frames to be written
	std::deque< InputFrame::Ptr > frames_out;

  /// Connection protocol version
	ProtocolVersion version;

}; // class Connection

#endif // CONNECTION_H
