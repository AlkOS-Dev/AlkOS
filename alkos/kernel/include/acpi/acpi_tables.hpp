#ifndef ALKOS_KERNEL_INCLUDE_ACPI_TABLES_HPP_
#define ALKOS_KERNEL_INCLUDE_ACPI_TABLES_HPP_

#include <uacpi/acpi.h>
#include <uacpi/internal/tables.h>
#include <uacpi/tables.h>
#include <extensions/defines.hpp>
#include <extensions/type_traits.hpp>
#include <extensions/types.hpp>
#include "acpi.tpp"

#include <string.h>

namespace ACPI
{

//////////////////////////////
//          Enums           //
//////////////////////////////

//////////////////////////////
//         Structs          //
//////////////////////////////

//////////////////////////////
//         Classes          //
//////////////////////////////

template <typename T, typename... Ts>
concept OneOf = (std::is_same_v<T, Ts> || ...);

template <typename T>
concept IsTable = OneOf<
    T, acpi_rsdt, acpi_xsdt, acpi_madt, acpi_srat, acpi_fadt, acpi_dsdt, acpi_ssdt, acpi_facs,
    acpi_mcfg, acpi_slit, acpi_gtdt, acpi_hpet, acpi_ecdt, acpi_rhct>;

template <IsTable T>
class Table
{
    public:
    explicit Table(const uacpi_table t) : table_(t) {}
    ~Table() { Unref(); }

    Table(const Table&)            = delete;
    Table& operator=(const Table&) = delete;
    Table(Table&& other) noexcept : table_(other.table_) { other.table_.ptr = nullptr; }
    Table& operator=(Table&& other) noexcept
    {
        if (this != &other) {
            Unref();
            table_           = other.table_;
            other.table_.ptr = nullptr;
        }
        return *this;
    }

    T* GetNative() const { return static_cast<T*>(table_.ptr); }
    size_t GetSize() const { return table_.hdr->length; }
    bool IsValid() const
    {
        if (table_.ptr == nullptr)
            return false;

        // FACS doesn't have a checksum since it has OSPM writable fields
        if (TableSignature<T>::value == "FACS")
            return true;

        if (!ValidChecksum())
            return false;

        return true;
    }
    bool IsNull() const { return table_.ptr == nullptr; }
    void GetNext() { uacpi_table_find_next_with_same_signature(&table_); }

    template <typename U>
    U* GetTableEntry();

    private:
    uacpi_table table_;

    bool ValidChecksum() const
    {
        const u8* bytes = static_cast<u8*>(table_.ptr);
        u8 checksum     = 0;

        for (size_t i = 0; i < table_.hdr->length; ++i) checksum += bytes[i];

        return checksum == 0;
    }

    void Unref()
    {
        if (table_.ptr) {
            uacpi_table_unref(&table_);
            table_.ptr = nullptr;
        }
    }
};

#include "tables/acpi_madt.tpp"
#include "tables/acpi_srat.tpp"

//////////////////////////////
//        Functions         //
//////////////////////////////

template <IsTable T>
static Table<T> GetTable()
{
    uacpi_table table;
    uacpi_table_find_by_signature(TableSignature<T>::value, &table);

    return Table<T>(table);
}

}  // namespace ACPI

#endif  // ALKOS_KERNEL_INCLUDE_ACPI_TABLES_HPP_
