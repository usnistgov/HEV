/*
 * The DIVERSE Toolkit
 * Copyright (C) 2000 - 2003  Virginia Tech
 * 
 * This software, the DIVERSE Toolkit library, is free software; you can
 * redistribute it and/or modify it under the terms of the GNU Lesser
 * General Public License (LGPL) as published by the Free Software
 * Foundation; either version 2.1 of the License, or (at your option) any
 * later version.
 * 
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this software, in the top level source directory in
 * a file named "COPYING.LGPL"; if not, see it at:
 * http://www.gnu.org/copyleft/lesser.html or write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA.
 *
 */
/****** serverProtocol.h ******/
/*********************************************************************
 * This file defines the client to dtk-server and dtk-server to remote
 * dtk-server communication protocol.  This is currently implemented
 * on top of TCP and UPD sockets in the dtk-server and dtkClient,
 * and dtkSharedMem classes in the DTK library, libdtk.
 *********************************************************************/


/*********************************************************************
 ****** Command Requests you can send to the dtk-server **************
 *********************************************************************/


// senddata refers to data sent on the socket or other socket like
// thingy.  All commands are characters or '\0' (null) terminated
// character strings.  All the senddata is put together in one long
// character string which has one or more '\0' characters in it.
// "hi" is the same as 'h','i','\0'.
// Most command requests are responded with a _DTKRESP_FAILED,
// _DTKRESP_SUCCESS, or _DTKRESP_BADREQUEST; unless otherwise stated.


#define _DTKREQ_WRITE               'W'
// senddata={'W',byte_order,"filename","size",data}
// There will be no reply from the server to the client.
// byte_order can be _DTK_NONNET_BYTEORDER or
// _DTK_NET_BYTEORDER

#define _DTKREQ_LOADSERVICE         'L'
// senddata = {'L',"filename","serviceName","arg"}
// or senddata = {'L',"filename","",""}
// to be the case with default name and no arg.

#define _DTKREQ_LOADCONFIG          'l'
// senddata = {'l',"configname","path"}
// example in chars: lConfig\0
// put another way: senddata = {'F',"Config"}

#define _DTKREQ_UNLOADSERVICE       'U'
// example in chars: UName\0
// put another way: senddata={'U',"Name"}

#define _DTKREQ_RESETSERVICE        'R'
// example in chars: RName\0
// put another way: senddata={'R',"Name"}

#define _DTKREQ_CHECKSERVICE        'C'
// example in chars: CName\0
// put another way: senddata={'C',"Name"}

#define _DTKREQ_SHUTDOWN            'X'
// just send the single char 'X'
// Server will respond with 'S' if
// it worked.

#define _DTKREQ_CONNECTSERVERS      'v'
// client to server request
// This must be done before any other server to server
// related requests.
// This is a client request to have two DTK server make TCP
// connections.
// senddata={'v',"remoteServerAddressPort","clientRequestFrom"}
// responds to client later if it's not connected yet.

#define _DTKSREQ_CONNECTSERVERS     'V'
// Server to Server request
// senddata={'V',"remoteBindPort","clientRequestFrom"}
// "remoteMasterPort" is the remote servers binded port number.
// The address will be gotten from the connected socket.
// This will respond with

#define _DTKSSREQ_CONNECTSERVERS         'r'
// Server to Server request
// senddata={'r',"remoteBindPort","clientRequestFrom","errMsg"}
// where "errMsg" will be "" on success or not on failure.
// "remoteMasterPort" is the remote servers binded port number.
// The address will be gotten from the connected socket.
// This will respond with

#define _DTKREQ_CONNECTREMOTESEG          'c'
// senddata={'c',"localName","remoteAddr:Port","remoteName"}
// The server will send a _DTKSREQ_CONNECTSEG to the remoteAddr:Port
// server.

#define _DTKSREQ_CONNECTREMOTESEG         's'
// This is a server to remote server request for making remote
// shared memory.
// senddata={'s',"localName","remoteName","sizeBase10","clientRequestFrom"}
// "clientRequestFrom" is sent to and from in order to know where to
// put the responce status back to.
// "remoteName" is the shared memory file name on the remote server.
// the response be a _DTKSSREQ_CONNECTSEG sent to the other server.

#define _DTKSSREQ_CONNECTREMOTESEG        'O'
// This is a server to server request that
// is a response from _DTKSREQ_CONNECTSEG
// senddata={'s',"localName","remoteName",
//           "clientRequestFrom","errorString"}
// This is the final in the _DTK*REQ_CONNECTSEG series of
// command requests.  All this stuff is needed to be sent back because
// the servers do not keep a record of commands that are not fully
// processed.

#define _DTKREQ_SEGWRITELIST        'w'
// This is a server to server or client to server request
// senddata={'w',"filename"}
// responds with _DTKRESP_SEGWRITELIST see below...

/*********************************************************************
 ************** Responses the dtk-server can respond with ************
 *********************************************************************/


#define _DTKRESP_SEGWRITELIST        'x'
// responds with={'x',WRITETYPE,"filename",
// "addressPort","name","addressPort","name"...,""}
// The sequence is terminated with "".
// WRITETYPE is _DTK_WRITE_TCP or _DTK_WRITE_UDP.


#define _DTKRESP_FAILED          'F'

#define _DTKRESP_SUCCESS         'S'

#define _DTKRESP_BADREQUEST      'B'
// bad request format. Server parsing failed.

// In addition other data may be sent after the _DTKRESP_* charactor,
// which depends on what request was sent.



/******************* byte order for writing shared memory ************/

#define _DTK_NET_BYTEORDER        'X'
#define _DTK_NONNET_BYTEORDER     'Y'


/****************** write type ***************************************/

#define _DTK_WRITE_TCP            'T'
#define _DTK_WRITE_UDP            'U'



/*********************************************************************/
