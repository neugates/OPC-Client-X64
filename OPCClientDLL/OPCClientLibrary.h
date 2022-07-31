// OPCClientLibrary.h - Contains declarations of math functions
#pragma once

#ifdef OPC_CLIENT_LIBRARY_EXPORTS
#define OPC_CLIENT_LIBRARY_API __declspec(dllexport)
#else
#define OPC_CLIENT_LIBRARY_API  __declspec(dllimport)
#endif

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

    typedef struct opc_client opc_client_t;
    typedef struct opc_host opc_host_t;

    OPC_CLIENT_LIBRARY_API void opc_client_init();
    OPC_CLIENT_LIBRARY_API void opc_client_stop();
    OPC_CLIENT_LIBRARY_API opc_host_t *opc_client_host(const char *host_name);

#ifdef __cplusplus
}
#endif
