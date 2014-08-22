/* Fri Aug 22 07:41:22 +0200 2014:
   This file was generated by mill from gen/stdmill.mh */

/*
    Copyright (c) 2014 Martin Sustrik  All rights reserved.

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"),
    to deal in the Software without restriction, including without limitation
    the rights to use, copy, modify, merge, publish, distribute, sublicense,
    and/or sell copies of the Software, and to permit persons to whom
    the Software is furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included
    in all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
    THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
    IN THE SOFTWARE.
*/

#ifndef stdmill_h_included
#define stdmill_h_included

#include <stdio.h>
#include <stddef.h>
#include <assert.h>
#include <stdlib.h>
#include <uv.h>

/******************************************************************************/
/*  ABI versioning support.                                                   */
/******************************************************************************/

/*  Don't change this unless you know exactly what you're doing and have      */
/*  read and understand the following documents:                              */
/*  www.gnu.org/software/libtool/manual/html_node/Libtool-versioning.html     */
/*  www.gnu.org/software/libtool/manual/html_node/Updating-version-info.html  */

/*  The current interface version. */
#define MILL_VERSION_CURRENT 0

/*  The latest revision of the current interface. */
#define MILL_VERSION_REVISION 0

/*  How many past interface versions are still supported. */
#define MILL_VERSION_AGE 0

/******************************************************************************/
/* Tracing support.                                                           */
/******************************************************************************/

void _mill_trace ();

/******************************************************************************/
/*  Coroutine metadata.                                                       */
/******************************************************************************/

/* Main body of the coroutine. Handles all incoming events.
   'cfptr' points to the coframe of the coroutine that receives the event.
   If the initial part of the coroutine is run 'event' is set to NULL.
   Afterwards, 'event' either points to the child coframe that have just
   finished execution or is NULL in case the parent have canceled this
   coroutine. When the last part of the coroutine is run 'event' is set to the
   same value as 'cfptr' in case the coroutine was selected by the parent
   coroutine or to NULL in case it was canceled by the parent coroutine.
   The handler returns 0 if the event was successfully processed,
   -1 otherwise. */
typedef int (*mill_fn_handler) (void *cfptr, void *event);

/* Structure containing all the coroutine metadata. */
struct mill_type {
    mill_fn_handler handler;
    const char *name;
};

/******************************************************************************/
/* Coframe head is common to all coroutines.                                  */
/******************************************************************************/

/* This structure is placed at the beginning of each coframe. */
struct mill_cfh {

    /* Coroutine metadata. */
    const struct mill_type *type;

    /* Coroutine's "program counter". Specifies which point in the coroutine
       is currently being executed. The values are specific to individual
       coroutines. */
    int pc;

    /* Once the coroutine finishes, the coframe is put into the loop's event
       queue and later on, if the event can't be processed immediately, into
       the parent coroutine's pending queue. This member implements the
       single-linked list that forms the queue. */
    struct mill_cfh *nextev;

    /* The queue of pending events that can't be processed at the moment. */
    struct mill_cfh *pfirst;
    struct mill_cfh *plast;

    /* Parent corouine. */
    struct mill_cfh *parent;

    /* List of child coroutines. */
    struct mill_cfh *children;

    /* Double-linked list of sibling coroutines. */
    struct mill_cfh *next;
    struct mill_cfh *prev;

    /* Event loop that executes this coroutine. */
    struct mill_loop *loop;
};

/******************************************************************************/
/* The event loop.                                                            */
/******************************************************************************/

struct mill_loop
{
    /* Underlying libuv loop. */
    uv_loop_t uv_loop;

    /* Libuv hook that processes the mill events. */
    uv_idle_t idle;

    /* Local event queue. Items in the list are processed using
       the hook above. */
    struct mill_cfh *first;
    struct mill_cfh *last;
};

/* Initialise the loop object. */
void mill_loop_init (struct mill_loop *self);

/* Deallocate resources used by the loop. This function can be called only
   after mill_loop_run exits. */
void mill_loop_term (struct mill_loop *self);

/* Start the loop. The loop runs until the top-level coroutine exits. */
void mill_loop_run (struct mill_loop *self);

