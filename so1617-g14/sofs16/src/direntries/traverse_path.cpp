/**
 *  \author Inês Moreira
 *  \tester Inês Moreira
 */

#include "direntries.h"
#include "direntry.h"
#include "dealers.h"
#include "core.h"
#include "unistd.h"
#include "sys/stat.h"
#include "errno.h"
#include "probing.h"
#include "exception.h"

#include <errno.h>
#include <libgen.h>

void soTraversePath(char *path, uint32_t * inp)
{
    soProbe(400, "soTraversePath(%s, %p)\n", path, inp);

    char *xpath = strdupa(path);
    char *bn = strdupa(basename(xpath));
    char *dn = dirname(xpath);

    /* Check if path and its components are valid */
    if(path == NULL || path[0] != '/')
        throw SOException(EINVAL, __FUNCTION__);

    if(strlen(bn) > SOFS16_MAX_NAME)
        throw SOException(ENAMETOOLONG, __FUNCTION__);

    /* Recursion base case */
    if(strcmp(path, "/") == 0) {
        *inp = 0;
        return;
    }

    /* Recursively get the path components' inode numbers */
    soTraversePath(dn, inp);
    int ih = iOpen(*inp);

    /* Check if we have execute permissions on the path component */
    if(!iCheckAccess(ih, X_OK)) {
        iClose(ih);
        throw SOException(EACCES, __FUNCTION__);
    }

    /* Get the component's inode number */
    soGetDirEntry(ih, bn, inp);
    if(*inp == NULL_REFERENCE) {
        iClose(ih);
        throw SOException(ENOENT, __FUNCTION__);
    }

    iClose(ih);
}
