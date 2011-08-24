/*! \file pmip_hnp_cache.c
* \brief PMIP binding cache functions
* \author OpenAir3 Group
* \date 12th of October 2010
* \version 1.0
* \company Eurecom
* \project OpenAirInterface
* \email: openair3@eurecom.fr
*/
#define PMIP
#define PMIP_HNP_CACHE_C
#include "pmip_hnp_cache.h"
#include "pmip_config.h"
#include    <config.h>
#include    <includes.h>
#include    <freeradius-client.h>
#include    <pathnames.h>

mnid_hnp_t mn_hn_map[MNCOUNT];

static int mn_count = 0;
static rc_handle *rh;
static char *default_realm;
static char msg[4096];
static char username[256];
static char username_realm[256];
static char passwd[AUTH_PASS_LEN + 1];
/*!
*  translate a MAC address (6 bytes) into an interface id (ipv6 suffix of 8 bytes)
*
* \param  macaddr a MAC address
* \return the corresponding IPV6 interface id.
*/
struct in6_addr eth_address2hw_address(struct in6_addr macaddr)
{
    struct in6_addr iid;
    iid.s6_addr16[0] = 0;
    iid.s6_addr16[1] = 0;
    iid.s6_addr16[2] = 0;
    iid.s6_addr16[3] = 0;
    iid.s6_addr16[4] = 0;
    iid.s6_addr16[5] = macaddr.s6_addr16[4] & ntohs(0x00FF);    // remove 0x02 part of 0x02xx
    iid.s6_addr16[6] = (macaddr.s6_addr16[6] & ntohs(0x00FF)) | (macaddr.s6_addr16[5] & ntohs(0xFF00)); // remove fffe part of xxff.fexx
    iid.s6_addr16[7] = macaddr.s6_addr16[7];
    return iid;
}
struct in6_addr hw_address2eth_address(struct in6_addr iid)
{
    struct in6_addr macaddr;
    macaddr.s6_addr16[0] = 0;
    macaddr.s6_addr16[1] = 0;
    macaddr.s6_addr16[2] = 0;
    macaddr.s6_addr16[3] = 0;
    macaddr.s6_addr16[4] = iid.s6_addr16[5] | ntohs(0x0200);    // add 0x0200 part
    macaddr.s6_addr16[5] = (iid.s6_addr16[6] | ntohs(0x00FF));
    macaddr.s6_addr16[6] = (iid.s6_addr16[6] & ntohs(0x00FF)) | ntohs(0xFE00);
    macaddr.s6_addr16[7] = iid.s6_addr16[7];
    return macaddr;
}

/*!
*  insert into the cache the mapping between  the mnid and the ipv6 address assigned
* \param mn_iid Mobile node identifier
* \param addr   Mobile node IPV6 address
*/
void pmip_insert_into_hnp_cache(struct in6_addr mn_iid, struct in6_addr addr)
{
    int j = 0;
    while (j < mn_count) {
        if (IN6_ARE_ADDR_EQUAL(&mn_hn_map[j].mn_iid, &mn_iid)) {
            dbg("mnid %x:%x:%x:%x:%x:%x:%x:%x already in cache, updating addr %x:%x:%x:%x:%x:%x:%x:%x -> %x:%x:%x:%x:%x:%x:%x:%x\n", NIP6ADDR(&mn_hn_map[j].mn_iid), NIP6ADDR(&mn_hn_map[j].mn_prefix), NIP6ADDR(&addr));
            mn_hn_map[j].mn_prefix = addr;
            return;
        }
        j++;
    }
    mn_hn_map[mn_count].mn_prefix = addr;
    mn_hn_map[mn_count].mn_iid = mn_iid;
    dbg("new entry in cache %x:%x:%x:%x:%x:%x:%x:%x -> %x:%x:%x:%x:%x:%x:%x:%x\n", NIP6ADDR(&mn_hn_map[mn_count].mn_prefix), NIP6ADDR(&mn_hn_map[mn_count].mn_iid));
    mn_count = mn_count + 1;
}



/*!
*  Initialize the home network prefix cache in the LMA
*/
void pmip_lma_mn_to_hnp_cache_init(void)
{
    memset((void*)mn_hn_map, 0, sizeof(mnid_hnp_t) * MNCOUNT);
}



