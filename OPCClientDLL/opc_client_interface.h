// opc_client_interface.h - Contains declarations of math functions
#pragma once

#ifdef DLL_EXPORTS
#define OPC_CLIENT_DLL_API __declspec(dllexport)
#else
#define OPC_CLIENT_DLL_API __declspec(dllimport)
#endif

#include <vector>

using ServerNames = std::vector<std::string>;
using ServerItems = std::vector<std::string>;

class OPC_CLIENT_DLL_API OPCClientInterface
{
  public:
    virtual ~OPCClientInterface()
    {
    }

    virtual void Init() = 0;
    virtual void Stop() = 0;
    virtual ServerNames GetServers(std::string &hostName) = 0;
    virtual bool Connect(std::string &hostName, std::string &serverName) = 0;
    virtual ServerItems GetItems() = 0;
};

extern "C" OPC_CLIENT_DLL_API ServerNames;
extern "C" OPC_CLIENT_DLL_API OPCClientInterface *instance_create();
extern "C" OPC_CLIENT_DLL_API void instance_destroy(OPCClientInterface *instance);
