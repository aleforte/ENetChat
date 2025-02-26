#pragma once
#include <utility>
#include <vector>

#include "util/byte_stream.h"

namespace protocol
{
    const unsigned DEFAULT_PORT = 7777;

    /**
     * @brief Contains all possible message types
     */
    enum PacketType : int8_t
    {
        USERNAME = 0,       // First packet from client containing username [client -> server]
        USERNAME_ACK = 1,   // Ack packet to client w/ current chat state, i.e. list of connected users [server -> client]
        STATE_ADD_USER = 2, // Chat state delta, new user added to chat
        STATE_REM_USER = 3, // Chat state delta, user removed from chat
        MESSAGE = 4,        // Chat message
    };
    
    struct Package
    {
        int8_t packet_type;
    
        Package(int8_t packet_type) : packet_type(packet_type) {}
    
        virtual void serialize(ByteStream& s) const
        {
            s.writeInt8(packet_type);
        }
    };

    // [client -> server]
    // Provides a client's username to the server
    struct UsernamePackage : Package
    {
        std::string username;

        UsernamePackage(std::string username)
            : Package(USERNAME), username(std::move(username)) {}

        UsernamePackage(ByteStream& s) : Package(s.readInt8())
        {
            username = s.readString();
        }

        void serialize(ByteStream& s) const override
        {
            Package::serialize(s);
            s.writeString(username, username.length());
        }
    };

    // [server -> client]
    // Provides a client their assigned User ID and current session state
    struct UsernameAckPackage : Package
    {
        user_id_t assigned_user_id;
        std::vector<UserInfo> users;

        UsernameAckPackage(user_id_t user_id, const UserMap& user_map)
            : Package(USERNAME_ACK), assigned_user_id(user_id)
        {
            for (const auto& pair : user_map) users.push_back(pair.second);
        }
        
        UsernameAckPackage(ByteStream& s)
            : Package(s.readInt8())
        {
            assigned_user_id = s.readUInt16();
            while (!s.end())
            {
                user_id_t id = s.readUInt16();
                UserInfo user(id, s.readString());
                users.push_back(user);
            }
        }

        void serialize(ByteStream& s) const override
        {
            Package::serialize(s);
            s.writeUInt16(assigned_user_id);
            for (UserInfo user : users) user.serialize(s);
        }
    };

    // [server -> client]
    // Notifies clients to add a new user to their user list/map
    struct AddUserPackage : Package
    {
        UserInfo user;

        AddUserPackage(UserInfo user)
            : Package(STATE_ADD_USER), user(std::move(user)) {}

        AddUserPackage(ByteStream& s)
            : Package(s.readInt8()), user()
        {
            user.user_id = s.readUInt16();
            user.name = s.readString();
        }

        void serialize(ByteStream& s) const override
        {
            Package::serialize(s);
            user.serialize(s); 
        }
    };

    // [server -> client]
    // Notifies clients to remove a new user to their user list/map
    struct RemoveUserPackage : Package
    {
        user_id_t user_id;

        RemoveUserPackage(user_id_t user_id)
            : Package(STATE_REM_USER), user_id(user_id) {}
        
        RemoveUserPackage(ByteStream& s)
            : Package(s.readInt8())
        {
            user_id = s.readUInt16();
        }

        void serialize(ByteStream& s) const override
        {
            Package::serialize(s);
            s.writeUInt16(user_id);
        }
    };

    // [client -> server, server -> client]
    // A new message to be posted in the chat
    struct MessagePackage : Package
    {
        user_id_t user_id;
        std::string message;

        MessagePackage(user_id_t user_id, const std::string& msg)
            : Package(MESSAGE), user_id(user_id), message(msg) {}

        MessagePackage(ByteStream& s)
            : Package(s.readInt8())
        {
            user_id = s.readUInt16();
            message = s.readString();
        }

        void serialize(ByteStream& s) const override
        {
            Package::serialize(s);
            s.writeUInt16(user_id);
            s.writeString(message, message.length());
        }
    };
}
