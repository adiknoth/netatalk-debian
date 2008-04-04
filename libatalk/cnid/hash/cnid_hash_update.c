/*
 * $Id: cnid_hash_update.c,v 1.1.2.1.2.1 2005/01/30 20:56:23 didg Exp $
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#ifdef CNID_BACKEND_HASH

#include "cnid_hash.h"

int cnid_hash_update(struct _cnid_db *cdb, const cnid_t id, const struct stat *st,
                     const cnid_t did, char *name, const int len
                     /*, const char *info, const int infolen */ )
{
    return 0;
}

#endif  /* CNID_BACKEND_HASH */
