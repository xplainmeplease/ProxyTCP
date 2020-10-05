#ifndef PROXYTCP_PROXY_HPP
#define PROXYTCP_PROXY_HPP

#include "Status.hpp"
#include "Offsets.hpp"
#include "Ports.hpp"

#include <cstdio>
#include <cstdlib>
#include <csignal>
#include <cstring>
#include <cerrno>
#include <fcntl.h>
#include <getopt.h>
#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <net/ethernet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netinet/ip.h>
#include <unistd.h>


namespace Proxy
{
    enum class InputArgs : int32_t
    {
        Mode = 0,
        Domain = 1,
        ListeningPort = 2,
        DestinationPort = 3,
        BannedDomain = 4,
    };

    enum class Mode : int32_t
    {
        Forwarding = 0,
        Tracking = 1,
        Ban = 2,
    };

    struct ForwardingData
    {
        int32_t listeningPort{}; // what  to listen
        int32_t destinationPort{}; // where to forward
        std::string hostName = "coolsite.io";
        std::string bannedHostName = "badsite.io";
    };

    // function pointer that used to chose the function that depends on --mode arg.
    typedef void (* ModeFunctionPointer)(const ForwardingData& fwd);

    // socket that will forward data to the destination
    Status CreateSocketForForwarding(int32_t& socketForForwarding, int32_t destinationPort, const char* hostName) noexcept;
    Status CreateSocketOnListeningPort(int32_t& listeningSocket, int32_t listeningPort, sockaddr_in& socketData) noexcept;
    Status TransferData(int32_t sourceSocket, int32_t destinationSocket) noexcept;

    void ForwardingMode(const ForwardingData& fwd) noexcept;
    void PrintStatusAndExit(const Status& status) noexcept;
    void PrintRecievedData(const char* buffer, uint32_t size) noexcept;

}

namespace Proxy::Tracking
{
    void TrackingMode(const ForwardingData& fwd) noexcept;

    bool IsClientHelloMesasge(const char* buff, int32_t offset = 0) noexcept;

    std::string GetDomainNameFromTCPPacket(const char* buffer, uint32_t offset = 0) noexcept;
}

namespace Proxy::Ban
{
    void BanMode(const ForwardingData& fwd) noexcept;

    Status TransferDataWithRestriction(int32_t listeningSocket, const std::string& bannedHostname, int32_t destinationPort) noexcept;
}

#endif // PROXYTCP_PROXY_HPP