/*!
*  Search if the mobile node id is already associated with a prefix in the hnp map
* \param mnid Mobile node ID
* \return a valid prefix if the mobile node id is already associated with a prefix in the hnp map
*/
struct in6_addr lma_mnid_hnp_map(struct in6_addr mnid, int *result)
{
    int j = 0;
    dbg("Entering the address match . . ");
    dbg("Searching for MNID  %x:%x:%x:%x:%x:%x:%x:%x  \n", NIP6ADDR(&mnid));
    while (j < mn_count) {
        dbg("Comparing with MNID  %x:%x:%x:%x:%x:%x:%x:%x  \n", NIP6ADDR(&mn_hn_map[j].mn_iid));
        if (IN6_ARE_ADDR_EQUAL(&mn_hn_map[j].mn_iid, &mnid)) {
            *result = 1;
            dbg("%x:%x:%x:%x:%x:%x:%x:%x found the prefix \n", NIP6ADDR(&mn_hn_map[j].mn_prefix));
            return (mn_hn_map[j].mn_prefix);
        }
        j++;
    }
    dbg("mnid not found ");

    struct in6_addr tmp;
    memset(&tmp, 0, sizeof(struct in6_addr));
    *result = -1;
    return tmp;
}



/*!
*  Initialize the home network prefix cache, if RADIUS is not configured the matching between MAC addresse and prefixes is read from a FILE "match"
* \return status of execution
*/
/*
int pmip_mn_to_hnp_cache_init(void)
{
    // LG not necessary memset(mn_hn_map, 0, sizeof(mnid_hnp_t) * MNCOUNT);
    rc_openlog("pmip_radius_client");
    if ((rh = rc_read_config(RC_CONFIG_FILE)) == NULL)
    return ERROR_RC;
    if (rc_read_dictionary(rh, rc_conf_str(rh, "dictionary")) != 0)
    return ERROR_RC;
    default_realm = rc_conf_str(rh, "default_realm");
    return 0;
}
*/

int pmip_mn_to_hnp_cache_init (void)
{
    FILE               *fp;

    char                str_addr[40], str_addr_iid[40];

    struct in6_addr     addr, addr1;

    unsigned int        ap, ap1;

    int                 i, j;

    j = 0;
    if ((fp = fopen ("/etc/match", "r")) == NULL) {
        printf ("can't open %s:", "/match");
        exit (0);
    }
    while ((fscanf (fp, "%32s %32s\n", str_addr, str_addr_iid) != EOF) && (j < MNCOUNT)) {
        for (i = 0; i < 16; i++) {
            sscanf (str_addr + i * 2, "%02x", &ap);
            addr.s6_addr[i] = (unsigned char) ap;
            mn_hn_map[j].mn_prefix = addr;
            sscanf (str_addr_iid + i * 2, "%02x", &ap1);
            addr1.s6_addr[i] = (unsigned char) ap1;
            mn_hn_map[j].mn_iid = addr1;
        }
        dbg ("%x:%x:%x:%x:%x:%x:%x:%x\t", NIP6ADDR (&mn_hn_map[j].mn_prefix));
        dbg ("%x:%x:%x:%x:%x:%x:%x:%x\n", NIP6ADDR (&mn_hn_map[j].mn_iid));
        j++;
        mn_count = mn_count + 1;
    }
    fclose (fp);
    if (j >= MNCOUNT) {
        dbg ("[MNP CACHE][INIT] ERROR TOO MANY MAPPINGS DEFINED IN CONFIG FILE\n");
        exit (0);
    }
    return 0;
}




