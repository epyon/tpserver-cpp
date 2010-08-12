/*  InputFrame class, the network packets for the TP procotol
 *
 *  Copyright (C) 2009  Kornel Kisielewicz and the Thousand Parsec Project
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

#include <string>
#include <cstdlib>
#include <string.h>
#include <sstream>

#include "inputframe.h"

#include "endians.h"
#include "logging.h"
#include "frameexception.h"

InputFrame::InputFrame( ProtocolVersion v, bool padding )
  : Frame(v), unpackptr(0), size(0), body_buffer(NULL)
{
  pad_strings = padding;
}

bool InputFrame::isEnoughRemaining(uint32_t rsize) const{
  LOG_DEBUG("isEnoughRemaining, checking for %d, have %d", rsize, size - unpackptr);
  return (size - unpackptr) >= rsize;
}

void InputFrame::advance( uint32_t amount )
{
  uint32_t newoffset = unpackptr + amount;
  if (newoffset < size - 4)
    unpackptr = newoffset;
  //else throw
}

int InputFrame::unpackInt()
{
  int nval;
  memcpy(&nval, body_buffer + unpackptr, 4);
  unpackptr += 4;
  return ntohl(nval);
}


std::string InputFrame::unpackString(){
  uint32_t len = unpackInt();
  if ( unpackptr + len > size ) 
  {
    throw FrameException( fec_FrameError, "Not enough raw_data.to unpack string!" );
  }

  const char* pos = body_buffer + unpackptr;
  unpackptr += len;

  if ( pad_strings ) 
  {
    int pad = unpackptr % 4;
    if ( pad != 0 ) 
    {
      if ( unpackptr + (4 - pad) > size ) 
      {
        throw FrameException( fec_FrameError, "Not enough raw_data.to unpack string padding!" );
      }
      unpackptr += 4-pad;
    }
  }

  return std::string(pos, len);
}

int64_t InputFrame::unpackInt64()
{
  int64_t nval;
  memcpy(&nval, body_buffer + unpackptr, 8);
  unpackptr += 8;
  return ntohq(nval);
}

char InputFrame::unpackInt8(){
  char rval = body_buffer[unpackptr];
  unpackptr += 1;
  if ( pad_strings )
    unpackptr += 3;
  return rval;
}

IdMap InputFrame::unpackMap(){
  IdMap map;
  uint32_t msize = unpackInt();
  for ( uint32_t i = 0; i < msize; ++i ) 
  {
    uint32_t idx = unpackInt();
    map[idx] = unpackInt();
  }
  return map;
}

IdSet InputFrame::unpackIdSet(){
  IdSet set;
  uint32_t msize = unpackInt();
  for( uint32_t i = 0; i < msize; ++i ) 
  {
    set.insert( unpackInt() );
  }
  return set;
}

char* InputFrame::getBodyBuffer()
{
  if (body_buffer != NULL) return body_buffer;
  if (size == 0) return NULL;
  body_buffer = new char[size];
  return body_buffer;
}


bool InputFrame::decodeHeader()
{
  // Extract version (TP03 or TP\4\0)
  char ver[4];
  memcpy(ver, header_buffer, 4);

  // Extract sequence, type and length
  memcpy(&sequence, header_buffer + 4,  4);
  memcpy(&type,     header_buffer + 8,  4);
  memcpy(&size,     header_buffer + 12, 4);

  // Network alignment
  sequence = ntohl(sequence);
  type     = (FrameType)ntohl(type);
  size     = ntohl(size);

  // Magic check
  if ( ver[0] != 'T' || ver[1] != 'P' )
  {
    LOG_DEBUG( "InputFrame::decodeHeader() -- message not decoded, failed to decode 'TP'" );
    return false;
  }
  
  // Version check
  if ( version == 3 )
  {
    if ( ver[2] != '0' || ver[3] != '3' ) return false;
  }
  else
  {
    version = (ProtocolVersion) ver[4];
  }

  if ( size > 1024*1024 ) return false;

  LOG_DEBUG("InputFrame::decodeHeader() -- decoded header : ver[%s|%d|%d|%db]", ver, sequence, type, size );
  return true;
}

InputFrame::~InputFrame()
{
  if (body_buffer) delete[] body_buffer;
}
