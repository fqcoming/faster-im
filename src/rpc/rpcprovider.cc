
#include <iostream>

#include "rpcprovider.h"
#include "rpcheader.pb.h"
#include "zookeeperutil.h"




void RpcProvider::NotifyService(google::protobuf::Service *service)
{
    const google::protobuf::ServiceDescriptor *pserviceDesc = service->GetDescriptor();
    std::string service_name = pserviceDesc->name();
    int methodNum = pserviceDesc->method_count();

    std::cout << "service_name: " << service_name << std::endl;
    std::cout << "methodnumber: " << methodNum    << std::endl;

    ServiceInfo service_info;
    for (int i = 0; i < methodNum; ++i)
    {
        const google::protobuf::MethodDescriptor* pmethodDesc = pserviceDesc->method(i);
        std::string method_name = pmethodDesc->name();
        service_info._methodMap.insert({method_name, pmethodDesc});
    }

    service_info._service = service;
    _serviceMap.insert({service_name, service_info});
}




void RpcProvider::Run(std::string serverIp, uint16_t serverPort, std::string zkIp, uint16_t zkPort)
{

    /* step 1: The first step in starting a service is to establish a session with the registry 
    and create a service node in the registry. */

    _zkClient.Start(zkIp, zkPort);

    // Add service name as permanent nodes 
    // and set method name as temporary nodes

    for (auto &sp : _serviceMap) 
    {
        // /service_name: /UserServiceRpc
        std::string service_path = "/" + sp.first;
        _zkClient.Create(service_path.c_str(), nullptr, 0);

        for (auto &mp : sp.second._methodMap)
        {
            // /service_name/method_name: /UserServiceRpc/Login
            // Store the IP and port of the current rpc service node host.

            std::string method_path = service_path + "/" + mp.first;
            char method_path_data[128] = {0};
            sprintf(method_path_data, "%s:%d", serverIp.c_str(), serverPort);

            // ZOO_EPHEMERAL indicates that znode is a temporary node.
            _zkClient.Create(method_path.c_str(), method_path_data, strlen(method_path_data), ZOO_EPHEMERAL);
        }
    }

    /* step 2: The second part of starting the service is to start the host TCP server. */

    _tcpServer->init();         // Start the worker thread for processing lock-free queue message event.
    _tcpServer->startLoop();   // Start the main thread for processing network IO events.
}


// 新的socket连接回调
void RpcProvider::OnConnection(const muduo::net::TcpConnectionPtr &conn)
{
    if (!conn->connected())
    {
        // 和rpc client的连接断开了
        conn->shutdown();
    }
}







// header_size(4 bytes) + header_str(service_name, method_name, args_size) + args_str

void RpcProvider::OnMessage(faster_event_t* event)
{
    std::string recv_buf(event->evbuf, event->size);  // deep copy
    memset(event->evbuf, 0, event->capacity);

    uint32_t header_size = 0;
    recv_buf.copy((char*)&header_size, 4, 0);

    std::string rpc_header_str = recv_buf.substr(4, header_size);

    faster::RpcHeader rpcHeader;

    std::string service_name;
    std::string method_name;
    uint32_t args_size;

    if (rpcHeader.ParseFromString(rpc_header_str)) {

        // data header deserialization successful
        service_name = rpcHeader.service_name();
        method_name = rpcHeader.method_name();
        args_size = rpcHeader.args_size();

    } else {

        // data header deserialization failed
        std::cout << "rpc_header_str:" << rpc_header_str << " parse error!" << std::endl;
        return;
    }


    // Obtain character stream data for rpc method parameters
    std::string args_str = recv_buf.substr(4 + header_size, args_size);


    std::cout << "============================================" << std::endl;
    std::cout << "header_size:    " << header_size    << std::endl; 
    std::cout << "rpc_header_str: " << rpc_header_str << std::endl; 
    std::cout << "service_name:   " << service_name   << std::endl; 
    std::cout << "method_name:    " << method_name    << std::endl; 
    std::cout << "args_str:       " << args_str       << std::endl; 
    std::cout << "============================================" << std::endl;


    auto service_iter = _serviceMap.find(service_name);
    if (service_iter == _serviceMap.end()) {
        std::cout << service_name << " is not exist!" << std::endl;
        return;
    }

    auto method_iter = service_iter->second._methodMap.find(method_name);
    if (method_iter == service_iter->second._methodMap.end()) {
        std::cout << service_name << ":" << method_name << " is not exist!" << std::endl;
        return;
    }

    google::protobuf::Service *service = service_iter->second._service;     // Get service object: new UserService
    const google::protobuf::MethodDescriptor *method = method_iter->second; // Get method object: Login


    // Generate request and response parameters for rpc method calls
    google::protobuf::Message *request = service->GetRequestPrototype(method).New();
    if (!request->ParseFromString(args_str)) {
        std::cout << "request parse error, content:" << args_str << std::endl;
        return;
    }


    google::protobuf::Message *response = service->GetResponsePrototype(method).New();

    // Bind a Closure callback function to the following CallMethod.
    // SendRpcResponse: Mainly used to send rpc responses
    google::protobuf::Closure *done = 
        google::protobuf::NewCallback<RpcProvider, faster_event_t*, google::protobuf::Message*>
            (this, &RpcProvider::SendRpcResponse, event, response);

    // new UserService().Login(controller, request, response, done)
    service->CallMethod(method, nullptr, request, response, done);
}




void RpcProvider::SendRpcResponse(faster_event_t* event, 
                                google::protobuf::Message *response) {
    std::string response_str;
    if (response->SerializeToString(&response_str)) {

        memcpy(event->evbuf, &response_str, response_str.size());
        event->size = response_str.size();
        _tcpServer->sendMessage(event);

    } else {
        std::cout << "serialize response_str error!" << std::endl; 
    }
}