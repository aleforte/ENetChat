#pragma once
#include <cstdint>
#include <map>

#include "network/address.h"
#include "util/byte_stream.h"

typedef uint16_t user_id_t;

/**
 * Chat User Info
 */
struct UserInfo
{
    user_id_t user_id;    ///< application-specific user ID
    std::string name;     ///< Nickname
    net::Address address; ///< Address (server only, not shared w/ clients for security reasons)
    
    UserInfo(net::Address addr = net::Address())
        : user_id(-1), address(addr),  name(name) {}

    UserInfo(user_id_t user_id, net::Address addr = net::Address())
        : user_id(user_id), address(addr) {}
        
    UserInfo(user_id_t user_id, const std::string& name, net::Address addr = net::Address())
        : user_id(user_id), address(addr),  name(name) {}

    void serialize(ByteStream& s) const
    {
        s.writeUInt16(user_id);
        s.writeString(name, name.length());
    }
}; 

typedef std::map<user_id_t, UserInfo> UserMap;
