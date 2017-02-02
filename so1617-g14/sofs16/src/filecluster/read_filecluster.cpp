/**
 *  \author Francisco Cunha
 *  \tester Francisco Cunha
 */

#include "filecluster.h"

#include "probing.h"
#include "exception.h"

#include "czdealer.h"
#include "core.h"
#include "stdio.h"

#include <errno.h>

/*	param ih inode handler
    param fcn file cluster number
    param buf pointer to the buffer where data must be read into */

void soReadFileCluster(int ih, uint32_t fcn, void *buf)
{
    soProbe(600, "soReadFileCluster(%d, %u, %p)\n", ih, fcn, buf);

    /* Get the physical cluster number */
    uint32_t cn; soGetFileCluster(ih, fcn, &cn);

    /* Read the cluster into the buffer */
    if (cn == NULL_REFERENCE)
        memset(buf, '\0', soGetBPC());
    else
        soReadCluster(cn, buf);
}
