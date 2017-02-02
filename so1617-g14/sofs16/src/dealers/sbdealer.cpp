/**
 *  \author Fábio Maia
 *  \tester Fábio Maia
 */

#include "rawdisk.h"

#include "probing.h"
#include "exception.h"
#include "core.h"
#include "dealers.h"

#include <errno.h>

static SOSuperBlock sb;
static bool isOpen = false;

void soOpenSuperblockDealer()
{
    isOpen = true;
    soReadRawBlock(0, &sb);
}

void soCloseSuperblockDealer()
{
    sbSave();
    isOpen = false;
}

SOSuperBlock *sbGetPointer()
{
    if (isOpen)
        return &sb;
    else
        throw SOException(EBADF, __FUNCTION__);
}

void sbSave()
{
    if (isOpen)
        soWriteRawBlock(0, &sb);
}
