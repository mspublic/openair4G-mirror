/**
 **	\file sockets-config.h
 **	\date  2007-04-14
 **	\author grymse@alhem.net
**/
/*
Copyright (C) 2007-2010  Anders Hedstrom

This library is made available under the terms of the GNU GPL, with
the additional exemption that compiling, linking, and/or using OpenSSL 
is allowed.

If you would like to use this library in a closed-source application,
a separate license agreement is available. For information about 
the closed-source license agreement for the C++ sockets library,
please visit http://www.alhem.net/Sockets/license.html and/or
email license@alhem.net.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/
#ifndef _SOCKETS_CONFIG_H
#define _SOCKETS_CONFIG_H

/* Limits */
#define TCP_LINE_SIZE 8192
#define MAX_HTTP_HEADER_COUNT 200

#ifndef _RUN_DP
/* First undefine symbols if already defined. */
#undef HAVE_OPENSSL
#undef ENABLE_IPV6
#undef USE_SCTP
#undef NO_GETADDRINFO
#undef ENABLE_POOL
#undef ENABLE_SOCKS4
#undef ENABLE_RESOLVER
#undef ENABLE_RECONNECT
#undef ENABLE_DETACH
#undef ENABLE_EXCEPTIONS
#undef ENABLE_XML
#endif // _RUN_DP


/* OpenSSL support. */
//#define HAVE_OPENSSL


/* Ipv6 support. */
#define ENABLE_IPV6


/* SCTP support. */
//#define USE_SCTP


/* Define NO_GETADDRINFO if your operating system does not support
   the "getaddrinfo" and "getnameinfo" function calls. */
#define NO_GETADDRINFO


/* Connection pool support. */
//#define ENABLE_POOL


/* Socks4 client support. */
//#define ENABLE_SOCKS4


/* Asynchronous resolver. */
#define ENABLE_RESOLVER


/* Enable TCP reconnect on lost connection.
	Socket::OnReconnect
	Socket::OnDisconnect
*/
#define ENABLE_RECONNECT


/* Enable socket thread detach functionality. */
#define ENABLE_DETACH


/* Enabled exceptions. */
#define ENABLE_EXCEPTIONS


/* XML classes. */
//#define ENABLE_XML


/* Resolver uses the detach function so either enable both or disable both. */
#ifndef ENABLE_DETACH
#undef ENABLE_RESOLVER
#endif


#endif // _SOCKETS_CONFIG_H
