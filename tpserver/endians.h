#ifndef ENDIAN_H
#define ENDIAN_H
/*  Endian functions for the TP procotol
 *
 *  Copyright (C) 2010  Kornel Kisielewicz and the Thousand Parsec Project
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

#include <netinet/in.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef htonll
# define htonq		htonll
# define ntohq		ntohll
#endif

#ifndef htonq
#ifndef WORDS_BIGENDIAN
# define htonq(i)	( ((int64_t)(htonl((i) & 0xffffffff)) << 32) | htonl(((i) >> 32) & 0xffffffff ) )
#else
#define htonq(i)        i
#endif 
# define ntohq		htonq
#endif

#endif // ENDIAN_H

