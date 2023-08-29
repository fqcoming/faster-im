
#include "userserver.h"
#include "rpcprovider.h"
#include "userservice.h"




int main(int argc, char *argv[]) {

    RpcProvider provider;
    provider.NotifyService(new UserService());  
    provider.StartService("127.0.0.1", 9999, "127.0.0.1", 2181);

    return 0;
}


























