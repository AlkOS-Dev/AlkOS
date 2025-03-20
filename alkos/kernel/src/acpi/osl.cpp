#include <string.h>
#include <uacpi/kernel_api.h>
#include <constants.hpp>
#include <extensions/debug.hpp>
#include <io.hpp>
#include <todo.hpp>

uacpi_status uacpi_kernel_get_rsdp(uacpi_phys_addr *out_rsdp_address)
{
    *out_rsdp_address = reinterpret_cast<uacpi_phys_addr>(kACPIRsdpAddr);
    return UACPI_STATUS_OK;
}

template <typename T>
uacpi_status uacpi_kernel_pci_read(uacpi_handle device, uacpi_size offset, T *value)
{
    const uacpi_pci_address addr = *static_cast<uacpi_pci_address *>(device);
    ASSERT(addr.device < 32 && addr.function < 8);

    const auto address = (addr.bus << 16) | (addr.device << 11) | (addr.function << 8) |
                         (offset & 0xFC) | 0x80000000;

    outl(address, 0xCF8);

    offset &= 3;  // Address must be 32-bit aligned
    switch (sizeof(T)) {
        case 1:
            *value = inb(0xCFC + offset);
            break;
        case 2:
            *value = inw(0xCFC + offset);
            break;
        case 4:
            *value = inl(0xCFC + offset);
            break;
        default:
            return UACPI_STATUS_INVALID_ARGUMENT;
    }

    return UACPI_STATUS_OK;
}

template <typename T>
uacpi_status uacpi_kernel_pci_write(uacpi_handle device, uacpi_size offset, T value)
{
    const uacpi_pci_address addr = *static_cast<uacpi_pci_address *>(device);
    ASSERT(addr.device < 32 && addr.function < 8);

    const auto address = (addr.bus << 16) | (addr.device << 11) | (addr.function << 8) |
                         (offset & 0xFC) | 0x80000000;

    outl(address, 0xCF8);

    offset &= 3;  // Address must be 32-bit aligned
    switch (sizeof(T)) {
        case 1:
            outb(0xCFC + offset, value);
            break;
        case 2:
            outw(0xCFC + offset, value);
            break;
        case 4:
            outl(0xCFC + offset, value);
            break;
        default:
            return UACPI_STATUS_INVALID_ARGUMENT;
    }

    return UACPI_STATUS_OK;
}

uacpi_status uacpi_kernel_pci_device_open(uacpi_pci_address address, uacpi_handle *out_handle)
{
    memcpy(out_handle, &address, sizeof(uacpi_pci_address));
    return UACPI_STATUS_OK;
}

void uacpi_kernel_pci_device_close(uacpi_handle handle) {}

uacpi_status uacpi_kernel_pci_read8(uacpi_handle device, uacpi_size offset, uacpi_u8 *value)
{
    return uacpi_kernel_pci_read(device, offset, value);
}

uacpi_status uacpi_kernel_pci_read16(uacpi_handle device, uacpi_size offset, uacpi_u16 *value)
{
    return uacpi_kernel_pci_read(device, offset, value);
}

uacpi_status uacpi_kernel_pci_read32(uacpi_handle device, uacpi_size offset, uacpi_u32 *value)
{
    return uacpi_kernel_pci_read(device, offset, value);
}

uacpi_status uacpi_kernel_pci_write8(uacpi_handle device, uacpi_size offset, uacpi_u8 value)
{
    return uacpi_kernel_pci_write(device, offset, value);
}

uacpi_status uacpi_kernel_pci_write16(uacpi_handle device, uacpi_size offset, uacpi_u16 value)
{
    return uacpi_kernel_pci_write(device, offset, value);
}

uacpi_status uacpi_kernel_pci_write32(uacpi_handle device, uacpi_size offset, uacpi_u32 value)
{
    return uacpi_kernel_pci_write(device, offset, value);
}

template <typename T>
uacpi_status uacpi_kernel_raw_io_write(uacpi_io_addr address, T in_value)
{
    switch (sizeof(T)) {
        case 1: {
            outb(address, in_value);
            break;
        }
        case 2: {
            outw(address, in_value);
            break;
        }
        case 4: {
            outl(address, in_value);
            break;
        }
        default:
            return UACPI_STATUS_INVALID_ARGUMENT;
    }

    return UACPI_STATUS_OK;
}

template <typename T>
uacpi_status uacpi_kernel_raw_io_read(uacpi_io_addr address, T *out_value)
{
    switch (sizeof(T)) {
        case 1: {
            *out_value = inb(address);
            break;
        }
        case 2: {
            *out_value = inw(address);
            break;
        }
        case 4: {
            *out_value = inl(address);
            break;
        }
        default:
            return UACPI_STATUS_INVALID_ARGUMENT;
    }

    return UACPI_STATUS_OK;
}

uacpi_status uacpi_kernel_io_map(uacpi_io_addr base, uacpi_size len, uacpi_handle *out_handle)
{
    *out_handle = reinterpret_cast<uacpi_handle>(base);
    return UACPI_STATUS_OK;
}

void uacpi_kernel_io_unmap(uacpi_handle handle) {}

