#ifndef ALKOS_LIBC_INCLUDE_EXTENSIONS_FORMATS_HPP_
#define ALKOS_LIBC_INCLUDE_EXTENSIONS_FORMATS_HPP_

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

#endif  // ALKOS_LIBC_INCLUDE_EXTENSIONS_FORMATS_HPP_
