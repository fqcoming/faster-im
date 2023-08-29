
#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <errno.h>
#include <sys/epoll.h>
#include <thread>

#include "pb/rpcheader.pb.h"
#include "pb/user.pb.h"
#include "rpcchannel.h"
#include "rpccontroller.h"
#include "zk/zookeeperutil.h"



FasterRpcChannel::FasterRpcChannel(std::string zkIp, uint16_t zkPort) : _zkIp(zkIp), _zkPort(zkPort) {

    sem_init(&_semResponse, 0, 0);
    startRecvThread();
}


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

    // Obtain the serialized string length of the parameter: args_size
    uint32_t args_size = 0;
    std::string args_str;
    if (request->SerializeToString(&args_str)) {
        args_size = args_str.size();
    } else {
        controller->SetFailed("serialize request error!");
        return;
    }
    

    faster::RpcHeader rpcHeader;
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

    // The string of rpc requests to be sent by the organization
    std::string send_rpc_str;
    send_rpc_str.insert(0, std::string((char*)&header_size, 4)); // header_size
    send_rpc_str += rpc_header_str;                              // rpcheader
    send_rpc_str += args_str;                                    // args


    std::cout << "\n============================================" << std::endl;
    std::cout << "call service_name: " << service_name << std::endl; 
    std::cout << "call method_name : " << method_name  << std::endl; 
    std::cout << "============================================\n" << std::endl;


    int clientfd = openConn(service_name, method_name);
    if (clientfd == -1) {
        controller->SetFailed("Failed to open connection with service provider!");
        return;
    }

    // Send rpc request
    memset(_recvBuf, 0, 1024);
    _recvSize = 0;
    if (-1 == send(clientfd, send_rpc_str.c_str(), send_rpc_str.size(), 0)) {
        closeConn(service_name, method_name);
        char errtxt[512] = {0};
        sprintf(errtxt, "send error! errno: %d", errno);
        controller->SetFailed(errtxt);
        return;
    }

    // Wait for the semaphore, and after the sub thread receiving the network packet processes 
    // the response message of the rpc, notify here
    sem_wait(&_semResponse);

    // Received response for rpc request
    // Deserialize response
    if (!response->ParseFromArray(_responseBuf, _responseSize)) {
        closeConn(service_name, method_name);
        char errtxt[2048] = {0};
        sprintf(errtxt, "parse error! response_str: %s", _recvBuf);
        controller->SetFailed(errtxt);
        return;
    }
}



int FasterRpcChannel::openConn(std::string service_name, std::string method_name) {

    //  /UserServiceRpc/Login
    std::string method_path = "/" + service_name + "/" + method_name;

    if (_serviceToConn.find(method_path) != _serviceToConn.end()) {
        if (_serviceToConn[method_path] != -1) {
            return _serviceToConn[method_path];
        }
    }

    ZkClient zkCli;
    zkCli.Start(_zkIp, _zkPort);

    // 127.0.0.1:8000
    std::string host_data = zkCli.GetData(method_path.c_str());

    if (host_data == "") {
        std::cout << method_path << " is not exist!" << std::endl;
        return -1;
    }

    int idx = host_data.find(":");
    if (idx == -1) {
        std::cout << method_path << " address is invalid!" << std::endl;
        return -1;
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

    if (-1 == connect(clientfd, (struct sockaddr*)&server_addr, sizeof(server_addr)))
    {
        close(clientfd);
        printf("connect error! errno: %d\n", errno);
        return -1;
    }

    _serviceToConn[method_path] = clientfd;
    _connToService[clientfd] = method_path;

    return clientfd;
}




void FasterRpcChannel::closeConn(std::string service_name, std::string method_name) {

    //  /UserServiceRpc/Login
    std::string method_path = "/" + service_name + "/" + method_name;

    if (_serviceToConn.find(method_path) != _serviceToConn.end()) {
        int fd = _serviceToConn[method_path];
        _connToService.erase(fd);
        _serviceToConn.erase(method_path);
        close(fd);
    }
}



void FasterRpcChannel::startRecvThread() {
    // Successfully connected to the server, starting the receive sub thread
    std::thread readTask(std::bind(&FasterRpcChannel::readTaskThreadFunc, this));  
    readTask.detach();                                   
}




void FasterRpcChannel::readTaskThreadFunc() {

    _epfd = epoll_create(1);
    struct epoll_event ev, events[EVENTS_LENGTH];

    for (;;) {
        int nready = epoll_wait(_epfd, events, EVENTS_LENGTH, -1); // -1, ms 
		int i = 0;

		for (i = 0; i < nready; i++) {
			int clientfd = events[i].data.fd;

            // For the client, if listening is not enabled, no listening events should occur.
			if (events[i].events & EPOLLIN) {   //clientfd

                memset(_recvBuf, 0, BUFFER_LENGTH);
				int n = recv(clientfd, _recvBuf, BUFFER_LENGTH - 1, 0);
				if (n > 0) {

                    _recvSize = n;

                    // Respond differently based on different message types
                    procRecvMessage(std::string(_recvBuf, n));
					
				} else if (n == 0) {

					ev.events = EPOLLIN;
					ev.data.fd = clientfd;
					epoll_ctl(_epfd, EPOLL_CTL_DEL, clientfd, &ev);
                    std::string method_path = _connToService[clientfd];
                    _serviceToConn.erase(method_path);
                    _connToService.erase(clientfd);
					close(clientfd);

				} else {
                    std::cout << "epoll loop read failed." << std::endl;
                }
				
			} else if (events[i].events & EPOLLOUT) {
                // For the client, the send data operation calls send() in the main thread.
			}
		} // for
    } // for 
}



void FasterRpcChannel::procRecvMessage(std::string message) {

    uint32_t header_size = 0;
    message.copy((char*)&header_size, sizeof(uint32_t), 0);

    std::string response_header_str = message.substr(sizeof(u_int32_t), header_size);
    faster::RpcResponseHeader responceHeader;

    uint32_t data_size;
    faster::RpcResponseHeader_Type type;
    if (responceHeader.ParseFromString(response_header_str)) {

        // Data header deserialization successful.
        type = responceHeader.type();
        data_size = responceHeader.data_size();

    } else {

        // Data header deserialization failed
        std::cout << "response_header_str parse error!" << std::endl;
        return;
        
    }

    std::string data_str = message.substr(sizeof(uint32_t) + header_size, data_size);

    if (type == faster::RpcResponseHeader_Type::RpcResponseHeader_Type_RESPONSE) {

        memset(_responseBuf, 0, BUFFER_LENGTH);
        memcpy(_responseBuf, _recvBuf, _recvSize);
        _responseSize = _recvSize;

        sem_post(&_semResponse);  // Notify Main Thread

    } else if (type == faster::RpcResponseHeader_Type::RpcResponseHeader_Type_PUSHMSGS) {

        std::cout << "recv: " << data_str << std::endl;

    } else {
        std::cout << "Received message with undefined behavior." << std::endl;
    }

}










