/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myMPD (c) 2018-2021 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/mympd
*/

#include "mympd_config_defs.h"
#include "mympd_pin.h"

#include "log.h"
#include "sds_extras.h"
#include "state_files.h"

#include <stdio.h>
#include <string.h>
#include <termios.h>

#ifdef ENABLE_SSL
    #include <openssl/evp.h>
#endif

//private definitions
static sds pin_hash(const char *pin);

//public functions

void pin_set(sds workdir) {
    struct termios old, new;
    if (tcgetattr(fileno(stdin), &old) != 0) {
        return;
    }
    new = old;
    new.c_lflag &= ~ECHO;
    if (tcsetattr(fileno(stdin), TCSAFLUSH, &new) != 0) {
        return;
    }
        
    printf("Enter pin: ");
    int c;
    sds pin = sdsempty();
    while ((c = getc(stdin)) != '\n') {
        pin = sdscatprintf(pin, "%c", c);
    }
    sds hex_hash;
    if (sdslen(pin) == 0) {
        hex_hash = sdsempty();
    }
    else {
        hex_hash = pin_hash(pin);
    }
    bool rc = state_file_write(workdir, "config", "pin_hash", hex_hash);
    FREE_SDS(hex_hash);
    tcsetattr(fileno(stdin), TCSAFLUSH, &old);
    printf("\n");
    if (rc == true) {
        if (sdslen(pin) > 0) {
            printf("Pin is now set, restart myMPD to apply.\n");
        }
        else {
            printf("Pin is now cleared, restart myMPD to apply.\n");
        }
    }
    FREE_SDS(pin);
}

bool pin_validate(const char *pin, const char *hash) {
    if (hash[0] == '\0') {
        MYMPD_LOG_ERROR("No pin is set");
        return false;
    }
    sds test_hash = pin_hash(pin);
    bool rc = false;
    if (strcmp(test_hash, hash) == 0) {
        MYMPD_LOG_INFO("Valid pin entered");
        rc = true;
    }
    else {
        MYMPD_LOG_ERROR("Invalid pin entered");
    }
    FREE_SDS(test_hash);
    return rc;
}

//private functions

static sds pin_hash(const char *pin) {
    sds hex_hash = sdsempty();
#ifdef ENABLE_SSL
    EVP_MD_CTX* context = EVP_MD_CTX_new();
    if (context == NULL) {
        return hex_hash;
    }
    if (EVP_DigestInit_ex(context, EVP_sha256(), NULL) == 0) {
        EVP_MD_CTX_free(context);
        return hex_hash;
    }
    if (EVP_DigestUpdate(context, pin, strlen(pin)) == 0) {
        EVP_MD_CTX_free(context);
        return hex_hash;
    }
    unsigned char hash[EVP_MAX_MD_SIZE];
    unsigned int hash_len = 0;
    if(EVP_DigestFinal_ex(context, hash, &hash_len) == 0) {
        EVP_MD_CTX_free(context);
        return hex_hash;
    }
    
    for (unsigned int i = 0; i < hash_len; i++) {
        hex_hash = sdscatprintf(hex_hash, "%02x", hash[i]);
    }
    EVP_MD_CTX_free(context);
#else
    (void) pin;
#endif
    return hex_hash;
}