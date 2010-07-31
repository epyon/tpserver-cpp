#ifndef ACCEPTOR_H
#define ACCEPTOR_H

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

#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/asio.hpp>

class Acceptor {
public:
    /// Callback for socket acception
    typedef boost::function< void ( const boost::asio::ip::tcp::socket& ) > Callback;

    /// Shared pointer typedef
    typedef boost::shared_ptr< Acceptor > Ptr;

    /**
     * Acceptor constructor.
     *
     * @param aIOS IOService object on which to register
     * @param aAddress TCP address to listen on
     * @param aPort port on which to listen
     */
    explicit Acceptor( boost::asio::io_service& aIOS, const std::string& aAddress, const std::string& aPort, Callback aCallback );

  private:
    /// Accept handler
    void accept( const boost::system::error_code& aError );

    /// ASIO's acceptor to listen for incoming connections
    boost::asio::ip::tcp::acceptor mAcceptor;

    /// Current socket
    boost::asio::ip::tcp::socket mSocket;

    /// Stored callback
    Callback mCallback;

};

#endif // ACCEPTOR_H

