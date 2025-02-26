#pragma once

#include <string>

/**
 * Simple ByteStream class used to pack and
 * unpack messages received over the network.
 */
class ByteStream
{
    const unsigned int INIT_CAPACITY = 64; ///< initial buffer capacity

public:
    ByteStream(const unsigned int cap = 0);
    ByteStream(const ByteStream& b);
    ByteStream(const char* buf, size_t length);
    ~ByteStream() { delete[] m_buffer; }

    // Resets read/write pointer to the beginning
    void resetPtr() { m_offset = 0; }

    // Returns true if we've reached the end of the buffer
    bool end() const { return m_offset == m_length; }

    // Returns a copy of the byte buffer as string
    std::string getBuf() const;
    
    // Returns the current buffer capacity
    unsigned int getCapacity() const;

    // Returns the current number of bytes written to the stream
    unsigned int getLength() const;

private:
    void peek(void* buffer, size_t length) const; // helper used by peek methods

public:
    char peekByte() const;
    int8_t peekInt8() const;

private:
    void read(void* buffer, size_t length); // helper used by read methods

public:
    char readByte();
    
    int8_t readInt8();
    int16_t readInt16();
    int32_t readInt32();
    int64_t readInt64();
    
    uint8_t readUInt8();
    uint16_t readUInt16();
    uint32_t readUInt32();
    uint64_t readUInt64();

    std::string readString();

private:
    void resize(); // helper used to resize when capacity is hit
    bool write(const void* data, size_t length); // helper used by write methods
    
public:
    bool writeByte(char byte);
    
    bool writeInt8(int8_t i);
    bool writeInt16(int16_t i);
    bool writeInt32(int32_t i);
    bool writeInt64(int64_t i);

    bool writeUInt8(uint8_t i);
    bool writeUInt16(uint16_t i);
    bool writeUInt32(uint32_t i);
    bool writeUInt64(uint64_t i);
    
    bool writeString(const std::string& str, uint64_t length);
    
private:
    unsigned char* m_buffer; ///< buffer
    unsigned int m_capacity; ///< current capacity, current memory available
    unsigned int m_length;   ///< current length, number of bytes written to buffer
    unsigned int m_offset;   ///< offset pointer, position where next byte is read/written
};
