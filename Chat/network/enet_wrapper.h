#pragma once

#include <map>
#include <mutex>
#include <enet/enet.h>

#include "address.h"

namespace net
{
    typedef enet_uint16 peer_id_t;
    
    /**
     * RAII Wrapper for ENet
     *
     * Start with creating an instance of this to initialize networking.
     * Important that it stays in scope for duration that it is needed. 
     */
    struct ENetContainer
    {
        ENetContainer() { enet_initialize(); }
        ~ENetContainer() { enet_deinitialize(); }
    };

    // Convert ENetPeer to net::Address
    inline Address convert(const ENetPeer* peer) { return peer ? Address(peer->address.host, peer->address.port) : Address(); }
    // Convert ENetAddress to net::Address
    inline Address convert(const ENetAddress& addr) { return Address(addr.host, addr.port); }

    // Network traffic wrapper
    struct NetworkTraffic
    {
        NetworkTraffic(const enet_uint8* pckt_d = NULL, size_t pckt_l = 0, enet_uint32 evdata = 0):
                peer_id(0), peer_address(), peer_data(NULL), packet_data(pckt_d),
                packet_length(pckt_l), ping(0), event_data(evdata) {}
        
        NetworkTraffic(ENetPeer* peer, void* data_ptr, const enet_uint8* pckt_d = NULL, size_t pckt_l = 0, enet_uint32 evdata = 0):
                peer_id(peer->incomingPeerID), peer_address(convert(peer->address)), peer_data(data_ptr),
                packet_data(pckt_d), packet_length(pckt_l), ping(peer->roundTripTime), event_data(evdata) {}
        
        peer_id_t peer_id; ///< Peer ID assigned by the library
        Address peer_address; ///< Address from which peer connected
        void* peer_data; ///< User data associated with the peer that sent the traffic
        const enet_uint8* packet_data; ///< The actual data packet (empty for connect/disconnect events)
        size_t packet_length; ///< Length of the packet data in bytes
        unsigned ping; ///< Average round-trip time to the peer
        enet_uint32 event_data; ///< Data associated with the event
    };

    // Convert incoming ENet data to net::NetworkTraffic
    template <typename T>
    NetworkTraffic convert(T& obj)
    {
        return NetworkTraffic(reinterpret_cast<enet_uint8*>(&obj), sizeof(T));
    }

    /**
     * @brief Callback class prototype
     *
     * Inherit this class and implement functions to get network events. These events
     *  are sent from another thread, but there will not be two simultaneous events.
     */
    class NetworkListener
    {
    public:
        // Peer connected
        // @param e the traffic associated with the event
        virtual void connectionEvent(NetworkTraffic const& e) {}
        // Peer disconnected
        // @param e the traffic associated with the event
        virtual void disconnectEvent(NetworkTraffic const& e) {}
        // Peer sent some data
        // @param e the traffic associated with the event
        virtual void receiveEvent(NetworkTraffic const& e) {}
    };
    
    class ENetWrapper
    {
    public:
        // non-copyable
        ENetWrapper(const ENetWrapper&) = delete; // non-construction-copyable
        ENetWrapper& operator=(const ENetWrapper&) = delete; // non-copyable
        
        ENetWrapper(NetworkListener& listener, bool hosting = true, int port = -1, void* data = NULL, int max_connections = 16):
                m_quit(false), m_host(NULL), m_listener(listener), m_data(data)
        {
            m_address.host = ENET_HOST_ANY;
            m_address.port = port < 0 ? ENET_PORT_ANY : port;
            m_host = enet_host_create(hosting ? &m_address : NULL, max_connections, 1, 0, 0);
            if (!m_host) throw std::runtime_error("An error occured while trying to create an ENet host.");
            // Start listener thread
            m_thread = std::jthread(&ENetWrapper::listen, std::ref(*this));
        }
        
        ~ENetWrapper()
        {
            terminate();
            std::lock_guard<std::mutex> lock(m_mutex);
            if (m_host)
            {
                ENetEvent e;
                enet_host_service(m_host, &e, 5);
                for (size_t i = 0; i < m_host->peerCount; ++i)
                {
                    enet_peer_disconnect(&m_host->peers[i], 0);
                }
                enet_host_destroy(m_host);
                m_host = nullptr;
            }
        }

