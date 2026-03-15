#include <efi/efi.h>
#include <efi/efilib.h>

#include "menu.h"
#include "actions.h"

#include "uefi/common/console/screen.h"
#include "uefi/common/console/input.h"
#include "uefi/common/image/image.h"

MenuEntry menu_entries[] =
    {
        {L"Load Kernel", action_load_and_start_image, L"\\EFI\\BOOT\\OsLoader.efi"},
        {L"Reboot", action_reboot, NULL},
        {L"Firmware", action_exit, NULL},
        {L"Shutdown", action_shutdown, NULL}};

MenuEntry *get_boot_menu_entries(void)
{
    return menu_entries;
}

UINTN get_boot_menu_entry_count(void)
{
    return sizeof(menu_entries) / sizeof(menu_entries[0]);
}

EFI_STATUS execute_menu_entry(
    MenuEntry *entry,
    EFI_HANDLE ImageHandle,
    EFI_SYSTEM_TABLE *SystemTable)
{
    if (entry == NULL || entry->action == NULL)
        return EFI_INVALID_PARAMETER;

    return entry->action(ImageHandle, SystemTable, entry->context);
}

static void draw_menu(
    EFI_SYSTEM_TABLE *SystemTable,
    const CHAR16 *title,
    MenuEntry *entries,
    UINTN entry_count,
    UINTN selected)
{

    clear_screen(SystemTable);

    Print(L"%s\r\n\r\n", title);

    for (UINTN i = 0; i < entry_count; i++)
    {
        if (i == selected)
            Print(L"> %s\r\n", entries[i].label);
        else
            Print(L"  %s\r\n", entries[i].label);
    }

    Print(L"\r\nUse Up/Down and Enter.\r\n");
}

EFI_STATUS run_menu(
    EFI_HANDLE ImageHandle,
    EFI_SYSTEM_TABLE *SystemTable,
    const CHAR16 *title,
    MenuEntry *entries,
    UINTN entry_count,
    UINTN *selected_index)
{
    EFI_INPUT_KEY key;
    UINTN selected = 0;

    (void)ImageHandle;

    if (SystemTable == NULL || title == NULL || entries == NULL || entry_count == 0 || selected_index == NULL)
        return EFI_INVALID_PARAMETER;

    for (;;)
    {
        draw_menu(SystemTable, title, entries, entry_count, selected);

        key = get_key(SystemTable);

        if (key.ScanCode != 0)
        {
            switch (key.ScanCode)
            {
            case SCAN_UP:
                if (selected > 0)
                    selected--;
                break;

            case SCAN_DOWN:
                if (selected + 1 < entry_count)
                    selected++;
                break;

            default:
                break;
            }
        }
        else
        {
            switch (key.UnicodeChar)
            {
            case CHAR_CARRIAGE_RETURN:
                *selected_index = selected;
                return EFI_SUCCESS;

            default:
                break;
            }
        }
    }
}
