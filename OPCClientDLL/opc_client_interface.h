// opc_client_interface.h - Contains declarations of math functions
#pragma once

#ifdef DLL_EXPORTS
#define OPC_CLIENT_DLL_API __declspec(dllexport)
#else
#define OPC_CLIENT_DLL_API __declspec(dllimport)
#endif

#include <map>
#include <vector>
#include <functional>

using ServerNames = std::vector<std::wstring>;
using ServerItems = std::vector<std::wstring>;
using ReadCallback = std::function<void(std::wstring, VARIANT &var)>;

class OPC_CLIENT_DLL_API OPCClientInterface
{
  public:
    virtual ~OPCClientInterface()
    {
    }

    virtual void Init() = 0;
    virtual void Stop() = 0;
    virtual ServerNames GetServers(std::wstring &host_name) = 0;
    virtual bool Connect(std::wstring &host_name, std::wstring &server_name) = 0;
    virtual ServerItems GetItems() = 0;
    virtual void AddGroup(std::wstring &group_name, ReadCallback callback) = 0;
    virtual void AddItem(std::wstring &group_name, std::wstring &item_name) = 0;
    virtual VARENUM GetItemType(std::wstring &item_name) = 0;
    virtual void ReadSync(std::wstring &group_name) = 0;
    virtual bool WriteSync(std::wstring &item_name, VARIANT &var) = 0;
    virtual void ReadAsync(std::wstring &group_nam) = 0;
    virtual void WriteAsync(std::wstring &item_name, VARIANT &var) = 0;
};

extern "C" OPC_CLIENT_DLL_API ServerNames;
extern "C" OPC_CLIENT_DLL_API ServerItems;
extern "C" OPC_CLIENT_DLL_API ReadCallback;
extern "C" OPC_CLIENT_DLL_API OPCClientInterface *instance_create();
extern "C" OPC_CLIENT_DLL_API void instance_destroy(OPCClientInterface *instance);
