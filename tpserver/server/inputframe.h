#ifndef INPUTFRAME_H
#define INPUTFRAME_H
/*  TP protocol InputFrame class
 *
 *  Copyright (C) 2009,2010 Kornel Kisielewicz and the Thousand Parsec Project
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

#include <tpserver/server/frame.h>
#include <tpserver/server/system.h>
#include <tpserver/server/common.h>
#include <boost/shared_ptr.hpp>

class InputFrame : public Frame {
  public:
    typedef boost::shared_ptr< InputFrame > Ptr;
  
  public:
    /**
     * Standard constructor
     *
     * Creates non-padded version 3 frames by default
     */
    explicit InputFrame(ProtocolVersion v = fv0_3, bool padding = false);
    
    /**
     * Checks if there's enough data available in the buffer
     */
    bool isEnoughRemaining(uint32_t size) const;
    
    /// Advances the frame position by amount bytes
    void advance(uint32_t amount);

    int unpackInt();
    std::string unpackString();
    int64_t unpackInt64();
    char unpackInt8();
    IdMap unpackMap();
    IdSet unpackIdSet();

  private:
    /// Current unpack position
    uint32_t unpackptr;
};

#endif
