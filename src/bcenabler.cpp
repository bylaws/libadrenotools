// SPDX-License-Identifier: BSD-2-Clause
// Copyright © 2021 Billy Laws

#include <fstream>
#include <string>
#include <cstring>
#include <sys/mman.h>
#include <adrenotools/bcenabler.h>
#include "gen/bcenabler_patch.h"

enum adrenotools_bcn_type adrenotools_get_bcn_type(uint32_t major, uint32_t minor) {
    if (major != 512)
        return ADRENOTOOLS_BCN_INCOMPATIBLE;

    if (minor >= 514)
        return ADRENOTOOLS_BCN_BLOB;

    return ADRENOTOOLS_BCN_PATCH;
}

// Searches /proc/self/maps for the first free page after the given address
static void *find_free_page(uintptr_t address) {
    std::ifstream procMaps("/proc/self/maps");

    uintptr_t end{};

    for (std::string line; std::getline(procMaps, line); ) {
        std::size_t addressSeparator{line.find('-')};
        uintptr_t start{std::strtoull(line.substr(0, addressSeparator).c_str(), nullptr, 16)};

        if (end > address && start != end)
            return reinterpret_cast<void *>(end);

        end = std::strtoull(line.substr(addressSeparator + 1, line.find( ' ')).c_str(), nullptr, 16);
    }

    return nullptr;
}

bool adrenotools_patch_bcn(void *vkGetPhysicalDeviceFormatPropertiesFn) {
    union Branch {
        struct {
            int32_t offset : 26; //!< 26-bit branch offset
            uint8_t sig : 6;  //!< 6-bit signature (0x25 for linked, 0x5 for jump)
        };

        uint32_t raw{};
    };
    static_assert(sizeof(Branch) == 4, "Branch size is invalid");

    // First branch in this function is targeted at the function we want to patch
    Branch *blInst{reinterpret_cast<Branch *>(vkGetPhysicalDeviceFormatPropertiesFn)};

    constexpr uint8_t BranchLinkSignature{0x25};

    // Search for first instruction with the BL signature
    while (blInst->sig != BranchLinkSignature)
        blInst++;

    // Internal QGL format conversion function that we need to patch
    uint32_t *convFormatFn{reinterpret_cast<uint32_t *>(blInst) + blInst->offset};

    // This would normally set the default result to 0 (error) in the format not found case
    constexpr uint32_t ClearResultSignature{0x2a1f03e0};

    // We replace it with a branch to our own extended if statement which adds in the extra things for BCn
    uint32_t *clearResultPtr{convFormatFn};
    while (*clearResultPtr != ClearResultSignature)
        clearResultPtr++;

    // Find the nearest unmapped page where we can place patch code
    void *patchPage{find_free_page(reinterpret_cast<uintptr_t>(clearResultPtr))};
    if (!patchPage)
        return false;

    void *ptr{mmap(patchPage, PAGE_SIZE,  PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED, 0, 0)};
    if (ptr != patchPage)
        return false;

    // Ensure we don't write out of bounds
    if (PatchRawData_size > PAGE_SIZE)
        return false;

    // Copy the patch function to our mapped page
    memcpy(patchPage, PatchRawData, PatchRawData_size);

    // Fixup the patch code so it correctly returns back to the driver after running
    constexpr uint32_t PatchReturnFixupMagic{0xffffffff};
    constexpr uint8_t BranchSignature{0x5};

    uint32_t *fixupTargetPtr = clearResultPtr + 1;
    uint32_t *fixupPtr = reinterpret_cast<uint32_t *>(patchPage);
    for (long unsigned int i{}; i < (PatchRawData_size / sizeof(uint32_t)); i++, fixupPtr++) {
        if (*fixupPtr == PatchReturnFixupMagic) {
            Branch branchToDriver{
                {
                   .offset = static_cast<int32_t>((reinterpret_cast<intptr_t>(fixupTargetPtr) - reinterpret_cast<intptr_t>(fixupPtr)) / sizeof(int32_t)),
                   .sig = BranchSignature,
                }
            };

            *fixupPtr = branchToDriver.raw;
        }
    }

    Branch branchToPatch{
        {
            .offset = static_cast<int32_t>((reinterpret_cast<intptr_t>(patchPage) - reinterpret_cast<intptr_t>(clearResultPtr)) / sizeof(int32_t)),
            .sig = BranchSignature,
        }
    };

    void *driverPatchPage{reinterpret_cast<void *>(reinterpret_cast<uintptr_t>(clearResultPtr) & ~(PAGE_SIZE - 1))};

    // For some reason mprotect just breaks entirely after we patch the QGL instruction so just set perms to RWX
    if (mprotect(driverPatchPage, PAGE_SIZE,  PROT_WRITE | PROT_READ | PROT_EXEC))
        return false;

    *clearResultPtr = branchToPatch.raw;

    asm volatile("ISB");

    // Done!
    return true;
}
