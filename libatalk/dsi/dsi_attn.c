/*
 * $Id: dsi_attn.c,v 1.5.6.1 2003/10/17 00:01:14 didg Exp $
 *
 * Copyright (c) 1997 Adrian Sun (asun@zoology.washington.edu)
 * All rights reserved. See COPYRIGHT.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>

#include <atalk/dsi.h>
#include <atalk/afp.h>
#include <netatalk/endian.h>

#ifndef MIN
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#endif /* MIN */

/* send an attention. this may get called at any time, so we can't use
 * DSI buffers to send one. 
   return 0 on error
 
 */
int dsi_attention(DSI *dsi, AFPUserBytes flags)
{
  /* header + AFPUserBytes */
  char block[DSI_BLOCKSIZ + sizeof(AFPUserBytes)];
  sigset_t oldset;
  u_int32_t len, nlen;
  u_int16_t id;

  if (dsi->asleep)
      return 1;
      
  id = htons(dsi_serverID(dsi));
  flags = htons(flags);
  len = MIN(sizeof(flags), dsi->attn_quantum);
  nlen = htonl(len);

  memset(block, 0, sizeof(block));
  block[0] = DSIFL_REQUEST; /* sending a request */
  block[1] = DSIFUNC_ATTN;  /* it's an attention */
  memcpy(block + 2, &id, sizeof(id));
  /* code = 0 */
  memcpy(block + 8, &nlen, sizeof(nlen));
  memcpy(block + 16, &flags, sizeof(flags));
  /* reserved = 0 */

  /* send an attention */
  sigprocmask(SIG_BLOCK, &dsi->sigblockset, &oldset);
  len = dsi_stream_write(dsi, block, DSI_BLOCKSIZ + len, 0);
  sigprocmask(SIG_SETMASK, &oldset, NULL);

  return len;
}