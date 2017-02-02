/**
 *  \author Fábio Maia
 *  \tester Fábio Maia
 */

#include "filecluster.h"

#include "probing.h"
#include "exception.h"
#include "czdealer.h"
#include "core.h"

#include <errno.h>

void soWriteFileCluster(int ih, uint32_t fcn, void *buf)
{
    soProbe(600, "soWriteFileCluster(%d, %u, %p)\n", ih, fcn, buf);

    /* Get the physical cluster number */
    uint32_t cn; soGetFileCluster(ih, fcn, &cn);

    /* If the cluster isn't there yet, allocate it */
    if (cn == NULL_REFERENCE)
        soAllocFileCluster(ih, fcn, &cn);

    /* Write the buffer to the cluster */
    soWriteCluster(cn, buf);
}
