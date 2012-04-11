#ifndef PRIMITIVES_H__
#define PRIMITIVES_H__
/**
 * \file
 * This file is the communication primitives module header file
 * of the Session C runtime library (libsc)
 *
 */

#include <stdarg.h>

#include "sc/session.h"

/**
 * \brief Send an integer.
 *
 * @param[in] val Value to send
 * @param[in] r   Role to send to
 *
 * \returns 0 if successful, -1 otherwise and set errno
 *          (See man page of zmq_send)
 */
int send_int(int val, role *r);


/**
 * \brief Send an integer to multiple roles.
 *
 * @param[in] val         Value to send
 * @param[in] nr_of_roles Number of roles to send to 
 * @param[in] ...         Variable number (subject to nr_of_roles)
 *                        of role variables
 *
 * \returns 0 if successful, -1 otherwise and set errno
 *          (See man page of zmq_recv)
 */
int vsend_int(int val, int nr_of_roles, ...);


/**
 * \brief Receive an integer (pre-allocated).
 *
 * @param[out] dst Pointer to variable storing recevied value
 * @param[in]  r   Role to receive from
 *
 * \returns 0 if successful, -1 otherwise and set errno
 *          (See man page of zmq_recv)
 */
int recv_int(int *dst, role *r);


#endif // PRIMITIVES_H__
