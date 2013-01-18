/* $Id: md5ossl.c 25189 2009-10-02 12:04:37Z akr $ */

#include "md5ossl.h"

void
MD5_Finish(MD5_CTX *pctx, unsigned char *digest)
{
    MD5_Final(digest, pctx);
}
