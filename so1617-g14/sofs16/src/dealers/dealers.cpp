/**
 *  \author Fábio Maia
 */

#include "dealers.h"
#include "rawdisk.h"

void soOpenDealersDisk(const char *devname, uint32_t * ntp)
{
    soOpenRawDisk(devname, ntp);
    soOpenSuperblockDealer();
    soOpenClusterZoneDealer();
    soOpenInodeTableDealer();
}

void soCloseDealersDisk()
{
    soCloseSuperblockDealer();
    soCloseClusterZoneDealer();
    soCloseInodeTableDealer();
    soCloseRawDisk();
}
