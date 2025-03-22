#ifndef ALKOS_KERNEL_INCLUDE_ACPI_TABLES_HPP_
#define ALKOS_KERNEL_INCLUDE_ACPI_TABLES_HPP_

#include <uacpi/tables.h>
#include <extensions/defines.hpp>
#include <extensions/types.hpp>

namespace ACPI
{

//////////////////////////////
//          Enums           //
//////////////////////////////

enum class TableType { kRSDT, kXSDT, kFADT, kMADT, kDSDT, kSSDT, kSRAT };

//////////////////////////////
//         Structs          //
//////////////////////////////

//////////////////////////////
//        Functions         //
//////////////////////////////

/**
 * @brief Get an ACPI table.
 * @param table The type of table to retrieve.
 * @return Requested ACPI table.
 */
uacpi_table GetTable(TableType type);

}  // namespace ACPI

#endif  // ALKOS_KERNEL_INCLUDE_ACPI_TABLES_HPP_
