/*  Tp Connection object
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

#include "tpconnection.h"

#include "logging.h"

TpConnection::TpConnection( 
    boost::asio::io_service& aIOS, 
    boost::asio::io_service::strand& aStrand 
) : Connection( aIOS, aStrand ), status( PRECONNECTED )
{
  last_ping_time = time(NULL);
}

void TpConnection::handleFrame()
{
  switch( status ) 
  {
    case PRECONNECTED : handlePreConnectFrame(); break;
    case CONNECTED    : handlePreLoginFrame();   break;
    case LOGGEDIN     : handleInGameFrame();     break;
  }
}

void TpConnection::handlePreConnectFrame()
{
  switch( frame_in->getType() )
  {
    case ft02_Connect            : processConnectFrame();  break;
    case ft03_Features_Get       : processFeaturesFrame(); break;
    default : /*throw*/;                                   break;
  }
}

void TpConnection::handlePreLoginFrame()
{
  switch( frame_in->getType() )
  {
    case ft02_Login              : processLoginFrame();        break;
    case ft03_Features_Get       : processFeaturesFrame();     break;
    case ft03_Ping               : processPingFrame();         break;
    case ft02_Time_Remaining_Get : processTimeRemainingFrame();break;
    case ft03_Account            : processAccountFrame();      break;
    case ft04_GameInfo_Get       : processGameInfoFrame();     break;
    case ft04_Filters_Set        : processFiltersFrame();      break;
    default : /*throw*/;
  }
}

void TpConnection::handleInGameFrame()
{
  switch( frame_in->getType() )
  {
    case ft03_Features_Get       : processFeaturesFrame();     break;
    case ft03_Ping               : processPingFrame();         break;
    case ft02_Time_Remaining_Get : processTimeRemainingFrame();break;
    case ft04_GameInfo_Get       : processGameInfoFrame();     break;
    case ft04_Filters_Set        : processFiltersFrame();      break;
    default : handleAgentFrame(); break;
  }
}

void TpConnection::processConnectFrame()
{

}

void TpConnection::processLoginFrame()
{

}

void TpConnection::processFeaturesFrame()
{

}

void TpConnection::processPingFrame()
{
  LOG_DEBUG( "TpConnection : Processing Ping frame" );

  // check for the time of the last frame, ignore this if
  // less than 60 seconds ago.
  if ( last_ping_time < static_cast<uint64_t>( time(NULL) ) - 60 )
  {
    last_ping_time = time(NULL);
    sendOK(frame, "Keep alive ok, hope you're still there");
    LOG_DEBUG( "TpConnection : Ping completed" );
  }
  else
  {
    LOG_WARNING( "TpConnection : Client tried to ping within 60 seconds of the last ping" );
  }

  LOG_DEBUG("TpConnection : Processing Ping frame -- finished");
}

void TpConnection::processTimeRemainingFrame()
{

}

void TpConnection::processAccountFrame()
{

}

void TpConnection::processGameInfoFrame()
{

}

void TpConnection::processFiltersFrame()
{

}


