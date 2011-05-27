/*
  Copyright (c) 2008,2009 Frank Lahm <franklahm@gmail.com>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#ifdef HAVE_LDAP
#define LDAP_DEPRECATED 1
#include <ldap.h>
#endif

#include <atalk/ldapconfig.h>
#include <atalk/uuid.h>
#include <atalk/logger.h>

#define STRNCMP(a, R, b, l) (strncmp(a,b,l) R 0)

static void usage()
{
    printf("Usage: afpldaptest -u <user> | -g <group> | -i <UUID>\n");
}

static void parse_ldapconf()
{
    static int inited = 0;

    if (! inited) {
#ifdef HAVE_LDAP
        /* Parse afp_ldap.conf */
        printf("Start parsing afp_ldap.conf\n");
        acl_ldap_readconfig(_PATH_ACL_LDAPCONF);
        printf("Finished parsing afp_ldap.conf\n");
        if (ldap_config_valid) {
            if (ldap_auth_method == LDAP_AUTH_NONE)
                printf("afp_ldap.conf is ok. Using anonymous bind.\n");
            else if (ldap_auth_method == LDAP_AUTH_SIMPLE)
                printf("afp_ldap.conf is ok. Using simple bind.\n");
            else {
                ldap_config_valid = 0;
                printf("afp_ldap.conf wants SASL which is not yet supported.\n");
                exit(EXIT_FAILURE);
            }
        } else {
            printf("afp_ldap.conf is not ok, not using LDAP. Only local UUID testing available.\n");
        }
#else
        printf("Built without LDAP support, only local UUID testing available.\n");
#endif
        inited = 1;
    }
}

int main( int argc, char **argv)
{
    int ret, c;
    int verbose = 0;
    atalk_uuid_t uuid;
    int logsetup = 0;
    uuidtype_t type;
    char *name = NULL;

    while ((c = getopt(argc, argv, ":vu:g:i:")) != -1) {
        switch(c) {

        case 'v':
            if (! verbose) {
                verbose = 1;
                setuplog("default log_maxdebug /dev/tty");
                logsetup = 1;
            }
            break;

        case 'u':
            if (! logsetup)
                setuplog("default log_info /dev/tty");
            parse_ldapconf();
            printf("Searching user: %s\n", optarg);
            ret = getuuidfromname( optarg, UUID_USER, uuid);
            if (ret == 0) {
                printf("User: %s ==> UUID: %s\n", optarg, uuid_bin2string(uuid));
            } else {
                printf("User %s not found.\n", optarg);
            }
            break;

        case 'g':
            if (! logsetup)
                setuplog("default log_info /dev/tty");
            parse_ldapconf();
            printf("Searching group: %s\n", optarg);
            ret = getuuidfromname( optarg, UUID_GROUP, uuid);
            if (ret == 0) {
                printf("Group: %s ==> UUID: %s\n", optarg, uuid_bin2string(uuid));
            } else {
                printf("Group %s not found.\n", optarg);
            }
            break;

        case 'i':
            if (! logsetup)
                setuplog("default log_info /dev/tty");
            parse_ldapconf();
            printf("Searching uuid: %s\n", optarg);
            uuid_string2bin(optarg, uuid);
            ret = getnamefromuuid( uuid, &name, &type);
            if (ret == 0) {
                switch (type) {
                case UUID_LOCAL:
                    printf("local UUID: %s\n", optarg);
                    break;
                case UUID_USER:
                    printf("UUID: %s ==> User: %s\n", optarg, name);
                    break;
                case UUID_GROUP:
                    printf("UUID: %s ==> Group: %s\n", optarg, name);
                    break;
                }
                free(name);
            } else {
                printf("UUID: %s not found.\n", optarg);
            }
            break;

        case ':':
        case '?':
        case 'h':
            usage();
            exit(EXIT_FAILURE);
        }
    }

    return 0;
}

