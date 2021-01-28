#ifndef PROXYTCP_CONSTANTS_HPP
#define PROXYTCP_CONSTANTS_HPP

#include <cstdint>

namespace Proxy::HeaderSize
{
    const uint8_t ETH = 14;
    const uint8_t IP  = 20;
    const uint8_t TCP = 32;
    const uint8_t TLS_RECORD = 5;
    const uint8_t TLS_MESSAGE = 4;

} // namespace Proxy::HeaderSize

namespace Proxy::Utilities::Offsets
{
    namespace TLS
    {

        const uint32_t TLS_DATA = 66; // start point of the tls data;
        const uint32_t RECORD_TYPE = 66;
        const uint32_t MESSAGE_TYPE = 71;
        const uint32_t EXTENTIONS_DATA_SIZE = 178;
        const uint32_t EXTENTIONS_DATA = 182;
        const uint32_t MESSAGE_SIZE = 1;
        const uint8_t  SESSION_ID_LENGTH = 109 - 66;
        namespace SNI
        {
            const uint32_t SERVER_NAME_LIST_LENGTH = 0;
            const uint32_t SERVER_NAME_TYPE = 2;
            const uint32_t SERVER_NAME_LENGTH = 3;
            const uint32_t SERVER_NAME = 5;
        }

    } // namespace Offsets::TLS_RECORD

    namespace TCP
    {

        const uint32_t TCP_DATA = 34;

    } // namespace Offsets::TCP

    namespace IP
    {

        const uint32_t IP_DATA = 14;

    } // namespace Offsets::IP

    namespace ETH
    {

    }// namespace Offsets::ETH

    namespace SOCKS5
    {
        namespace Handshake
        {
            namespace Client
            {
                const uint8_t IPv4_ADDRESS = 4;
                const uint8_t IPv6_ADDRESS = 16;
                const uint8_t DOMAIN_NAME = 5;
                const uint8_t DOMAIN_NAME_SIZE = 4;
            }  // namespace Client

            namespace Server
            {

            } // namespace Server

        } // namespace Handshake

    } // namespace Offsets::SOCKS5

} // namespace Proxy::Offsets

namespace Proxy::Utilities::TLS::ContentType
{
    const uint8_t Handshake = 0x16;
    const uint8_t ApplicationData = 0x17;
    const uint8_t ChangeCipherSpec = 0x14;
}


namespace Proxy::Utilities::Ports
{

    const int32_t HTTP = 80;
    const int32_t HTTPS = 443;

} // namespace Proxy::Utilities::Ports


#endif //PROXYTCP_CONSTANTS_HPP