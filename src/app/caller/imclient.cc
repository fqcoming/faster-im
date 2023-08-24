
#include "rpcchannel.h"
#include "calluserservice.h"












int main(int argc, char *argv[]) {

    UserServiceCaller caller(new FasterRpcChannel());
    caller.createConn();

    


    return 0;
}








