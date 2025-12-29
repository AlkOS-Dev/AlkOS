#ifndef LIBS_LIBCPP_INCLUDE_FORMATS_HPP_
#define LIBS_LIBCPP_INCLUDE_FORMATS_HPP_

template <typename ObjT>
char *DumpObjToBufferHex(const ObjT &obj, char *buffer, size_t buffer_size)
{
    assert(buffer != nullptr);
    assert(buffer_size > 0);

    const auto obj_bytes = reinterpret_cast<const byte *>(&obj);
    for (size_t i = 0; i < sizeof(ObjT); ++i) {
        const int bytes_written = snprintf(buffer, buffer_size, "%02X ", obj_bytes[i]);
        assert(
            bytes_written < static_cast<int>(buffer_size) && "DumpObjToBufferHex buffer fully used!"
        );

        buffer += bytes_written;
        buffer_size -= bytes_written;
    }

    return buffer;
}

template <typename ObjT, typename FuncT>
void DumpObjToHex(FuncT func, const ObjT &obj)
{
    static constexpr size_t kBuffSize = 3 * sizeof(ObjT) + 2;
    char buffer[kBuffSize];

    auto ptr = DumpObjToBufferHexWithSep(obj, buffer, kBuffSize);
    ptr[0]   = '\n';
    ptr[1]   = '\0';
    func(buffer);
}

template <typename ObjT, size_t kSepDist = 8>
char *DumpObjToBufferHexWithSep(const ObjT &obj, char *buffer, size_t buffer_size)
{
    assert(buffer != nullptr);
    assert(buffer_size > 0);

    const auto obj_bytes = reinterpret_cast<const byte *>(&obj);
    size_t sep_dist      = 0;
    for (size_t i = 0; i < sizeof(ObjT); ++i) {
        const int bytes_written = snprintf(
            buffer, buffer_size, ++sep_dist == kSepDist ? "%02X | " : "%02X ", obj_bytes[i]
        );
        sep_dist %= kSepDist;

        assert(
            bytes_written < static_cast<int>(buffer_size) &&
            "DumpObjToBufferHexWithSep buffer fully used!"
        );

        buffer += bytes_written;
        buffer_size -= bytes_written;
    }

    return buffer;
}

template <typename ObjT, typename FuncT, size_t kSepDist = 8>
void DumpObjHexWithSep(FuncT func, const ObjT &obj)
{
    static constexpr size_t kBuffSize = sizeof(ObjT) * 3 + 2 * (sizeof(ObjT) / kSepDist) + 2;
    char buffer[kBuffSize];

    const auto ptr = DumpObjToBufferHexWithSep<ObjT, kSepDist>(obj, buffer, kBuffSize);
    ptr[0]         = '\n';
    ptr[1]         = '\0';
    func(buffer);
}

template <size_t kSize, size_t kSepDist = 8, typename FuncT>
void DumpMemHex(FuncT func, void *mem)
{
    static constexpr size_t kBuffSize = 4 * kSize;
    char buffer[kBuffSize];

    size_t sep_dist    = 0;
    u8 *ptr            = reinterpret_cast<u8 *>(mem);
    char *buff_ptr     = buffer;
    size_t buffer_size = kBuffSize;
    for (size_t i = 0; i < kSize; ++i) {
        const int bytes_written =
            snprintf(buff_ptr, buffer_size, ++sep_dist == kSepDist ? "%02X\n" : "%02X", ptr[i]);

        sep_dist %= kSepDist;

        assert(
            bytes_written < static_cast<int>(buffer_size) &&
            "DumpObjToBufferHexWithSep buffer fully used!"
        );

        buffer_size -= bytes_written;
        buff_ptr += bytes_written;
    }

    func(buffer);
}

#endif  // LIBS_LIBCPP_INCLUDE_FORMATS_HPP_
