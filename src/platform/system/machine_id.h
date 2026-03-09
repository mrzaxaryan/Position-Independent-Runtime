/**
 * @file machine_id.h
 * @brief Machine-unique identifier retrieval
 *
 * @details Provides a cross-platform function to retrieve a stable,
 * machine-unique UUID from the operating system. The identifier is
 * constant across reboots and process restarts.
 *
 * Platform implementations:
 * - Windows: SMBIOS Type 1 UUID via NtQuerySystemInformation (hardware-level,
 *   survives OS reinstalls)
 * - Linux/Android/FreeBSD: /etc/machine-id (systemd, generated at install)
 *
 * @ingroup platform
 */

#pragma once

#include "core/types/uuid.h"
#include "core/types/error.h"
#include "core/types/result.h"

/**
 * @brief Retrieves a machine-unique UUID from the operating system.
 *
 * @details Platform-specific implementations:
 * - Windows: Extracts the SMBIOS Type 1 (System Information) UUID from
 *   firmware tables via NtQuerySystemInformation. This is a hardware-level
 *   UUID assigned by the OEM, constant across reboots and OS reinstalls.
 * - Linux/Android/FreeBSD: Reads /etc/machine-id (systemd 128-bit identifier,
 *   generated at install time, constant across reboots).
 *
 * @return Ok(UUID) on success, Err on failure (firmware table unavailable,
 *         SMBIOS Type 1 not found, file not readable, etc.)
 */
[[nodiscard]] Result<UUID, Error> GetMachineUUID();
