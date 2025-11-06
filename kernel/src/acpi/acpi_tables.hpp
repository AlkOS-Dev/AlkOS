#ifndef KERNEL_SRC_ACPI_ACPI_TABLES_HPP_
#define KERNEL_SRC_ACPI_ACPI_TABLES_HPP_

#include <uacpi/acpi.h>
#include <uacpi/tables.h>
#include <template_lib.hpp>
#include "acpi.tpp"

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

template <typename Callback>
concept TableEntryCallback = requires(Callback cb, acpi_entry_hdr *entry) {
    { cb(entry) } -> std::same_as<void>;
};

template <typename T>
concept IsTable = concepts_ext::OneOf<
    T, acpi_dsdt, acpi_madt, acpi_srat, acpi_fadt, acpi_ssdt, acpi_facs, acpi_mcfg, acpi_slit,
    acpi_gtdt, acpi_hpet, acpi_ecdt, acpi_rhct>;

template <IsTable T>
class Table
{
    public:
    explicit Table(const uacpi_table t) : table_(t) {}
    ~Table() { Unref_(); }

    Table(const Table &)            = delete;
    Table &operator=(const Table &) = delete;
    Table(Table &&other) noexcept : table_(other.table_) { other.table_.ptr = nullptr; }
    Table &operator=(Table &&other) noexcept
    {
        if (this != &other) {
            Unref_();
            table_           = other.table_;
            other.table_.ptr = nullptr;
        }
        return *this;
    }

    [[nodiscard]] T *GetNative() const { return static_cast<T *>(table_.ptr); }

    [[nodiscard]] size_t GetSize() const
    {
        ASSERT_NOT_NULL(table_.ptr);
        return table_.hdr->length;
    }

    [[nodiscard]] bool IsValid() const;
    [[nodiscard]] bool IsNull() const { return table_.ptr == nullptr; }
    void GetNext() { uacpi_table_find_next_with_same_signature(&table_); }

    template <TableEntryCallback Callback>
    void ForEachTableEntry(Callback callback)
    {
        static_assert(false, "Entries not available for this table");
    }

    private:
    uacpi_table table_;

    bool ValidChecksum_() const;

    void Unref_();
};

//////////////////////////////
//        Functions         //
//////////////////////////////

template <IsTable T>
static Table<T> GetTable()
{
    uacpi_table table{};
    uacpi_table_find_by_signature(TableSignature<T>::value, &table);

    return Table<T>(table);
}

}  // namespace ACPI

#include "acpi_tables.tpp"
#include "tables/acpi_madt.tpp"
#include "tables/acpi_srat.tpp"

#endif  // KERNEL_SRC_ACPI_ACPI_TABLES_HPP_
