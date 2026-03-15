#ifndef MENU_H
#define MENU_H

typedef EFI_STATUS (*MenuAction)(
    EFI_HANDLE ImageHandle,
    EFI_SYSTEM_TABLE *SystemTable,
    void *context);

typedef struct
{
    const CHAR16 *label;
    MenuAction action;
    void *context;
} MenuEntry;

EFI_STATUS run_menu(
    EFI_HANDLE ImageHandle,
    EFI_SYSTEM_TABLE *SystemTable,
    const CHAR16 *title,
    MenuEntry *entries,
    UINTN entry_count,
    BOOLEAN escape_to_exit);

#endif
