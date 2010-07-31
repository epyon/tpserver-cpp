#ifndef SERVER_H
#define SERVER_H

/*  tpserver main server singleton class
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

#include <tpserver/acceptor.h> 

class Server : private boost::noncopyable
{
  public:
    /** 
     * Constructor
     */
    explicit Server();

    /**
     * Start admin connection
     */
    void startAdmin();

    /**
     * Stop admin connection
     */
    void stopAdmin();

    /**
     * Stop server
     *
     * Stops the asio loop and terminates the server
     */
    void stop();

    /**
     * Run server
     *
     * Enters the asio loop. Doesn't return unless stop is called,
     * or error happens.
     */
    void run();

    /**
     * Destructor
     */
    ~Server();
  private:
    /// Accepts an admin connection
    void acceptAdmin( const boost::asio::ip::tcp::socket& socket );

    /// Accepts tp protocol connection
    void acceptTp( const boost::asio::ip::tcp::socket& socket );

    /// Accepts http protocol connection
    void acceptHttp( const boost::asio::ip::tcp::socket& socket );

    /// Load PluginManager modules
    void loadModules();

    /// Main TP protocol listen port
    Acceptor::Ptr mMainPort;

    /// TP over SSL protocol listen port
    Acceptor::Ptr mSSLPort;

    /// TP over HTTP protocol listen port
    Acceptor::Ptr mHTTPPort;

    /// TP over HTTPS protocol listen port
    Acceptor::Ptr mHTTPSPort;

    /// TP admin protocol listen port
    Acceptor::Ptr mAdminPort;

    /// The asio object (currently just one)
    boost::asio::io_service mIOS;
};

#endif // SERVER_H
