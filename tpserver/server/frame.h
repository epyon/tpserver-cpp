#ifndef FRAME_H
#define FRAME_H

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

#include <boost/noncopyable.hpp>
#include <tpserver/protocol.h>

class Frame : public boost::noncopyable {
public:
protected:
	/// Version of protocol that this frame is encoded with
	ProtocolVersion version;
	/// Frame type
	FrameType type;
	/// Type version
	uint32_t type_version;
	/// Which packet sequence does this refer to?
	uint32_t sequence;
	/// Size of the frame in bytes
	uint32_t size;
	/// Actual data of the frame
	std::string data;
	/// Whether to pad strings with \0 values
	bool pad_strings;

};

#endif // FRAME_H
