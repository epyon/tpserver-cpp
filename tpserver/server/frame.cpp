/* tpserver frame base class.
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

#include "frame.h"

#include <sstream>
#include <istream>

#include "endians.h"

const char* Frame::data() const 
{ 
  return raw_data.data(); 
}

const char* Frame::body() const 
{ 
  return data() + HEADER_LENGTH; 
}

char* Frame::data() 
{ 
  return const_cast<char*>(data()); 
}

char* Frame::body() 
{ 
  return const_cast<char*>(body()); 
}

bool Frame::decodeHeader()
{
  std::istringstream str( raw_data, std::ios_base::binary | std::ios_base::in );

  // Extract version (TP03 or TP\0\4)
  char ver[4];
  str.get( ver, 4 );

  // Extract sequence, type and length
  str.get( (char*)&sequence, 4 );
  str.get( (char*)&type, 4 );
  str.get( (char*)&size, 4 );

  // Network alignment
  sequence = ntohl(sequence);
  type     = (FrameType)ntohl(type);
  size     = ntohl(size);

  // Magic check
  if ( ver[0] != 'T' || ver[1] != 'P' ) return false;
  
  // Version check
  if ( version == 3 )
  {
    if ( ver[2] != '0' || ver[3] != '3' ) return false;
  }
  else
  {
    version = (ProtocolVersion) ver[3];
  }

  return true;
}

