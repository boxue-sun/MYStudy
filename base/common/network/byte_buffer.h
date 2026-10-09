/*********************************************************************************
 * @file		byte_buffer.h
 * @brief		byte_buffer belongs to CICTCI
 * @details
 * @author		cs
 * @date		2014-08-21
 * @copyright	Copyright (c) 2024 CICTCI-Airos Division.
 * @verbatim
 *
 *  Change History:
 *  Date      Author    Version  ChangeId           Description
 *  ------------------------------------------------------------------------------
 *  2014-08-21 cs       1.0       ————             Create this file
 *
 * @endverbatim
 ********************************************************************************/
#ifndef _BYTE_BUFFER_H
#define _BYTE_BUFFER_H

#include "define.h"
#include "net_util.h"

NAMESPACE_AFL_NET_START

/// A buffer class modeled after org.jboss.netty.buffer.ChannelBuffer
/// see http://blog.csdn.net/solstice/article/details/6329080
/// @code
/// +-------------------+------------------+------------------+
/// | prependable bytes |  readable bytes  |  writable bytes  |
/// |                   |     (CONTENT)    |                  |
/// +-------------------+------------------+------------------+
/// |                   |                  |                  |
/// 0      <=      readerIndex   <=   writerIndex    <=     size
/// @endcode
class ByteBuffer
{
public:
  //  static const size_t kCheapPrepend = 8;
  //  static const size_t kInitialSize = 1024;

public:
    ByteBuffer(size_t cheapPrepend = 8, size_t initialSize = 1024)
		: kCheapPrepend(cheapPrepend)
		, kInitialSize(initialSize)
        , readerIndex_(kCheapPrepend)
        , writerIndex_(kCheapPrepend)
        , buffer_(kCheapPrepend + kInitialSize)
    {
        assert(readableBytes() == 0);
        assert(writableBytes() == kInitialSize);
        assert(prependableBytes() == kCheapPrepend);
    }

    size_t readableBytes() const
    {
        return writerIndex_ - readerIndex_;
    }

    size_t writableBytes() const
    {
        return buffer_.size() - writerIndex_;
    }

    size_t prependableBytes() const
    {
        return readerIndex_;
    }

    std::string toString() const
    {
        return std::string(peek(), static_cast<int>(readableBytes()));
    }

public:
    void write(const std::string& str)
    {
        write(str.data(), str.size());
    }

    void write(const char* data)
    {
        write(data, strlen(data));
    }

    void write(const char* data, size_t len)
    {
        ensureWritableBytes(len);
        std::copy(data, data + len, beginWrite());
        hasWritten(len);
    }

    void write(const void* data, size_t len)
    {
        write(static_cast<const char*>(data), len);
    }

    template <typename Number>
    void write(Number num)
    {
        Number nnum = NetUtil::host2Net(num);
        write(&nnum, sizeof(nnum));
    }

    template <typename Number>
    Number read()
    {
        Number nnum = peek<Number>();
        retrieve(sizeof(nnum));
        return nnum;
    }


    template <typename Number>
    Number peek() const
    {
        assert(readableBytes() >= sizeof(Number));
        Number nnum = 0;
        ::memcpy(&nnum, peek(), sizeof(nnum));
        return NetUtil::net2Host(nnum);
    }


    template <typename Number>
    void prepend(Number num)
    {
        Number nnum = NetUtil::host2Net(num);
        prepend(&nnum, sizeof(nnum));
    }

    void prepend(const void* data, size_t len)
    {
        assert(len <= prependableBytes());
        readerIndex_ -= len;
        const char* d = static_cast<const char*>(data);
        std::copy(d, d + len, begin() + readerIndex_);
    }

    void retrieve(size_t len)
    {
        assert(len <= readableBytes());
        if (len < readableBytes())
        {
            readerIndex_ += len;
        }
        else
        {
            retrieveAll();
        }
    }

    template <typename Number>
    void retrieve()
    {
        retrieve(sizeof(Number));
    }

    void retrieveUntil(const char* end)
    {
        assert(peek() <= end);
        assert(end <= beginWrite());
        retrieve(end - peek());
    }

