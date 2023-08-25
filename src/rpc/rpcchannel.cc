
#include <string>
#include "rpcheader.pb.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <errno.h>

#include "rpcchannel.h"
#include "rpccontroller.h"
#include "zookeeperutil.h"





FasterRpcChannel::FasterRpcChannel(std::string zkIp, uint16_t zkPort) : _zkIp(zkIp), _zkPort(zkPort) {
}



// rpc 协议格式:
// header_size + (service_name + method_name + args_size) + args

void FasterRpcChannel::CallMethod(const google::protobuf::MethodDescriptor* method,
                                google::protobuf::RpcController* controller, 
                                const google::protobuf::Message* request,
                                google::protobuf::Message* response,
                                google::protobuf:: Closure* done)
{
    const google::protobuf::ServiceDescriptor* sd = method->service();
    std::string service_name = sd->name(); 
    std::string method_name = method->name();

    // 获取参数的序列化字符串长度 args_size
    uint32_t args_size = 0;
    std::string args_str;
    if (request->SerializeToString(&args_str)) {
        args_size = args_str.size();
    } else {
        controller->SetFailed("serialize request error!");
        return;
    }
    
    // 定义rpc的请求header
    mprpc::RpcHeader rpcHeader;
    rpcHeader.set_service_name(service_name);
    rpcHeader.set_method_name(method_name);
    rpcHeader.set_args_size(args_size);

    uint32_t header_size = 0;
    std::string rpc_header_str;
    if (rpcHeader.SerializeToString(&rpc_header_str)) {
        header_size = rpc_header_str.size();
    } else {
        controller->SetFailed("serialize rpc header error!");
        return;
    }

    // 组织待发送的rpc请求的字符串
    std::string send_rpc_str;
    send_rpc_str.insert(0, std::string((char*)&header_size, 4)); // header_size
    send_rpc_str += rpc_header_str; // rpcheader
    send_rpc_str += args_str; // args


    // 打印调试信息
    std::cout << "\n============================================" << std::endl;
    std::cout << "call service_name: " << service_name << std::endl; 
    std::cout << "call method_name : " << method_name  << std::endl; 
    std::cout << "============================================\n" << std::endl;


    int clientfd = startConnWithProvider(service_name, method_name);
    if (clientfd == -1) {
        controller->SetFailed("Failed to open connection with service provider!");
        return;
    }


    // 发送rpc请求
    if (-1 == send(clientfd, send_rpc_str.c_str(), send_rpc_str.size(), 0))
    {
        endConnWithProvider(service_name, method_name);
        char errtxt[512] = {0};
        sprintf(errtxt, "send error! errno: %d", errno);
        controller->SetFailed(errtxt);
        return;
    }


    // 接收rpc请求的响应值
    char recv_buf[1024] = {0};
    int recv_size = 0;
    if (-1 == (recv_size = recv(clientfd, recv_buf, 1024, 0)))
    {
        endConnWithProvider(clientfd);
        char errtxt[512] = {0};
        sprintf(errtxt, "recv error! errno:%d", errno);
        controller->SetFailed(errtxt);
        return;
    }

    // 反序列化rpc调用的响应数据
    if (!response->ParseFromArray(recv_buf, recv_size))
    {
        endConnWithProvider(clientfd);
        char errtxt[512] = {0};
        sprintf(errtxt, "parse error! response_str:%s", recv_buf);
        controller->SetFailed(errtxt);
        return;
    }

}



int FasterRpcChannel::startConnWithProvider(std::string service_name, std::string method_name) {

    if (_serviceToConn.find(service_name) != _serviceToConn.end()) {
        if (_serviceToConn[service_name] != -1) {
            return _serviceToConn[service_name];
        }
    }

    // 先与zk建立短连接，与服务端建立长连接，后面若需要在改成长连接
    ZkClient zkCli;
    zkCli.Start();


    //  /UserServiceRpc/Login
    std::string method_path = "/" + service_name + "/" + method_name;
    // 127.0.0.1:8000
    std::string host_data = zkCli.GetData(method_path.c_str());


    if (host_data == "") {
        std::cout << method_path << " is not exist!" << std::endl;
        return -1;
    }
    int idx = host_data.find(":");
    if (idx == -1) {
        std::cout << method_path << " address is invalid!" << std::endl;
        return;
    }


    std::string ip = host_data.substr(0, idx);
    uint16_t port = atoi(host_data.substr(idx+1, host_data.size()-idx).c_str()); 

    int clientfd = socket(AF_INET, SOCK_STREAM, 0);
    if (-1 == clientfd) {
        printf("create socket error! errno: %d\n", errno);
        return -1;
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(ip.c_str());


    // 连接rpc服务节点
    if (-1 == connect(clientfd, (struct sockaddr*)&server_addr, sizeof(server_addr)))
    {
        close(clientfd);
        printf("connect error! errno: %d\n", errno);
        return;
    }

    _serviceToConn[service_name] = clientfd;
    return clientfd;
}




void FasterRpcChannel::endConnWithProvider(std::string service_name, int clientfd) {

    if (_serviceToConn.find(service_name) != _serviceToConn.end()) {
        _serviceToConn.erase(service_name);
    }

    if (clientfd != -1) {
        close(clientfd);
    }
}







