#include "byte_stream.h"

#include <stdexcept>

ByteStream::ByteStream(const unsigned int cap)
    : m_length(0), m_offset(0)
{
    m_capacity = cap > 0 ? cap : INIT_CAPACITY;
    m_buffer = new unsigned char[m_capacity];
}

ByteStream::ByteStream(const ByteStream& b)
    : m_length(b.m_length), m_offset(0)
{
    m_capacity = b.m_capacity;
    m_buffer = new unsigned char[m_capacity];
    memcpy(m_buffer, b.m_buffer, m_capacity*sizeof(unsigned char));
}

ByteStream::ByteStream(const char* buf, size_t length)
    : m_length(length), m_offset(0)
{
    m_capacity = length;
    m_buffer = new unsigned char[m_capacity];
    memcpy(m_buffer, buf, length);
}

std::string ByteStream::getBuf() const
{
    return std::string(reinterpret_cast<char*>(m_buffer), m_length);
}

unsigned int ByteStream::getCapacity() const
{
    return m_capacity;
}

unsigned int ByteStream::getLength() const
{
    return m_length;
}

// helper method
void ByteStream::peek(void* buffer, size_t length) const
{
    length -= m_offset + length > m_length ? (m_length - m_offset) : 0;
    memcpy(buffer, m_buffer + m_offset, length);
}

char ByteStream::peekByte() const
{
    char c;
    peek(&c, sizeof(char));
    return c;
}

int8_t ByteStream::peekInt8() const
{
    int8_t c;
    peek(&c, sizeof(int8_t));
    return c;
}

// helper method
void ByteStream::read(void* buffer, size_t length)
{
    if (m_offset + length > m_length)
    {
        throw std::out_of_range("Exceeded end of buffer");
    }
    memcpy(buffer, m_buffer + m_offset, length);
    m_offset += length;
}

char ByteStream::readByte()
{
    char c;
    read(&c, sizeof(char));
    return c;
}

int8_t ByteStream::readInt8()
{
    int8_t i;
    read(&i, sizeof(int8_t));
    return i;
}

int16_t ByteStream::readInt16()
{
    int16_t i;
    read(&i, sizeof(int16_t));
    return i;
}

int32_t ByteStream::readInt32()
{
    int32_t i;
    read(&i, sizeof(int32_t));
    return i;
}

int64_t ByteStream::readInt64()
{
    int64_t i;
    read(&i, sizeof(int64_t));
    return i;
}

uint8_t ByteStream::readUInt8()
{
    uint8_t i;
    read(&i, sizeof(uint8_t));
    return i;   
}

uint16_t ByteStream::readUInt16()
{
    uint16_t i;
    read(&i, sizeof(uint16_t));
    return i;   
}

uint32_t ByteStream::readUInt32()
{
    uint32_t i;
    read(&i, sizeof(uint32_t));
    return i;   
}

uint64_t ByteStream::readUInt64()
{
    uint64_t i;
    read(&i, sizeof(uint64_t));
    return i;   
}

std::string ByteStream::readString()
{
    uint64_t length = readUInt64();
    if (m_offset + length > m_length)
    {
        throw std::out_of_range("Exceeded end of buffer");
    }
    std::string str(reinterpret_cast<char*>(m_buffer + m_offset), length);
    m_offset += length;
    return str;
}


// helper method
void ByteStream::resize()
{
    m_capacity = m_capacity * 2;
    unsigned char* temp = new unsigned char[m_capacity];

    memcpy(temp, m_buffer, m_capacity);
    delete[] m_buffer;
    m_buffer = temp;
}

// helper method
bool ByteStream::write(const void* data, size_t length)
{
    m_length += length;
    // loop in case of large string
    while (m_length >= m_capacity) 
    {
        resize();
    }
    memcpy(m_buffer + m_offset, data, length);
    m_offset += length;
    return true;
}

bool ByteStream::writeByte(char byte)
{
    return write(&byte, sizeof(char));
}

bool ByteStream::writeInt8(int8_t i)
{
    return write(&i, sizeof(int8_t));
}

bool ByteStream::writeInt16(int16_t i)
{
    return write(&i, sizeof(int16_t));
}

bool ByteStream::writeInt32(int32_t i)
{
    return write(&i, sizeof(int32_t));
}

bool ByteStream::writeInt64(int64_t i)
{
    return write(&i, sizeof(int64_t));
}

bool ByteStream::writeUInt8(uint8_t i)
{
    return write(&i, sizeof(uint8_t));
}

bool ByteStream::writeUInt16(uint16_t i)
{
    return write(&i, sizeof(uint16_t));
}

bool ByteStream::writeUInt32(uint32_t i)
{
    return write(&i, sizeof(uint32_t));
}

bool ByteStream::writeUInt64(uint64_t i)
{
    return write(&i, sizeof(uint64_t));
}

bool ByteStream::writeString(const std::string& str, uint64_t length)
{
    bool success = writeUInt64(length);
    return success && write(str.c_str(), length);
}