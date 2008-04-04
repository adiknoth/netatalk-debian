/* 
   Unix SMB/CIFS implementation.
   minimal iconv implementation
   Copyright (C) Andrew Tridgell 2001
   Copyright (C) Jelmer Vernooij 2002,2003
   
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
   
   From samba 3.0 beta and GNU libiconv-1.8
   It's bad but most of the time we can't use libc iconv service:
   - it doesn't round trip for most encoding
   - it doesn't know about Apple extension
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */
#include <stdlib.h>
#include <errno.h>

#include <netatalk/endian.h>
#include <atalk/unicode.h>
#include <atalk/logger.h>
#include <atalk/unicode.h>
#include "byteorder.h"


static size_t   utf8_pull(void *,char **, size_t *, char **, size_t *);
static size_t   utf8_push(void *,char **, size_t *, char **, size_t *);

struct charset_functions charset_utf8 =
{
	"UTF8",
	0x08000103,
	utf8_pull,
	utf8_push,
	CHARSET_VOLUME | CHARSET_MULTIBYTE | CHARSET_PRECOMPOSED,
	NULL, NULL
};

struct charset_functions charset_utf8_mac =
{
	"UTF8-MAC",
	0x08000103,
	utf8_pull,
	utf8_push,
	CHARSET_VOLUME | CHARSET_CLIENT | CHARSET_MULTIBYTE | CHARSET_DECOMPOSED,
	NULL, NULL
};

/* ------------------------ */
static size_t utf8_pull(void *cd, char **inbuf, size_t *inbytesleft,
			 char **outbuf, size_t *outbytesleft)
{
	ucs2_t uc = 0;
	int len;

	while (*inbytesleft >= 1 && *outbytesleft >= 2) {
		unsigned char *c = (unsigned char *)*inbuf;
		len = 1;

		if ((c[0] & 0x80) == 0) {
			uc = c[0];
		} else if ((c[0] & 0xf0) == 0xe0) {
			if (*inbytesleft < 3) {
				LOG(log_debug, logtype_default, "short utf8 char");
				goto badseq;
			}
			uc = ((ucs2_t) (c[0] & 0x0f) << 12) | ((ucs2_t) (c[1] ^ 0x80) << 6) | (ucs2_t) (c[2] ^ 0x80);
			len = 3;
		} else if ((c[0] & 0xe0) == 0xc0) {
			if (*inbytesleft < 2) {
				LOG(log_debug, logtype_default, "short utf8 char");
				goto badseq;
			}
			uc = ((ucs2_t) (c[0] & 0x1f) << 6) | (ucs2_t) (c[1] ^ 0x80);
			len = 2;
		}
		else {
			errno = EINVAL;
			return -1;
		}

		SSVAL(*outbuf,0,uc);
		(*inbuf)  += len;
		(*inbytesleft)  -= len;
		(*outbytesleft) -= 2;
		(*outbuf) += 2;
	}

	if (*inbytesleft > 0) {
		errno = E2BIG;
		return -1;
	}
	
	return 0;

badseq:
	errno = EINVAL;
	return -1;
}

/* ------------------------ */
static size_t utf8_push(void *cd, char **inbuf, size_t *inbytesleft,
			 char **outbuf, size_t *outbytesleft)
{
	ucs2_t uc=0;
	int len;

	while (*inbytesleft >= 2 && *outbytesleft >= 1) {
		unsigned char *c = (unsigned char *)*outbuf;
		uc = SVAL((*inbuf),0);
		len=1;

		if ( uc >= 0x800 ) {
			if ( uc >= 0x202a && uc <= 0x202e ) {
				/* ignore bidi hint characters */
			   len = 0;
		 	}
		 	else {
				if (*outbytesleft < 3) {
					LOG(log_debug, logtype_default, "short utf8 write");
					goto toobig;
				}
				c[2] = 0x80 | (uc & 0x3f);
				uc = uc >> 6;
				uc |= 0x800;
                        	c[1] = 0x80 | (uc&0x3f);
                        	uc = uc >> 6;
                        	uc |= 0xc0;
                        	c[0] = uc;
				len = 3;
			}
		} else if (uc >= 0x80) {
			if (*outbytesleft < 2) {
				LOG(log_debug, logtype_default, "short utf8 write");
				goto toobig;
			}
			c[1] = 0x80 | (uc&0x3f);
			uc = uc >> 6;
			uc |= 0xc0;
			c[0] = uc;
			len = 2;
		} else {
			c[0] = uc;
		}

		(*inbytesleft)  -= 2;
		(*outbytesleft) -= len;
		(*inbuf)  += 2;
		(*outbuf) += len;
	}

	if (*inbytesleft == 1) {
		errno = EINVAL;
		return -1;
	}

	if (*inbytesleft > 1) {
		errno = E2BIG;
		return -1;
	}
	
	return 0;

toobig:
	errno = E2BIG;
	return -1;
}