uacpi_status uacpi_kernel_io_read8(uacpi_handle handle, uacpi_size offset, uacpi_u8 *out_value)
{
    const auto addr = reinterpret_cast<uacpi_io_addr>(handle);
    return uacpi_kernel_raw_io_read(addr + offset, out_value);
}

uacpi_status uacpi_kernel_io_read16(uacpi_handle handle, uacpi_size offset, uacpi_u16 *out_value)
{
    const auto addr = reinterpret_cast<uacpi_io_addr>(handle);
    return uacpi_kernel_raw_io_read(addr + offset, out_value);
}

uacpi_status uacpi_kernel_io_read32(uacpi_handle handle, uacpi_size offset, uacpi_u32 *out_value)
{
    const auto addr = reinterpret_cast<uacpi_io_addr>(handle);
    return uacpi_kernel_raw_io_read(addr + offset, out_value);
}

uacpi_status uacpi_kernel_io_write8(uacpi_handle handle, uacpi_size offset, uacpi_u8 in_value)
{
    auto addr = reinterpret_cast<uacpi_io_addr>(handle);
    return uacpi_kernel_raw_io_write(addr + offset, in_value);
}

uacpi_status uacpi_kernel_io_write16(uacpi_handle handle, uacpi_size offset, uacpi_u16 in_value)
{
    auto addr = reinterpret_cast<uacpi_io_addr>(handle);
    return uacpi_kernel_raw_io_write(addr + offset, in_value);
}

uacpi_status uacpi_kernel_io_write32(uacpi_handle handle, uacpi_size offset, uacpi_u32 in_value)
{
    auto addr = reinterpret_cast<uacpi_io_addr>(handle);
    return uacpi_kernel_raw_io_write(addr + offset, in_value);
}

void *uacpi_kernel_map(uacpi_phys_addr addr, uacpi_size len)
{
    TODO_WHEN_VMEM_WORKS
    return nullptr;
}

void uacpi_kernel_unmap(void *addr, uacpi_size len) { TODO_WHEN_VMEM_WORKS }

void *uacpi_kernel_alloc(uacpi_size size)
{
    // return kmalloc(size);
    return nullptr;
}

void uacpi_kernel_free(void *mem)
{
    if (mem) {
        // kfree();
    }
}

void uacpi_kernel_log(uacpi_log_level level, const uacpi_char *log)
{
    switch (level) {
        case UACPI_LOG_ERROR:
            FormatTrace("[ERROR]     %s", log);
            break;
        case UACPI_LOG_WARN:
            FormatTrace("[WARNING]   %s", log);
            break;
        case UACPI_LOG_INFO:
            FormatTrace("[INFO]      %s", log);
            break;
        case UACPI_LOG_DEBUG:
            FormatTrace("[DEBUG]     %s", log);
            break;
        case UACPI_LOG_TRACE:
            FormatTrace("[TRACE]     %s", log);
            break;
    }
}

uacpi_u64 uacpi_kernel_get_nanoseconds_since_boot() { return 0; }

void uacpi_kernel_stall(uacpi_u8 usec) {}

void uacpi_kernel_sleep(uacpi_u64 msec) {}

uacpi_handle uacpi_kernel_create_mutex() { return nullptr; }

void uacpi_kernel_free_mutex(uacpi_handle) {}

uacpi_handle uacpi_kernel_create_event() { return nullptr; }

void uacpi_kernel_free_event(uacpi_handle) {}

uacpi_thread_id uacpi_kernel_get_thread_id() { return nullptr; }

uacpi_status uacpi_kernel_acquire_mutex(uacpi_handle, uacpi_u16)
{
    return UACPI_STATUS_UNIMPLEMENTED;
}

void uacpi_kernel_release_mutex(uacpi_handle) {}

uacpi_bool uacpi_kernel_wait_for_event(uacpi_handle, uacpi_u16) { return UACPI_FALSE; }

void uacpi_kernel_signal_event(uacpi_handle) {}

void uacpi_kernel_reset_event(uacpi_handle) {}

uacpi_status uacpi_kernel_handle_firmware_request(uacpi_firmware_request *)
{
    return UACPI_STATUS_UNIMPLEMENTED;
}

uacpi_status uacpi_kernel_install_interrupt_handler(
    uacpi_u32 irq, uacpi_interrupt_handler, uacpi_handle ctx, uacpi_handle *out_irq_handle
)
{
    return UACPI_STATUS_UNIMPLEMENTED;
}

uacpi_status uacpi_kernel_uninstall_interrupt_handler(
    uacpi_interrupt_handler, uacpi_handle irq_handle
)
{
    return UACPI_STATUS_UNIMPLEMENTED;
}

uacpi_handle uacpi_kernel_create_spinlock() { return nullptr; }

void uacpi_kernel_free_spinlock(uacpi_handle) {}

uacpi_cpu_flags uacpi_kernel_lock_spinlock(uacpi_handle) { return 0; }

void uacpi_kernel_unlock_spinlock(uacpi_handle, uacpi_cpu_flags) {}

uacpi_status uacpi_kernel_schedule_work(uacpi_work_type, uacpi_work_handler, uacpi_handle ctx)
{
    return UACPI_STATUS_UNIMPLEMENTED;
}

uacpi_status uacpi_kernel_wait_for_work_completion() { return UACPI_STATUS_UNIMPLEMENTED; }
