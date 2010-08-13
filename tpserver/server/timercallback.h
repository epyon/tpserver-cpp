/*  TimerCallback class/template
 *
 *  Copyright (C) 2007  Lee Begg and the Thousand Parsec Project
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
#ifndef TIMERCALLBACK_H
#define TIMERCALLBACK_H

#include <boost/asio/deadline_timer.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/function.hpp>
#include <boost/bind.hpp>

class TimerCallback 
  : public boost::enable_shared_from_this< TimerCallback >
{
  public:
    /// typedef for callback function
    typedef boost::function< void() > Callback;
    /// typedef for shared pointer
    typedef boost::shared_ptr< TimerCallback > Ptr;
    /// typedef for implementation
    typedef boost::scoped_ptr< boost::asio::deadline_timer > Timer;

    TimerCallback (Callback acallback, uint32_t sec) : callback(acallback) {
      expiretime = sec + time(NULL);
      seconds    = sec;
    }
    
    TimerCallback(): callback(NULL), expiretime(0), seconds(0) {}
    
    uint64_t getExpireTime() const {
      return expiretime;
    }
    
    bool isValid() const {
      return (callback != NULL);
    }

    void fire( boost::asio::io_service& io ) {
      timer.reset( new boost::asio::deadline_timer( io, boost::posix_time::seconds( seconds ) ) );
      timer->async_wait( boost::bind( &TimerCallback::call, shared_from_this(), _1 ) );
    }
    
    void invalidate() {
      callback = NULL;
      timer->cancel();
    }
    
    void call( const boost::system::error_code& ) const {
      if (callback != NULL) 
      {
        // note that this check is made, because
        // call will be ran even on cancel!
        callback();
      }
    }
  
  private:
    Callback callback;
    uint64_t expiretime;
    uint32_t seconds;
    Timer timer;
};

#endif