/* Enqueue the event to the loop's event queue. */
void mill_loop_emit (struct mill_loop *self, struct mill_cfh *ev);

/******************************************************************************/
/*  Helpers used to implement mill keywords.                                  */
/******************************************************************************/

#define mill_typeof(expr)\
    ((expr) ? (((struct mill_cfh*) expr)->type) : 0)

void mill_coframe_init (
    void *cfptr,
    const struct mill_type *type,
    void *parent,
    struct mill_loop *loop);
void mill_coframe_term (
    void *cfptr,
    int canceled);
void mill_emit (
    void *cfptr);
void mill_cancel_children (
    void *cfptr);
int mill_has_children (
    void *cfptr);

/******************************************************************************/
/*  Standard library of elementary coroutines.                                */
/******************************************************************************/

extern const struct mill_type mill_type_msleep;

void mill_go_msleep (
    const struct mill_type *type,
    struct mill_loop *loop,
    void *parent,
    int milliseconds);

void msleep (
    int milliseconds);

extern const struct mill_type mill_type_getaddressinfo;

void mill_go_getaddressinfo (
    const struct mill_type *type,
    struct mill_loop *loop,
    void *parent,
    int *rc,
    const char * node,
    const char * service,
    const struct addrinfo * hints,
    struct addrinfo * *res);

void getaddressinfo (
    int *rc, 
    const char * node, 
    const char * service, 
    const struct addrinfo * hints, 
    struct addrinfo * *res);

void freeaddressinfo (
    struct addrinfo *ai);

struct tcpsocket {
    uv_tcp_t s;
    uv_loop_t *loop;
    int pc;

    /* Handle of the receive coroutine currently being executed on this
       socket. Also any socket-scoped asynchronous operations such as connect,
       accept or term. */
    void *recvcfptr;

    /* Handle of the send coroutine currently being executed on this socket. */
    void *sendcfptr;
};

int tcpsocket_init (
    struct tcpsocket *self,
    struct mill_loop *loop);

extern const struct mill_type mill_type_tcpsocket_term;

void mill_go_tcpsocket_term (
    const struct mill_type *type,
    struct mill_loop *loop,
    void *parent,
    struct tcpsocket * self);

void tcpsocket_term (
    struct tcpsocket * self);

int tcpsocket_bind (
    struct tcpsocket *self,
    struct sockaddr *addr,
    int flags);

int tcpsocket_listen (
    struct tcpsocket *self,
    int backlog);

extern const struct mill_type mill_type_tcpsocket_connect;

void mill_go_tcpsocket_connect (
    const struct mill_type *type,
    struct mill_loop *loop,
    void *parent,
    int *rc,
    struct tcpsocket * self,
    struct sockaddr * addr);

void tcpsocket_connect (
    int *rc, 
    struct tcpsocket * self, 
    struct sockaddr * addr);

extern const struct mill_type mill_type_tcpsocket_accept;

void mill_go_tcpsocket_accept (
    const struct mill_type *type,
    struct mill_loop *loop,
    void *parent,
    int *rc,
    struct tcpsocket * self,
    struct tcpsocket * newsock);

void tcpsocket_accept (
    int *rc, 
    struct tcpsocket * self, 
    struct tcpsocket * newsock);

extern const struct mill_type mill_type_tcpsocket_send;

void mill_go_tcpsocket_send (
    const struct mill_type *type,
    struct mill_loop *loop,
    void *parent,
    int *rc,
    struct tcpsocket * self,
    const void * buf,
    size_t len);

void tcpsocket_send (
    int *rc, 
    struct tcpsocket * self, 
    const void * buf, 
    size_t len);

extern const struct mill_type mill_type_tcpsocket_recv;

void mill_go_tcpsocket_recv (
    const struct mill_type *type,
    struct mill_loop *loop,
    void *parent,
    int *rc,
    struct tcpsocket * self,
    void * buf,
    size_t len);

void tcpsocket_recv (
    int *rc, 
    struct tcpsocket * self, 
    void * buf, 
    size_t len);

#endif

