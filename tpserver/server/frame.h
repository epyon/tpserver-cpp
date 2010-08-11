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
#include <boost/shared_ptr.hpp>
#include <tpserver/server/protocol.h>
#include <string>

class Frame : public boost::noncopyable {
public:
	/// Typedef for boost shared pointer
	typedef boost::shared_ptr< Frame > Ptr;
	/// Header size in bytes
	enum { HEADER_LENGTH = 16 };
	/// Maximum body size in bytes
	enum { MAX_BODY_LENGTH = 1048576 - HEADER_LENGTH };

public:
    /**
     * Return the length of the header section
     */
	int getHeaderLength() const { return HEADER_LENGTH; };

    /**
     * Return the length of the data section
     */
	int getDataLength() const { return size; }

    /**
     * Return the frame length
     *
     * Equals to length of data section + length of header section
     */
	int getLength() const { return size + HEADER_LENGTH; }

	/**
	 * Return frame type
	 *
	 * See protocol description or protocol.h 
	 */ 
	FrameType getType() const { return type; }

	/**
	 * Returns the sequence number
	 */
	int getSequence() const { return sequence; }

	/**
	 * Returns the protocol version
	 */
	ProtocolVersion getVersion() const { return version; }

	/**
	 * Returns whether strings are padded
	 */
	bool isPadStrings() const { return pad_strings; }

  /**
   * Decode header information from raw data
   *
   * @returns true if succeeded false, otherwise
   */
  bool decodeHeader();

// Currently public, will be made protected probably

	/// Constructor
	explicit Frame( ProtocolVersion v ) 
		: version(v), type(ft_Invalid), 
		  sequence(0), size(0), pad_strings(false) {}

	/// Returns const char to all data
	const char* data() const;

	/// Returns const char to body
	const char* body() const;

	/// Returns char to all data
	char* data();

	/// Returns char to body
	char* body();



protected: // fields
	/// Version of protocol that this frame is encoded with
	ProtocolVersion version;
	/// Frame type
	FrameType type;
	/// Which packet sequence does this refer to?
	uint32_t sequence;
	/// Size of the frame in bytes
	uint32_t size;
	/// Actual data of the frame
	std::string raw_data;
	/// Whether to pad strings with \0 values
	bool pad_strings;
};

#endif // FRAME_H