        // listener thread -- DO NOT CALL DIRECTLY
        void listen()
        {
            ENetEvent e;
            while (!m_quit)
            {
                {
                    std::lock_guard<std::mutex> lock(m_mutex); // ensures no other thread can access
                    enet_host_service(m_host, &e, 0); // nonblocking
                }
                switch (e.type)
                {
                    case ENET_EVENT_TYPE_NONE: {
                            break;
                    } case ENET_EVENT_TYPE_CONNECT: {
                            m_peers[e.peer->incomingPeerID] = e.peer;
                            m_listener.connectionEvent(NetworkTraffic(e.peer, e.peer->data, NULL, 0, e.data));
                            break;
                    } case ENET_EVENT_TYPE_DISCONNECT: {
                            m_listener.disconnectEvent(NetworkTraffic(e.peer, e.peer->data, NULL, 0, e.data));
                            e.peer->data = NULL;
                            m_peers.erase(e.peer->incomingPeerID);
                            break;
                    } case ENET_EVENT_TYPE_RECEIVE: {
                            m_listener.receiveEvent(NetworkTraffic(e.peer, e.peer->data, e.packet->data, e.packet->dataLength, e.data));
                            enet_packet_destroy(e.packet);
                            break;
                    }
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(1)); // ensures events processed sequentially
            }
        }

        /**
         * Connect to a peer
         * @param host the address to connect to
         * @param port the port to connect to
         * @param data application specific data that can be retrieved in events
         */
        void connect(const std::string& host, int port, void* data = NULL)
        {
            ENetAddress address;
            enet_address_set_host(&address, host.c_str());
            address.port = port;
            // initiating connection
            ENetPeer* peer = NULL;
            std::lock_guard<std::mutex> lock(m_mutex);
            peer = enet_host_connect(m_host, &address, 0, 0);
            if (!peer) throw std::runtime_error("No available peers for initializing an ENet connection.");
            peer->data = data;
        }

        /**
         * Connect to a peer
         * @param addr the address to connect to
         * @param data application specific data that can be retrieved in events
         */
        void connect(const Address& addr, void* data = NULL)
        {
            ENetAddress address;
            address.host = addr.host;
            address.port = addr.port;
            // initiating connection
            ENetPeer* peer = NULL;
            std::lock_guard<std::mutex> lock(m_mutex);
            peer = enet_host_connect(m_host, &address, 0, 0);
            if (!peer) throw std::runtime_error("No available peers for initializing an ENet connection.");
            peer->data = data;
        }

        // Send a packet to all peers
        void broadcast(const NetworkTraffic& msg)
        {
            ENetPacket* packet = enet_packet_create(msg.packet_data, msg.packet_length, 0);
            std::lock_guard<std::mutex> lock(m_mutex);
            enet_host_broadcast(m_host, 0, packet);
        }

        // Send a string to all peers
        void broadcast(const std::string& msg)
        {
            NetworkTraffic traffic(reinterpret_cast<const enet_uint8*>(msg.c_str()), msg.length());
            broadcast(traffic);
        }

        // send a packet to a specific peer
        void send(peer_id_t peer_id, const NetworkTraffic& msg)
        {
            ENetPeer* peer = getPeerPtr(peer_id);
            if (!peer) return;
            ENetPacket* packet = enet_packet_create(msg.packet_data, msg.packet_length, 0);
            std::lock_guard<std::mutex> lock(m_mutex);
            enet_peer_send(peer, 0, packet);
        }

        // send a string to a specific peer
        void send(peer_id_t peer_id, const std::string& msg)
        {
            NetworkTraffic traffic(reinterpret_cast<const enet_uint8*>(msg.c_str()), msg.length());
            send(peer_id, traffic);
        }

        // disconnects the given peer
        void disconnect(peer_id_t peer_id, bool force = false, uint32_t disconnection_data = 0)
        {
            ENetPeer* peer = getPeerPtr(peer_id);
            if (!peer) return;
            if (force) enet_peer_disconnect_now(peer, disconnection_data);
            else enet_peer_disconnect(peer, disconnection_data);
        }

        // disconnects all peers
        void disconnectAll(bool force = false, uint32_t disconnection_data = 0)
        {
            for (size_t i = 0; i < m_host->peerCount; ++i)
            {
                if (force) enet_peer_disconnect_now(&m_host->peers[i], disconnection_data);
                else enet_peer_disconnect(&m_host->peers[i], disconnection_data);
            }
        }
        
        // returns a raw ENet peer pointer
        ENetPeer* getPeerPtr(peer_id_t peer_id)
        {
            Peers::iterator it = m_peers.find(peer_id);
            if (it != m_peers.end()) return it->second;
            return NULL;
        }

        // get the address of the local host
        Address getAddress() const {
            std::lock_guard<std::mutex> lock(m_mutex);
            return convert(m_address);
        }
        
        // terminate the network thread, called on destruction
        void terminate()
        {
            m_quit = true;
            m_thread.join();
        }
    
    private:
        bool m_quit;
        ENetAddress m_address;
        ENetHost* m_host;
        typedef std::map<peer_id_t, ENetPeer*> Peers;
        Peers m_peers;
        mutable std::mutex m_mutex;
        std::jthread m_thread;
        NetworkListener& m_listener;
        void* m_data;
    };
}