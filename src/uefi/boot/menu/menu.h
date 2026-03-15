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

extern MenuEntry menu_entries[];
extern const UINTN menu_entry_count;

void show_boot_menu();

MenuEntry *get_boot_menu_entries(void);
UINTN get_boot_menu_entry_count(void);

EFI_STATUS execute_menu_entry(
    MenuEntry *entry,
    EFI_HANDLE ImageHandle,
    EFI_SYSTEM_TABLE *SystemTable);

EFI_STATUS run_menu(
    EFI_HANDLE ImageHandle,
    EFI_SYSTEM_TABLE *SystemTable,
    const CHAR16 *title,
    MenuEntry *entries,
    UINTN entry_count,
    UINTN *selected_index);

#endif
