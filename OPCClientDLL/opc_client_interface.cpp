#include "pch.h" // use stdafx.h in Visual Studio 2017 and earlier
#include <algorithm>
#include <iostream>
#include <limits.h>
#include <mutex>
#include <sys\timeb.h>
#include <thread>
#include <utility>

#include "OPCClient.h"
#include "OPCGroup.h"
#include "OPCHost.h"
#include "OPCItem.h"
#include "OPCServer.h"
#include "opcda.h"

#include "opc_client_interface.h"

using namespace std;

using Items = vector<COPCItem *>;
using GroupTuple = tuple<COPCGroup *, Items, ReadCallback>;

class OPCClient : public OPCClientInterface
{
  public:
    void Init() override;
    void Stop() override;
    bool Connect(wstring &host_name, wstring &server_name) override;
    ServerNames GetServers(wstring &host_name) override;
    ServerItems GetItems() override;
    void AddGroup(wstring &group_name, ReadCallback callback) override;
    void AddItem(wstring &group_name, wstring &item_name) override;
    VARENUM GetItemType(wstring &item_name) override;
    void ReadSync(wstring &group_name) override;
    bool WriteSync(wstring &item_name, VARIANT &var) override;
    void ReadAsync(std::wstring &group_nam) override;
    void WriteAsync(std::wstring &item_name, VARIANT &var) override;

  private:
    wstring host_name_;
    wstring server_name_;
    COPCHost *host_;
    COPCServer *server_;
    map<wstring, GroupTuple> groups_;
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

bool OPCClient::Connect(wstring &host_name, wstring &server_name)
{
    host_name_ = host_name;
    server_name_ = server_name;

    host_ = COPCClient::makeHost(host_name_);
    vector<CLSID> local_class_ids;
    vector<wstring> local_servers;
    host_->getListOfDAServers(IID_CATID_OPCDAServer20, local_servers, local_class_ids);

    int id = -1;
    for (unsigned i = 0; i < local_servers.size(); i++)
    {
        if (local_servers[i] == server_name_)
        {
            server_ = host_->connectDAServer(server_name_);
            ServerStatus status{0};
            const char *server_state_msg[] = {"unknown",   "running", "failed",    "co-config",
                                              "suspended", "test",    "comm-fault"};
            for (int i = 0; i < 10; i++)
            {
                server_->getStatus(status);
                // wcout << "server state is " << server_state_msg[status.dwServerState] << ", vendor is "
                //      << status.vendorInfo.c_str() << endl;

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

ServerNames OPCClient::GetServers(wstring &host_name)
{
    COPCHost *host = COPCClient::makeHost(host_name);
    vector<CLSID> local_class_ids;
    ServerNames servers;
    host->getListOfDAServers(IID_CATID_OPCDAServer20, servers, local_class_ids);
    delete host;
    return servers;
}

ServerItems OPCClient::GetItems()
{
    ServerItems items{};
    server_->getItemNames(items);
    return items;
}

void OPCClient::AddGroup(wstring &group_name, ReadCallback callback)
{
    if (groups_.end() != groups_.find(group_name))
    {
        return;
    }

    unsigned long refresh_rate;
    COPCGroup *group = server_->makeGroup(group_name, true, 1000, refresh_rate, 0.0);
    GroupTuple group_tuple = make_tuple(group, Items{}, callback);
    groups_[group_name] = group_tuple;
}

void OPCClient::AddItem(wstring &group_name, wstring &item_name)
{
    if (groups_.end() == groups_.find(group_name))
    {
        return;
    }

    auto &group_tuple = groups_[group_name];
    auto &items = get<1>(group_tuple);

    auto it = find_if(items.begin(), items.end(), [item_name](COPCItem *item) { return item->getName() == item_name; });

    if (items.end() != it)
    {
        return;
    }

    COPCItem *item = get<0>(group_tuple)->addItem(item_name, true);
    items.push_back(item);
}

VARENUM OPCClient::GetItemType(wstring &item_name)
{
    VARENUM type = VT_NULL;
    for_each(groups_.begin(), groups_.end(), [item_name, &type](auto &group_kv) {
        GroupTuple &group_tuple = group_kv.second;
        Items &items = get<1>(group_tuple);

        auto it =
            find_if(items.begin(), items.end(), [item_name](COPCItem *item) { return item->getName() == item_name; });
        if (items.end() != it)
        {
            auto item = *it;
            vector<CPropertyDescription> prop_desc;
            item->getSupportedProperties(prop_desc);
            CAutoPtrArray<SPropertyValue> prop_vals;
            item->getProperties(prop_desc, prop_vals);

            if (prop_vals[0])
            {
                // wcout << "name: " << item_name << ", data type: " << prop_vals[0]->value.iVal << endl;
                type = static_cast<VARENUM>(prop_vals[0]->value.iVal);
            }
        }
    });

    return type;
}

void OPCClient::ReadSync(wstring &group_name)
{
    if (groups_.end() == groups_.find(group_name))
    {
        return;
    }

    auto &group_tuple = groups_[group_name];
    auto &group = get<0>(group_tuple);
    auto &items = get<1>(group_tuple);
    auto &callback = get<2>(group_tuple);

    if (items.empty())
    {
        return;
    }

    COPCItemDataMap item_data_map;
    group->readSync(items, item_data_map, OPC_DS_DEVICE);
    POSITION pos = item_data_map.GetStartPosition();
    while (pos)
    {
        OPCHANDLE handle = item_data_map.GetKeyAt(pos);
        OPCItemData *data = item_data_map.GetNextValue(pos);
        if (data)
        {
            const COPCItem *item = data->item();
            if (item)
            {
                //printf("-----> '%ws', handle: %u, group sync read quality %d value %d\n", item->getName().c_str(),
                //       handle, data->wQuality, data->vDataValue.iVal);

                callback(item->getName(), data->vDataValue);
            }
        }
    }

}

bool OPCClient::WriteSync(wstring &item_name, VARIANT &var)
{
    bool result{false};
    for_each(groups_.begin(), groups_.end(), [item_name, &var, &result](auto &group_kv) {
        GroupTuple &group_tuple = group_kv.second;
        Items &items = get<1>(group_tuple);

        auto it =
            find_if(items.begin(), items.end(), [item_name](COPCItem *item) { return item->getName() == item_name; });
        if (items.end() != it)
        {
            auto item = *it;
            result = item->writeSync(var);
        }
    });

    return result;
}

void OPCClient::ReadAsync(std::wstring &group_nam)
{
    // TODO:
}

void OPCClient::WriteAsync(std::wstring &item_name, VARIANT &var)
{
    // TODO:
}
