#ifndef PLAYERCONNECTION_H
#define PLAYERCONNECTION_H
/*  Player Connection class
 *
 *  Copyright (C) 2004-2005, 2007, 2008  Lee Begg and the Thousand Parsec Project
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

#include <stdint.h>
#include <tpserver/frame.h>
#include <tpserver/tcpconnection.h>

class PlayerAgent;
class PlayerConnection: public TcpConnection {
  public:
    virtual ~PlayerConnection();

    void process();
    virtual void sendFrame(Frame * frame) = 0;

    Frame* createFrame(Frame* oldframe = NULL);

  protected:
    PlayerConnection(int fd);

    virtual void verCheck() = 0;
    virtual bool readFrame(Frame * recvframe) = 0;

    void processGetFeaturesFrame(Frame* frame);

    void sendFail(Frame* oldframe, FrameErrorCode code, const std::string& error );

    ProtocolVersion version;
  private:
    /// Blocked to disallow non-fd creation
    PlayerConnection() {}
    
  private:
    void processGetGameInfoFrame(Frame* frame);
    void processSetFilters(Frame* frame);
    void processTimeRemainingFrame(Frame* frame);

    void inGameFrame();
    void login();
    
    PlayerAgent *playeragent;
    uint64_t lastpingtime;
    bool paddingfilter;
};

#endif
