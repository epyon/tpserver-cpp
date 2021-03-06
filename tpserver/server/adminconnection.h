#ifndef ADMINCONNECTION_H
#define ADMINCONNECTION_H

/* tpserver admin connection base class.
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

#include <tpserver/server/tpconnection.h>

class AdminConnection : public TpConnection
{
public:
	/// Boost shared pointer
	typedef boost::shared_ptr< AdminConnection > Ptr;

public:

    /**
     * Constructor.
     *
     * @param aIOS Boost io_service on which the connection operates
     * @param aStrand the thread on which the connection operates
     */
    AdminConnection( 
        boost::asio::io_service& aIOS, 
        boost::asio::io_service::strand& aStrand 
        );

    virtual void handleAgentFrame();

}; // class AdminConnection

#endif // ADMINCONNECTION_H
