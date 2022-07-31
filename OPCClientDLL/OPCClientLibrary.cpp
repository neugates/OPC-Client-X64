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
    char host_name[256];
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

void opc_client_stop()
{
    COPCClient::stop();
}

opc_host_t *opc_client_host(const char *host_name)
{
    if (NULL == host_name)
    {
        return NULL;
    }

    opc_host_t *host = (opc_host_t *)malloc(sizeof(opc_host_t));
    if (NULL == host)
    {
        return NULL;
    }

    memset(host, 0, sizeof(opc_host));
    strncpy_s(host->host_name, host_name, strlen(host_name));

    host->host = COPCClient::makeHost(COPCHost::S2WS(host_name));
    return host;
}

void opc_client_severs(opc_host_t *host)
{
}

