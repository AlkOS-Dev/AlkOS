// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 The AlkOS Authors
// See the AUTHORS file for the full list of contributors.

#ifndef KERNEL_SRC_SYS_LOADER_HPP_
#define KERNEL_SRC_SYS_LOADER_HPP_

#include <expected.hpp>

#include "fs/vfs/path.hpp"
#include "mem/virt/addr_space.hpp"

namespace System
{
using std::expected;

enum class LoadError : u8 { FileNotFound, InvalidElf, IoError, MemoryError };

static constexpr const char *to_string(const LoadError &error)
{
    switch (error) {
        case LoadError::FileNotFound:
            return "FileNotFound";
        case LoadError::IoError:
            return "IoError";
        case LoadError::MemoryError:
            return "MemoryError";
        case LoadError::InvalidElf:
            return "InvalidElf";
        default:
            return "Unknown";
    }
}

class ElfLoader
{
    public:
    /**
     * @brief Loads an ELF executable from the filesystem into the specified address space.
     * @param path Path to the ELF file.
     * @param as The target address space to load into.
     * @return The entry point address or an error.
     */
    static expected<Mem::VPtr<void>, LoadError> Load(const vfs::Path &path, Mem::AddressSpace &as);
};

}  // namespace System

#endif  // KERNEL_SRC_SYS_LOADER_HPP_
