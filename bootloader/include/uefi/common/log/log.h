#ifndef UEFI_COMMON_CONSOLE_LOG_H
#define UEFI_COMMON_CONSOLE_LOG_H

#include <efi/efi.h>
#include <efi/efilib.h>

#define LOG_INFO(fmt, ...) Print(L"[INFO] " fmt L"\r\n", ##__VA_ARGS__)
#define LOG_WARN(fmt, ...) Print(L"[WARN] " fmt L"\r\n", ##__VA_ARGS__)
#define LOG_ERROR(fmt, ...) Print(L"[ERR ] " fmt L"\r\n", ##__VA_ARGS__)

#ifdef DEBUG
#define LOG_DEBUG(fmt, ...) Print(L"[DBG ] " fmt L"\r\n", ##__VA_ARGS__)
#else
#define LOG_DEBUG(fmt, ...)
#endif

#endif
