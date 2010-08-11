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
    //sendOK(frame, "Keep alive ok, hope you're still there");
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

OutputFrame::Ptr TpConnection::createFrame(InputFrame::Ptr oldframe)
{
  OutputFrame::Ptr newframe;
  if ( oldframe != NULL ) 
  {
    newframe.reset( new OutputFrame( oldframe->getVersion(),pad_spaces ) );
    newframe->setSequence( oldframe->getSequence() );
  } 
  else 
  {
    newframe.reset( new OutputFrame( version,pad_spaces ) );
  }
  return newframe;
}

void TpConnection::sendFrame( OutputFrame::Ptr frame )
{
  if ( version != frame->getVersion() ) 
  {
    LOG_WARNING("TpConnection : Version mis-match, packet %d, connection %d", frame->getVersion(), version );
  }

  frames_out.push_back( frame );
  send();
}


void TpConnection::sendFail(InputFrame::Ptr oldframe, FrameErrorCode code, const std::string& error, const RefList& reflist ) 
{
  OutputFrame::Ptr frame = createFrame( oldframe );
  frame->setType( ft02_Fail );
  frame->packInt( code );
  frame->packString( error );
  if ( frame->getVersion() >= fv0_4 ){
    frame->packInt(reflist.size());
    for (RefList::const_iterator ref = reflist.begin(); ref != reflist.end(); ++ref){
      frame->packInt(ref->first);
      frame->packInt(ref->second);
    }
  }
  sendFrame(frame);
}

void TpConnection::sendFail(InputFrame::Ptr oldframe, const FrameException& fe)
{
    sendFail(oldframe, fe.getErrorCode(), fe.getErrorMessage(), fe.getRefList());
}

void TpConnection::sendSequence(InputFrame::Ptr oldframe, size_t sequence_size )
{
  OutputFrame::Ptr frame = createFrame(oldframe);
  frame->setType( ft02_Sequence );
  frame->packInt( sequence_size );
  sendFrame(frame);
}

void TpConnection::sendOK(InputFrame::Ptr oldframe, const std::string& message )
{
  OutputFrame::Ptr frame = createFrame(oldframe);
  frame->setType( ft02_OK );
  frame->packString( message );
  sendFrame(frame);
}

void TpConnection::sendModList(InputFrame::Ptr oldframe, FrameType ft, uint32_t sequence, const IdModList& modlist, uint32_t count, uint32_t start, uint64_t fromtime ) 
{
  if ( start > modlist.size() ) 
  {
    LOG_DEBUG("Starting number too high, snum = %d, size = %d", start, modlist.size());
    sendFail(oldframe,fec_NonExistant, "Starting number too high");
    return;
  }
  if  (count > modlist.size() - start ) 
  {
    count = modlist.size() - start;
  }

  if ( count > MAX_ID_LIST_SIZE + ((oldframe->getVersion() < fv0_4)? 1 : 0)) 
  {
    LOG_DEBUG("Number of items to get too high, numtoget = %d", count);
    sendFail(oldframe,fec_FrameError, "Too many items to get, frame too big");
    return;
  }
  OutputFrame::Ptr frame = createFrame(oldframe);
  frame->setType(ft);
  frame->packInt(sequence);
  frame->packIdModList(modlist,count,start);
  if (frame->getVersion() >= fv0_4) 
  {
    frame->packInt64(fromtime);
  }
  sendFrame(frame);
}