    void retrieveAll()
    {
        readerIndex_ = kCheapPrepend;
        writerIndex_ = kCheapPrepend;
    }

    std::string retrieveAllAsString()
    {
        return retrieveAsString(readableBytes());;
    }

    std::string retrieveAsString(size_t len)
    {
        assert(len <= readableBytes());
        std::string result(peek(), len);
        retrieve(len);
        return result;
    }

public:    // search
    const char* peek() const
    {
        return begin() + readerIndex_;
    }

    const char* findCRLF() const
    {
        const char* crlf = std::search(peek(), beginWrite(), kCRLF, kCRLF + 2);
        return crlf == beginWrite() ? NULL : crlf;
    }

    const char* findCRLF(const char* start) const
    {
        assert(peek() <= start);
        assert(start <= beginWrite());
        const char* crlf = std::search(start, beginWrite(), kCRLF, kCRLF + 2);
        return crlf == beginWrite() ? NULL : crlf;
    }

    const char* findDoubleCRLF() const
    {
        const char* crlf = std::search(peek(), beginWrite(), kDoubleCRLF, kDoubleCRLF + 4);
        return crlf == beginWrite() ? NULL : crlf;
    }

    const char* findEOL() const
    {
        const void* eol = memchr(peek(), '\n', readableBytes());
        return static_cast<const char*>(eol);
    }

    const char* findEOL(const char* start) const
    {
        assert(peek() <= start);
        assert(start <= beginWrite());
        const void* eol = memchr(start, '\n', beginWrite() - start);
        return static_cast<const char*>(eol);
    }

    const char* findFeature(const void* f, size_t l) const
    {
        const char* feature = std::search(peek(), beginWrite(), (char*)f, ((char*)f) + l);
        return feature == beginWrite() ? NULL : feature;
    }

public:
    void ensureWritableBytes(size_t len)
    {
        if (writableBytes() < len)
        {
            makeSpace(len);
        }
        assert(writableBytes() >= len);
    }

    char* beginWrite()
    {
        return begin() + writerIndex_;
    }

    const char* beginWrite() const
    {
        return begin() + writerIndex_;
    }

    void hasWritten(size_t len)
    {
        assert(len <= writableBytes());
        writerIndex_ += len;
    }

    void unwrite(size_t len)
    {
        assert(len <= readableBytes());
        writerIndex_ -= len;
    }

    void shrink(size_t reserve)
    {
        ByteBuffer other;
        other.ensureWritableBytes(readableBytes() + reserve);
        other.write(toString());
        swap(other);
    }

    size_t capacity() const
    {
        return buffer_.capacity();
    }

    void swap(ByteBuffer& rhs)
    {
        buffer_.swap(rhs.buffer_);
        std::swap(readerIndex_, rhs.readerIndex_);
        std::swap(writerIndex_, rhs.writerIndex_);
    }

private:
    char* begin()
    {
        return &*buffer_.begin();
    }

    const char* begin() const
    {
        return &*buffer_.begin();
    }

    void makeSpace(size_t len)
    {
        if (writableBytes() + prependableBytes() < len + kCheapPrepend)
        {
            // FIXME: move readable data
            buffer_.resize(writerIndex_ + len);
        }
        else
        {
            // move readable data to the front, make space inside buffer
            assert(kCheapPrepend < readerIndex_);
            size_t readable = readableBytes();
            std::copy(begin() + readerIndex_, begin() + writerIndex_, begin() + kCheapPrepend);
            readerIndex_ = kCheapPrepend;
            writerIndex_ = readerIndex_ + readable;
            assert(readable == readableBytes());
        }
    }

private:
    const size_t kCheapPrepend;
    const size_t kInitialSize;

    size_t readerIndex_;
    size_t writerIndex_;
    std::vector<char> buffer_;     // save buffer of network endian

    static const char kCRLF[];
    static const char kDoubleCRLF[];
};

NAMESPACE_AFL_NET_END
#endif  /* _BYTEBUFFER_H */
