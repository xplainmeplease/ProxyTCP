
#include "Proxy.hpp"

namespace Proxy
{

    Status CreateSocketOnListeningPort(int32_t &listeningSocket, int32_t listeningPort, sockaddr_in &socketData) noexcept
    {
        Status status {};

        listeningSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if(listeningSocket == -1)
        {
            status = Status(Status::Error::BadListeningSocketInitializaton);
            return status;
        }

        memset(reinterpret_cast<char*>(&socketData),0,sizeof(socketData));

        socketData.sin_family = AF_INET;
        socketData.sin_addr.s_addr = INADDR_ANY;
        socketData.sin_port = htons(listeningPort);

        if(bind(listeningSocket, reinterpret_cast<sockaddr*>(&socketData), sizeof(socketData)) == -1)
        {
            status = Status(Status::Error::BadBindListeningPortToSocket);
            return status;
        }

        if(listen(listeningSocket, 5) == -1)
        {
            status = Status(Status::Error::MarkSocketPassive);
            return status;
        }

        return status;
    }

    Status TransferData(int32_t sourceSocket, int32_t destinationSocket) noexcept
    {
        Status status {};

        const int32_t BUFFER_SIZE = 4096;
        char buffer[BUFFER_SIZE];
        uint32_t totalBytesWritten {};
        int32_t bytesWritten {};
        int32_t bytesRead {};



        while((bytesRead = read(sourceSocket, buffer, BUFFER_SIZE)) > 0)
        {
            totalBytesWritten = 0;
            while(totalBytesWritten < bytesRead)
            {
                int32_t writingSize = bytesRead - totalBytesWritten;
                bytesWritten = write(destinationSocket, buffer + totalBytesWritten, writingSize );

                if(bytesWritten == - 1)
                {
                    status = Status(Status::Error::BadDataWrittenOnForwarding);
                    return status;
                }

                totalBytesWritten += bytesWritten;
            }
        }

        if(bytesRead == -1)
        {
            status = Status(Status::Error::NoDataReadFromSocket);
        }

        shutdown(sourceSocket, SHUT_RD);
        shutdown(destinationSocket, SHUT_WR);

        close(sourceSocket);
        close(destinationSocket);

        return status;

    }

    void PrintStatusAndTerminateProcess(const Status &status) noexcept
    {
        std::cout << "[Status code: " << status.Code() << " | " << strerror(errno) << "]\n";
        exit(status.Code());
    }

    Status CreateSocketForForwarding(int32_t &socketForForwarding, int32_t destinationPort, const char *hostName) noexcept
    {
        Status status {};

        hostent* destinationHost = nullptr;
        sockaddr_in destinationAddress {};

        destinationHost = gethostbyname(hostName);

        if(destinationHost == nullptr)
        {
            status =  Status( Status::Error::BadDestinationHost);
            PrintStatusAndTerminateProcess(status);
        }

        memset(&destinationAddress, 0, sizeof(destinationAddress));

        destinationAddress.sin_family = AF_INET;

        memcpy(reinterpret_cast<char*>(&destinationAddress.sin_addr.s_addr),
               reinterpret_cast<char*>(destinationHost->h_addr),
               destinationHost->h_length);

        destinationAddress.sin_port = htons(destinationPort);

        socketForForwarding = socket(AF_INET, SOCK_STREAM, 0);

        if(socketForForwarding == -1)
        {
            status =  Status( Status::Error::BadForwardingSocketCreation);
            PrintStatusAndTerminateProcess(status);
        }

        auto connectResult = connect(socketForForwarding, reinterpret_cast<sockaddr*>(&destinationAddress), sizeof(destinationAddress));

        if(connectResult == -1)
        {
            status =  Status( Status::Error::BadConnectionSocketToAddress);
            PrintStatusAndTerminateProcess(status);
        }

        return status;
    }

    void ForwardingMode(const ConnectionInfo &info) noexcept
    {
        std::cout << "[Forwarding mode]\n";

        Status status {};
        sockaddr_in socketData {};

        int32_t listeningSocket {};

        status =  CreateSocketOnListeningPort(listeningSocket, info.listeningPort, socketData);
        if(status.Failed()) { PrintStatusAndTerminateProcess(status); }

        bool waitingForConnection { true };
        while(waitingForConnection)
        {
            int32_t destinationSocket {};
            pid_t parentPID {};

            destinationSocket = accept(listeningSocket, nullptr , nullptr);
            if(destinationSocket == -1)
            {
                status =  Status( Status::Error::BadConnectionFromListeningSocket);
                PrintStatusAndTerminateProcess(status);
            }

            parentPID = fork();
            if(parentPID == -1)
            {
                status =  Status( Status::Error::BadProcessFork);
                PrintStatusAndTerminateProcess(status);
            }

            if(parentPID == 0)
            {
                int32_t forwardingSocket {};
                pid_t childPID {};

                status =  CreateSocketForForwarding( forwardingSocket, info.destinationPort, info.hostName.c_str());

                // child process forward traffic to the client,
                // parent process forward traffic to the destination[port]
                childPID = fork();
                if(childPID == -1)
                {
                    status =  Status( Status::Error::BadProcessFork);
                    PrintStatusAndTerminateProcess(status);
                }

                if(childPID == 0)
                {
                    std::cout << "[Transfering data from port " << info.listeningPort << " to port " << info.destinationPort << "]\n";
                    status =  TransferData(forwardingSocket, destinationSocket);
                    if(status.Failed()) { PrintStatusAndTerminateProcess(status); }
                }
                else
                {
                    std::cout << "[Transfering data from port " << info.destinationPort << " to port " << info.listeningPort << "]\n";
                    status =  TransferData(destinationSocket, forwardingSocket);
                    if(status.Failed()) { PrintStatusAndTerminateProcess(status); }
                }

                exit(static_cast<int32_t>( Status::Success::Success));
            }

            close(destinationSocket);
        }

    }

