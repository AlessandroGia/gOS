#include <efi/efi.h>
#include <efi/efilib.h>

#include "uefi/common/console/menu/menu.h"

#include "uefi/common/console/screen.h"
#include "uefi/common/console/input.h"

static void draw_menu(
    EFI_SYSTEM_TABLE *SystemTable,
    const CHAR16 *title,
    MenuEntry *entries,
    UINTN entry_count,
    UINTN selected,
    UINTN *start_menu,
    UINTN *end_menu)
{

    clear_screen(SystemTable);

    Print(L"%s\r\n\r\n", title);

    if (*end_menu > entry_count)
        *end_menu = entry_count;

    if (selected > (*end_menu - 1))
    {
        *end_menu += 1;
        *start_menu += 1;
    }
    if (selected < *start_menu)
    {
        *start_menu -= 1;
        *end_menu -= 1;
    }
    for (UINTN i = *start_menu; i < *end_menu; i++)
    {
        if (i == selected)
            Print(L"> %s\r\n", entries[i].label);
        else
            Print(L"  %s\r\n", entries[i].label);
    }

    Print(L"\r\nUse Up/Down and Enter.\r\n");
}

static EFI_STATUS menu(
    EFI_HANDLE ImageHandle,
    EFI_SYSTEM_TABLE *SystemTable,
    const CHAR16 *title,
    MenuEntry *entries,
    UINTN entry_count,
    UINTN *selected_index,
    BOOLEAN escape_to_exit,
    BOOLEAN *flag_exit,
    UINTN window)
{
    EFI_INPUT_KEY key;
    UINTN start_menu = 0;
    UINTN end_menu = entry_count < window ? entry_count : window;
    UINTN selected = 0;

    (void)ImageHandle;

    if (SystemTable == NULL || title == NULL || entries == NULL || entry_count == 0 || selected_index == NULL || flag_exit == NULL)
        return EFI_INVALID_PARAMETER;

    for (;;)
    {
        draw_menu(SystemTable, title, entries, entry_count, selected, &start_menu, &end_menu);

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

            case SCAN_ESC:
                if (escape_to_exit)
                {
                    *flag_exit = TRUE;
                    clear_screen(SystemTable);
                    return EFI_SUCCESS;
                }
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
                clear_screen(SystemTable);
                return EFI_SUCCESS;

            default:
                break;
            }
        }
    }
}

static EFI_STATUS execute_menu_entry(
    MenuEntry *entry,
    EFI_HANDLE ImageHandle,
    EFI_SYSTEM_TABLE *SystemTable)
{
    if (entry == NULL || entry->action == NULL)
        return EFI_INVALID_PARAMETER;

    return entry->action(ImageHandle, SystemTable, entry->context);
}

EFI_STATUS run_menu(
    EFI_HANDLE ImageHandle,
    EFI_SYSTEM_TABLE *SystemTable,
    const CHAR16 *title,
    MenuEntry *entries,
    UINTN entry_count,
    BOOLEAN escape_to_exit,
    UINTN window)
{
    UINTN selected_index = 0;
    BOOLEAN flag_exit = FALSE;

    while (TRUE)
    {
        menu(ImageHandle, SystemTable, title, entries, entry_count, &selected_index, escape_to_exit, &flag_exit, window);
        if (escape_to_exit && flag_exit)
            return EFI_SUCCESS;
        execute_menu_entry(&entries[selected_index], ImageHandle, SystemTable);
    }
    return EFI_SUCCESS;
}