/*!
*  Search if the mobile node id is already associated with a prefix in the hnp map
* \param mnid Mobile node ID
* \return a valid prefix if the mobile node id is already associated with a prefix in the hnp map
*/
struct in6_addr mnid_hnp_map(struct in6_addr mnid, int *aaa_result)
{
    int fl1ag = 0;
    int j = 0;
    dbg("Entering the address match . . ");
    dbg("Searching for MNID  %x:%x:%x:%x:%x:%x:%x:%x  \n", NIP6ADDR(&mnid));
    while (j < mn_count) {
        dbg("Comparing with MNID  %x:%x:%x:%x:%x:%x:%x:%x  \n", NIP6ADDR(&mn_hn_map[j].mn_iid));
        if (IN6_ARE_ADDR_EQUAL(&mn_hn_map[j].mn_iid, &mnid)) {
            fl1ag = 1;
            dbg("%x:%x:%x:%x:%x:%x:%x:%x found the prefix \n", NIP6ADDR(&mn_hn_map[j].mn_prefix));
            *aaa_result = 10;
            return (mn_hn_map[j].mn_prefix);
        }
        j++;
    }

/*    VALUE_PAIR *send, *received;
    VALUE_PAIR *vp;
    struct in6_addr prefix;
    uint32_t service;


    *aaa_result = 0;

    sprintf(username, "%04x%04x%04x%04x", ntohs(mnid.s6_addr16[4]), ntohs(mnid.s6_addr16[5]), ntohs(mnid.s6_addr16[6]), ntohs(mnid.s6_addr16[7]));
    strncpy(passwd, RADIUS_PASSWORD, sizeof(RADIUS_PASSWORD));

// Fill in User-Name

    strncpy(username_realm, username, sizeof(username_realm));
// Append default realm 
    if ((strchr(username_realm, '@') == NULL) && default_realm && (*default_realm != '\0')) {
    strncat(username_realm, "@", sizeof(username_realm) - strlen(username_realm) - 1);
    strncat(username_realm, default_realm, sizeof(username_realm) - strlen(username_realm) - 1);
    }
    dbg("RADIUS USER NAME %s\n", username_realm);
    dbg("RADIUS PASSWORD  %s\n", passwd);
    if (rc_avpair_add(rh, &send, PW_USER_NAME, username_realm, -1, 0) == NULL) {
        fprintf(stderr, "[RADIUS] ERROR rc_avpair_add PW_USER_NAME\n");
    } else {
//
// Fill in User-Password

    if (rc_avpair_add(rh, &send, PW_USER_PASSWORD, passwd, -1, 0) == NULL) {
        fprintf(stderr, "[RADIUS] ERROR rc_avpair_add PW_USER_PASSWORD\n");
    } else {

// Fill in Service-Type

        service = PW_AUTHENTICATE_ONLY;
        if (rc_avpair_add(rh, &send, PW_SERVICE_TYPE, &service, -1, 0)
        == NULL) {
        fprintf(stderr, "[RADIUS] ERROR rc_avpair_add PW_SERVICE_TYPE\n");
        } else {
        // result = RESULT always < 0 !!!
        rc_auth(rh, 0, send, &received, msg);
        {
            *aaa_result = 0;
            if (received != NULL) {
            if ((vp = rc_avpair_get(received, PW_FRAMED_IPV6_PREFIX, 0)) != NULL) {
                *aaa_result += 1;
                int netmask = vp->strvalue[1];
                int num_bytes = netmask / 8;
                int i;
                for (i = 0; i < num_bytes; i++) {
                prefix.s6_addr[i] = vp->strvalue[2 + i];
                }
                for (i = num_bytes; i < 16; i++) {
                prefix.s6_addr[i] = 0;
                }
            }
            if ((vp = rc_avpair_get(received, PW_FRAMED_INTERFACE_ID, 0)) != NULL) {
                *aaa_result += 1;
                int i;
                for (i = 0; i < 8; i++) {
                prefix.s6_addr[8 + i] = prefix.s6_addr[8 + i] | vp->strvalue[i];
                }
            }
            rc_avpair_free(received);
            }
            if (*aaa_result >= 2) {
            fl1ag = 1;
            fprintf(stderr,
                "[RADIUS] Assigned IPv6 @ for MN UID %x:%x:%x:%x:%x:%x:%x:%x <=> %x:%x:%x:%x:%x:%x:%x:%x\n", NIP6ADDR(&mnid), NIP6ADDR(&prefix));
            fprintf(stderr, "[RADIUS] \"%s\" Authentication OK\n", username);
            pmip_insert_into_hnp_cache(mnid, prefix);
            return prefix;
            }
        }
        }
    }
    }
*/
    if (fl1ag == 0) {
        dbg("mnid not found ");
        struct in6_addr tmp;
        memset(&tmp, 0, sizeof(struct in6_addr));
        *aaa_result = -1;
        return tmp;
    }
}