    void PrintRecievedData(const char *buffer, uint32_t size) noexcept
    {
        printf("\n\n=============\n\n");
        for(int32_t i = 0; i < size; ++i)
        {
            if( (i%8 == 0) && (i%16 !=0) )
                printf("\t");

            if(i%16 == 0)
                printf("\n");

            printf("%02x ", static_cast<uint8_t>(buffer[i]));
        }
        printf("\n\n=============\n\n");
    }

} // namespace Proxy

namespace  Proxy::Tracking
{
    void TrackingMode(const ConnectionInfo &info) noexcept
    {
        std::cout << "[Tracking mode]\n";

        Status status {};
        sockaddr_in socketData {};

        int32_t socketDataSize = sizeof(socketData);
        int32_t listeningSocket {};

        int32_t recievedData {};

        const int32_t BUFFER_SIZE = 4096;
        char  buffer [BUFFER_SIZE];

        status = CreateSocketOnListeningPort(listeningSocket, info.listeningPort, socketData);
        if(status.Failed()) { PrintStatusAndTerminateProcess(status); }

        bool waitingForConnection {true};
        while(waitingForConnection)
        {
            int32_t newConnectionSocket {};

            newConnectionSocket = accept(listeningSocket, nullptr, nullptr);

            if(newConnectionSocket == -1)
            {
                status = Status(Status::Error::BadConnectionFromListeningSocket);
                PrintStatusAndTerminateProcess(status);
            }
            std::cout << "new connection found.\n";

            while((recievedData = read(newConnectionSocket,buffer,BUFFER_SIZE)) > 0)
            { close(newConnectionSocket);
                PrintRecievedData(buffer,75);

                if(IsClientHelloMesasge(buffer, Offsets::TLS::TLS_DATA))
                {
                    std::string domain = GetDomainNameFromTCPPacket(buffer, Offsets::TLS::TLS_DATA);
                    std::cout << "You have been connected with: " << domain << "\n";
                }
            }

            if(recievedData == -1)
            {
                status = Status(Status::Error::BadRecievingDataFromSocket);
                PrintStatusAndTerminateProcess(status);
            }

        }
    }

    bool IsClientHelloMesasge(const char* buff, int32_t offset ) noexcept
    {
        // offset is used for compatibility with raw sockets when they need.

        // buff[66] - position of the TLS Content Type field. [0x16 - Handshake]/[0x17 - Application Data]
        // buff[71] - position of the Handshake Type [0x01 - ClienHello]/[0x02 - ServerHello]
        return ( static_cast<uint32_t>(buff[Offsets::TLS::MESSAGE_TYPE - offset]) == 0x16 ) &&
               ( static_cast<uint32_t>(buff[Offsets::TLS::HANDSHAKE_TYPE - offset]) == 0x01 );
    }

    std::string GetDomainNameFromTCPPacket(const char* buffer, uint32_t offset) noexcept
    {
        auto domainNameSize = static_cast<uint32_t>(buffer[Offsets::TLS::SNI_SIZE - offset]);
        auto domainName = new char[domainNameSize];

        memcpy(domainName,buffer + (Offsets::TLS::SNI - offset), domainNameSize);

        std::string tmp(domainName);
        delete[] domainName;

        return tmp;
    }

} //namespace  Proxy::Tracking

