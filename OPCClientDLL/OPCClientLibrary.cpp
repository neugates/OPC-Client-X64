#include "pch.h" // use stdafx.h in Visual Studio 2017 and earlier
#include <limits.h>
#include <utility>
#include <sys\timeb.h>

#include "OPCClient.h"
#include "OPCGroup.h"
#include "OPCHost.h"
#include "OPCItem.h"
#include "OPCServer.h"
#include "opcda.h"

#include "OPCClientLibrary.h"

struct opc_host
{
    COPCHost *host;
};

struct opc_client
{
    void *object;
};

void opc_client_init()
{
    COPCClient::init();
}

opc_host_t *opc_client_host(char *host_name)
{
    opc_host_t *host = (opc_host_t *)malloc(sizeof(opc_host_t));
    if (NULL == host)
    {
        return NULL;
    }

    host->host = COPCClient::makeHost(COPCHost::S2WS(host_name));
    return host;
}
