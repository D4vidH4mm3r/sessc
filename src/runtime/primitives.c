/**
 * \file
 * Session C runtime library (libsc)
 * communication primitives module.
 */

#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <zmq.h>

#include "sc/primitives.h"


/**
 * \brief Helper function to deallocate send queue.
 *
 */
void _dealloc(void *data, void *hint)
{
  free(data);
}


inline int send_int(int val, role *r)
{
  return send_int_array(&val, 1, r);
}


int send_int_array(const int arr[], size_t count, role *r)
{
  int rc = 0;
  zmq_msg_t msg;
  size_t size = sizeof(int) * count;

#ifdef __DEBUG__
  fprintf(stderr, " --> %s ", __FUNCTION__);
#endif

  int *buf = (int *)malloc(size);
  memcpy(buf, arr, size);

  zmq_msg_init_data(&msg, buf, size, _dealloc, NULL);
  switch (r->type) {
    case SESSION_ROLE_P2P:
      rc = zmq_send(r->p2p->ptr, &msg, 0);
      break;
    case SESSION_ROLE_GRP:
#ifdef __DEBUG__
      fprintf(stderr, "bcast -> %s(%d endpoints) ", r->grp->name, r->grp->nendpoint);
#endif

      rc = zmq_send(r->grp->out->ptr, &msg, 0);
      break;
    default:
        fprintf(stderr, "%s: Unknown endpoint type: %d\n", __FUNCTION__, r->type);
  }
  zmq_msg_close(&msg);

  if (rc != 0) perror(__FUNCTION__);
 
#ifdef __DEBUG__
  fprintf(stderr, ".\n");
#endif

  return rc;
}


int vsend_int(int val, int nr_of_roles, ...)
{
  int rc = 0;
  int i;
  role *r;
  va_list roles;

#ifdef __DEBUG__
  fprintf(stderr, " --> %s(%d)@%d ", __FUNCTION__, val, nr_of_roles);
#endif
  va_start(roles, nr_of_roles);
  for (i=0; i<nr_of_roles; i++) {
    r = va_arg(roles, role *);

#ifdef __DEBUG__
    fprintf(stderr, "   +");
#endif
    rc |= send_int(val, r);
    if (rc != 0) perror(__FUNCTION__);
  }
  va_end(roles);

#ifdef __DEBUG__
  fprintf(stderr, ".\n");
#endif

  return rc;
}


inline int recv_int(int *dst, role *r)
{
  size_t count;
  return recv_int_array(dst, &count, r);
}


int recv_int_array(int *arr, size_t *count, role *r)
{
  int rc = 0;
  zmq_msg_t msg;
  size_t size = -1;

#ifdef __DEBUG__
  fprintf(stderr, " <-- %s() ", __FUNCTION__);
#endif

  zmq_msg_init(&msg);
  switch (r->type) {
    case SESSION_ROLE_P2P:
      rc = zmq_recv(r->p2p->ptr, &msg, 0);
      break;
    case SESSION_ROLE_GRP:
#ifdef __DEBUG__
      fprintf(stderr, "bcast <- %s(%d endpoints) ", r->grp->name, r->grp->nendpoint);
#endif
      rc = zmq_recv(r->grp->in->ptr, &msg, 0);
      break;
    default:
        fprintf(stderr, "%s: Unknown endpoint type: %d\n", __FUNCTION__, r->type);
  }
  size = zmq_msg_size(&msg);
  if (*count * sizeof(int) >= size) {
    memcpy(arr, (int *)zmq_msg_data(&msg), size);
    if (size % sizeof(int) == 0) {
      *count = size / sizeof(int);
    }
  } else {
    memcpy(arr, (int *)zmq_msg_data(&msg), *count * sizeof(int));
    fprintf(stderr,
      "%s: Received data (%zu bytes) > memory size (%zu), data truncated\n",
      __FUNCTION__, size, *count * sizeof(int));
  }
  zmq_msg_close(&msg);

  if (rc != 0) perror(__FUNCTION__);

#ifdef __DEBUG__
  fprintf(stderr, "[%d ...] .\n", *arr);
#endif

  return rc;
}


int barrier(role *grp_role, char *at_rolename)
{
  if (grp_role->type != SESSION_ROLE_GRP) {
    fprintf(stderr, "Error: cannot perform barrier synchronisation with non group role!\n");
    return -1;
  }

  int rc = 0;
  zmq_msg_t msg;
  int i;

  if (strcmp(grp_role->s->name, at_rolename) == 0) { // Master role

    rc |= zmq_setsockopt(grp_role->grp->in->ptr, ZMQ_UNSUBSCRIBE, "", 0);
    if (rc != 0) perror(__FUNCTION__);
    rc |= zmq_setsockopt(grp_role->grp->in->ptr, ZMQ_SUBSCRIBE, "S1", 2);
    if (rc != 0) perror(__FUNCTION__);

    // Wait for S1 (Phase 1) messages.
    for (i=0; i<grp_role->grp->nendpoint; ++i) {
      zmq_msg_init(&msg);
      rc |= zmq_recv(grp_role->grp->in->ptr, &msg, 0);
      if (rc != 0) perror(__FUNCTION__);
      zmq_msg_close (&msg);
    }

    // Reset filters.
    rc |= zmq_setsockopt(grp_role->grp->in->ptr, ZMQ_UNSUBSCRIBE, "S1", 2);
    if (rc != 0) perror(__FUNCTION__);
    rc |= zmq_setsockopt(grp_role->grp->in->ptr, ZMQ_SUBSCRIBE, "", 0);
    if (rc != 0) perror(__FUNCTION__);

    zmq_msg_init_size(&msg, 2);
    memcpy(zmq_msg_data(&msg), "S2", 2);
    rc |= zmq_send(grp_role->grp->out->ptr, &msg, 0);
    if (rc != 0) perror(__FUNCTION__);
    zmq_msg_close(&msg);

    // Synchronised.

  } else { // Slave role

    // Send S1 (Phase 1) messages.
    zmq_msg_init_size(&msg, 2);
    memcpy(zmq_msg_data(&msg), "S1", 2);
    rc |= zmq_send(grp_role->grp->out->ptr, &msg, 0);
    if (rc != 0) perror(__FUNCTION__);
    zmq_msg_close(&msg);

    rc |= zmq_setsockopt(grp_role->grp->in->ptr, ZMQ_UNSUBSCRIBE, "", 0);
    if (rc != 0) perror(__FUNCTION__);
    rc |= zmq_setsockopt(grp_role->grp->in->ptr, ZMQ_SUBSCRIBE, "S2", 2);
    if (rc != 0) perror(__FUNCTION__);

    // Wait for S2 (Phase 2) messages.
    zmq_msg_init(&msg);
    rc |= zmq_recv(grp_role->grp->in->ptr, &msg, 0);
    if (rc != 0) perror(__FUNCTION__);
    zmq_msg_close(&msg);

    // Reset filters.
    rc |= zmq_setsockopt(grp_role->grp->in->ptr, ZMQ_UNSUBSCRIBE, "S2", 2);
    if (rc != 0) perror(__FUNCTION__);
    rc |= zmq_setsockopt(grp_role->grp->in->ptr, ZMQ_SUBSCRIBE, "", 0);
    if (rc != 0) perror(__FUNCTION__);

    // Synchronised.
  }

  return rc;
}