namespace  Proxy::Ban
{
    void BanMode(const ConnectionInfo &info) noexcept
    {
        std::cout << "[Ban mode]\n";

        Status status {};
        sockaddr_in socketData {};

        int32_t listeningSocket {};
//        int32_t newConnectionSocket {};

        pid_t parentPID;

        std::vector<std::thread> threads;
        // listening on 8081
        status =  CreateSocketOnListeningPort(listeningSocket, info.listeningPort, socketData);
        if(status.Failed()) { PrintStatusAndTerminateProcess(status); }

        bool waitingForConnection { true };
        while(waitingForConnection)
        {

            int32_t newConnectionSocket = accept(listeningSocket, nullptr , nullptr);
            if(newConnectionSocket == -1)
            {
                status = Status(Status::Error::BadConnectionFromListeningSocket);
                PrintStatusAndTerminateProcess(status);
            }

            std::thread newConnectionThread ([newConnectionSocket, &info = std::as_const(info)] () {

              std::cout << "[Thread " << std::this_thread::get_id() << "]" << "\t\t[" << info.listeningPort << "->" << info.destinationPort << "]\n";

              Status transferStatus = Ban::TransferDataWithRestriction(newConnectionSocket, info.bannedHostName, info.destinationPort);
              if (transferStatus.Failed() && (transferStatus.Code() != static_cast<int32_t>(Status::Error::BannedHostDataTransfer)))
              { PrintStatusAndTerminateProcess(transferStatus); }

              close(newConnectionSocket);

            });

            threads.emplace_back(std::move(newConnectionThread));


//            parentPID = fork();
//            if(parentPID == -1)
//            {
//                status = Status(Status::Error::BadProcessFork);
//                PrintStatusAndTerminateProcess(status);
//            }
//
//
//            if(parentPID == 0)
//            {
//                std::cout << "[" << info.listeningPort << "->" << info.destinationPort << "]\n";
//
//                status = Ban::TransferDataWithRestriction(newConnectionSocket, info.bannedHostName, info.destinationPort);
//                if (status.Failed() && (status.Code() != static_cast<int32_t>(Status::Error::BannedHostDataTransfer)))
//                { PrintStatusAndTerminateProcess(status); }
//
//                close(newConnectionSocket);
//            }
//            close(newConnectionSocket);
        }

    }

    Status TransferDataWithRestriction(int32_t listeningSocket, const std::string& bannedHostname, int32_t destinationPort) noexcept
    {
        const int32_t BUFFER_SIZE = 4096;
        char  buffer [BUFFER_SIZE];

        Status status {};

        int32_t destinationSocket {};
        int32_t bytesSended {};
        int32_t bytesRead {};
        uint32_t totalBytesSended {};

        bool connectionIsAllowed = false;

        std::string connectedHostDomainName;

        //recieving ALL data that come to our listenignSocket
        while( (bytesRead = recv(listeningSocket , buffer , BUFFER_SIZE , 0)) > 0 )
        {
            std::cout << "[Thread " << std::this_thread::get_id() << "]" << "\t\t[" << bytesRead << "b Client->Server]\n";
            if(Tracking::IsClientHelloMesasge(buffer, Offsets::TLS::TLS_DATA) && bytesRead > 6)
            {
                connectedHostDomainName = Tracking::GetDomainNameFromTCPPacket(buffer, Offsets::TLS::TLS_DATA);
                if( connectedHostDomainName == bannedHostname)
                {
                    std::cout << "Connection refused!\n";
                    status = Status::Success::BannedHostConnectionRefused;
                    return status;
                }

                std::cout << "[Thread " << std::this_thread::get_id() << "]" << "\t\t[ClientHello: " << connectedHostDomainName << "]\n";
                // if host is not banned - allow connection
                connectionIsAllowed = true;

                status = CreateSocketForForwarding(destinationSocket, destinationPort, connectedHostDomainName.c_str());
                if(status.Failed()) { PrintStatusAndTerminateProcess(status); }
            }

            if(connectionIsAllowed)
            {
                totalBytesSended = 0;
                while(totalBytesSended < bytesRead)
                {
                    int32_t writingSize = bytesRead - totalBytesSended;
                    bytesSended = send(destinationSocket, buffer, writingSize, 0);
                    if(bytesSended == - 1)
                    {
                        status = Status(Status::Error::BadDataWrittenOnForwarding);
                        return status;
                    }

                    totalBytesSended += bytesSended;

                    auto recievedFromServer = recv(destinationSocket, buffer, BUFFER_SIZE, 0);
                    if(recievedFromServer == -1)
                    {
                        status = Status(Status::Error::BadRecievingDataFromSocket);
                        return status;
                    }

                    std::cout << "[Thread " << std::this_thread::get_id() << "]" << "\t\t[" << recievedFromServer << "b Server->Proxy]\n";

                    auto sendedFromServer = send(listeningSocket, buffer, recievedFromServer, 0);
                    if(sendedFromServer == -1)
                    {
                        status = Status(Status::Error::BadSendingDataToServer);
                        return status;
                    }

                    std::cout << "[Thread " << std::this_thread::get_id() << "]" <<"\t\t[" << sendedFromServer << "b Proxy->Client]\n";


                }
            }
        }

        std::cout << "[All data has been sended. Closing socket]\n";

        if(bytesRead == -1)
        {
            status = Status(Status::Error::NoDataReadFromSocket);
            close(destinationSocket);
        }

//        shutdown(listeningSocket, SHUT_RD);
//
//        close(listeningSocket);
        close(destinationSocket);
        return status;
    }



} //namespace  Proxy::Ban

