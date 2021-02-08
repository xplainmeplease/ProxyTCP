#ifndef PROXYTCP_CLIENTHELLOTRANSMISSION_HPP
#define PROXYTCP_CLIENTHELLOTRANSMISSION_HPP

#include "../ConversationFlow.hpp"
#include "ConnectionRequestTransmission.hpp"
#include <memory>

#include "Connection/ClientConnection.hpp"
#include "Connection/ServerConnection.hpp"

namespace Proxy::SOCKS5Flow
{
    class ClientHelloTransmission : public ConversationFlow
    {
    public:
        std::unique_ptr<ConversationFlow>
        PerformTransaction(ClientConnection& clientConnection, ServerConnection& serverConnection, int32_t epollfd, int32_t sockfdWithEvent) noexcept override;

    private:
        static ByteStream GenerateClientInitiationResponse() noexcept;
    };
}

#endif //PROXYTCP_CLIENTHELLOTRANSMISSION_HPP
