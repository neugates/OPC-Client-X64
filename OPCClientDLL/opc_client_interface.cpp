#include "pch.h" // use stdafx.h in Visual Studio 2017 and earlier
#include <iostream>
#include <limits.h>
#include <sys\timeb.h>
#include <utility>

#include "OPCClient.h"
#include "OPCGroup.h"
#include "OPCHost.h"
#include "OPCItem.h"
#include "OPCServer.h"
#include "opcda.h"

#include "opc_client_interface.h"

using namespace std;

class OPCClient : public OPCClientInterface
{
  public:
    void Init() override;
    void Stop() override;
    bool Connect(std::string &hostName, std::string &serverName) override;
    ServerNames GetServers(std::string &hostName) override;
    ServerItems GetItems() override;

  private:
    wstring host_name_;
    wstring server_name_;
    COPCHost *host_;
    COPCServer *server_;
};

OPCClientInterface *instance_create()
{
    return new OPCClient();
}

void instance_destroy(OPCClientInterface *instance)
{
    OPCClient *client = (OPCClient *)instance;
    delete client;
}

void OPCClient::Init()
{
    COPCClient::init();
}

void OPCClient::Stop()
{
    COPCClient::stop();
}

bool OPCClient::Connect(std::string &hostName, std::string &serverName)
{
    host_name_ = COPCHost::S2WS(hostName);
    server_name_ = COPCHost::S2WS(serverName);

    host_ = COPCClient::makeHost(host_name_);
    vector<CLSID> localClassIds;
    vector<wstring> localServers;
    host_->getListOfDAServers(IID_CATID_OPCDAServer20, localServers, localClassIds);

    int id = -1;
    for (unsigned i = 0; i < localServers.size(); i++)
    {
        if (localServers[i] == server_name_)
        {
            server_ = host_->connectDAServer(server_name_);
            ServerStatus status{0};
            const char *serverStateMsg[] = {"unknown",   "running", "failed",    "co-config",
                                            "suspended", "test",    "comm-fault"};
            for (int i = 0; i < 10; i++)
            {
                server_->getStatus(status);
                cout << "server state is " << serverStateMsg[status.dwServerState] << ", vendor is "
                     << status.vendorInfo.c_str() << endl;

                if (status.dwServerState == OPC_STATUS_RUNNING)
                {
                    return true;
                }

                Sleep(100);
            }

            delete server_;
            server_ = nullptr;

            delete host_;
            host_ = nullptr;
            return false;
        }
    }

    delete host_;
    host_ = nullptr;
    return false;
}

ServerNames OPCClient::GetServers(std::string &hostName)
{
    COPCHost *host = COPCClient::makeHost(COPCHost::S2WS(hostName));
    ServerNames servers;
    vector<CLSID> localClassIds;
    vector<wstring> localServers;
    host->getListOfDAServers(IID_CATID_OPCDAServer20, localServers, localClassIds);

    for (unsigned i = 0; i < localServers.size(); i++)
    {
        servers.push_back(COPCHost::WS2S(localServers[i]));
    }

    delete host;
    return servers;
}

ServerItems OPCClient::GetItems()
{
    return ServerItems();
}
