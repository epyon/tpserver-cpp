#ifndef TPCONNECTION_H
#define TPCONNECTION_H

/* tpserver tpprotocol connection base class.
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

#include <tpserver/server/connection.h>
#include <tpserver/server/system.h>

class TpConnection : public Connection
{
  public:
    /// Boost shared pointer
    typedef boost::shared_ptr< TpConnection > Ptr;

    /// Connection state
    enum Type {
      PLAYER,
      ADMIN
    };


    // Connection status
    enum Status {
      PRECONNECTED = 0, //!< Connection not established
      CONNECTED,        //!< Connection established but not ready
      LOGGEDIN          //!< Connection ready
    };
  public:

    /**
     * Constructor.
     *
     * @param aIOS Boost io_service on which the connection operates
     * @param aStrand the thread on which the connection operates
     */
    TpConnection( 
        boost::asio::io_service& aIOS, 
        boost::asio::io_service::strand& aStrand 
        );

    /** 
     * The working method -- processes the received message
     */
    virtual void handleFrame();

    /**
     * Process frames before connect frame.
     *
     * The server will answer a get features frame or a connect
     * frame.
     */
    virtual void handlePreConnectFrame();

    /**
     * Process frames before login frame but after connect.
     */
    virtual void handlePreLoginFrame();

    /**
     * Process in game (after login) frames.
     */
    virtual void handleInGameFrame();


    virtual void handleAgentFrame() = 0;

  private:
    void processConnectFrame();
    void processLoginFrame();
    void processFeaturesFrame();
    void processPingFrame(); 
    void processTimeRemainingFrame();
    void processAccountFrame();
    void processGameInfoFrame();
    void processFiltersFrame();

  public:
//    void sendFail(InputFrame::Ptr oldframe, FrameErrorCode code, const std::string& error, const RefList& reflist = RefList());
//    void sendFail(InputFrame::Ptr oldframe, const FrameException& fe);
//    void sendSequence(InputFrame::Ptr oldframe, size_t sequence_size );
//    void send(InputFrame::Ptr oldframe, const Packable* packable );
//    void send(InputFrame::Ptr oldframe, const Packable::Ptr packable );
//    void sendOK(InputFrame::Ptr oldframe, const std::string& message );
//    void sendModList(InputFrame::Ptr oldframe, FrameType ft, uint32_t sequence, const IdModList& modlist,
//        uint32_t count, uint32_t start, uint64_t fromtime );

  protected:
    OutputFrame::Ptr createFrame( InputFrame::Ptr oldframe );
    void sendFrame( OutputFrame::Ptr frame );

    /// Connection status
    Status status;

    /// Last ping time
    uint64_t last_ping_time;

}; // class TpConnection

#endif // TPCONNECTION_H
