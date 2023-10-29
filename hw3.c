#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#define PAGE_SIZE 8
#define VM_SIZE 128
#define VM_PAGE_COUNT 16 // also equals to page table's size
#define MM_SIZE 32
#define MM_PAGE_COUNT 4

int timer = 0;
struct PageTableEntry
{
    int valid_bit;
    int dirty_bit;
    int virtual_page_number; // = page_table[i]
    int physical_page_number;
    int last_access_time; // For FIFO
    int access_count;     // how many times we access this for LRU
};

void init_memory(int *memory, int size, int value)
{
    for (int i = 0; i < size; i++)
    {
        memory[i] = value;
    }
}

void init_page_table(struct PageTableEntry *page_table, int size)
{
    for (int i = 0; i < size; i++)
    {
        page_table[i].valid_bit = 0;
        page_table[i].dirty_bit = 0;
        page_table[i].virtual_page_number = i;
        page_table[i].physical_page_number = -1; // -1 no page in main memory
        page_table[i].last_access_time = timer;
        page_table[i].access_count = 0;
    }
}

int calc_physical_address(int physical_page_number, int offset)
{
    int physical_address = physical_page_number * PAGE_SIZE + offset;
    return (physical_page_number < VM_PAGE_COUNT) ? physical_address : -1;
}

int find_empty_page_in_main_mem(struct PageTableEntry *page_table)
{
    int main_mem_page_in_use[MM_PAGE_COUNT] = {0};
    for (int i = 0; i < VM_PAGE_COUNT; i++) // VM_PAGE_COUNT = page table's size
    {
        if (page_table[i].valid_bit == 1)
        {
            main_mem_page_in_use[page_table[i].physical_page_number] = 1;
        }
    }
    for (int i = 0; i < MM_PAGE_COUNT; i++)
    {
        if (main_mem_page_in_use[i] == 0)
        {
            return i;
        }
    }
    return -1;
}

int find_page_to_evict_fifo(struct PageTableEntry *page_table) // NEED TO BE FIXED
{
    int victim_page_number = -1;
    int earliest_access_time = timer + 1;
    for (int i = 0; i < VM_PAGE_COUNT; i++) // VM_PAGE_COUNT = page table's size
    {
        if (page_table[i].valid_bit == 1 && page_table[i].last_access_time < earliest_access_time) // has to be valid_bit |||| fifo: first_el.last_access_time always the least
        {
            victim_page_number = i;
            earliest_access_time = page_table[i].last_access_time;
        }
    }
    return victim_page_number;
}

int find_page_to_evict_lru(struct PageTableEntry *page_table) // NEED TO BE FIXED
{
    int victim_page_number = -1;
    int least_access_count = INT_MAX;
    for (int i = 0; i < VM_PAGE_COUNT; i++) // VM_PAGE_COUNT = page table's size
    {
        if (page_table[i].valid_bit == 1 && page_table[i].access_count < least_access_count) // has to be valid_bit |||| evict element that is being used the least
        {
            victim_page_number = i;
            least_access_count = page_table[i].access_count;
        }
    }
    return victim_page_number;
}

void load_page(struct PageTableEntry *page_table, int *disk_memory, int *main_memory, int virtual_page_number, int replacement_algorithm)
{
    int physical_page_number = find_empty_page_in_main_mem(page_table);
    if (physical_page_number == -1) // no available space in RAM. Swap
    {                               // FYI: never executed this block of line
        printf("No empty page in Main Mem. Need to evict\n");
        if (replacement_algorithm == 0)
        {
            physical_page_number = 0;
            // physical_page_number = find_page_to_evict_fifo(page_table);
            if (physical_page_number == -1)
            {
                printf("ERR: in find_page_to_evict_fifo");
            }
        }
        else
        {
            physical_page_number = find_page_to_evict_lru(page_table);
            if (physical_page_number == -1)
            {
                printf("ERR: in find_page_to_evict_lru");
            }
        }
        printf("Evicting page %d in Main Mem\n", physical_page_number);
        if (page_table[physical_page_number].dirty_bit) // update disk bc file editted
        {
            printf("Page %d has been edited. Copy it to Disk\n", physical_page_number);
            int disk_page_number = page_table[physical_page_number].virtual_page_number; // physical_page_number = virtual_page_number btw
            for (int i = 0; i < PAGE_SIZE; i++)
            {
                memcpy(&disk_memory[disk_page_number * PAGE_SIZE + i], &main_memory[physical_page_number * PAGE_SIZE + 1], sizeof(int));
            }
            page_table[physical_page_number].dirty_bit = 0;
        }
        page_table[physical_page_number].valid_bit = 0;
    }
    else
    { // there is an available sapce in RAM. in use rn
        printf("Page %d in Main Mem is empty.\n", physical_page_number);
    }

    printf("Copying from page %d in Disk to page %d in Main Mem.\n", virtual_page_number, physical_page_number);
    for (int i = 0; i < PAGE_SIZE; i++)
    {
        memcpy(&main_memory[physical_page_number * PAGE_SIZE + i], &disk_memory[virtual_page_number * PAGE_SIZE + 1], sizeof(int)); // load file to RAM |||| what is the +1 here: &disk_memory[virtual_page_number * PAGE_SIZE + 1]? shouldnt it be +i?. if this is a mistake, fix line 130 as well. you have that thing in load_page function as well
    }
    page_table[virtual_page_number].physical_page_number = physical_page_number;
    page_table[virtual_page_number].virtual_page_number = virtual_page_number; // Update the virtual_page_number
    page_table[virtual_page_number].valid_bit = 1;
    page_table[virtual_page_number].last_access_time = timer; // Update access time
    page_table[virtual_page_number].dirty_bit = 0;
}

void read_memory(struct PageTableEntry *page_table, int *disk_memory, int *main_memory, int virtual_address, int replacement_algorithm)
{
    int virtual_page_number = virtual_address / PAGE_SIZE;
    int offset = virtual_address % PAGE_SIZE;
    if (page_table[virtual_page_number].valid_bit == 0)
    {
        printf("A Page Fault Has Occurred\n");
        load_page(page_table, disk_memory, main_memory, virtual_page_number, replacement_algorithm);
    }
    int physical_page_number = page_table[virtual_page_number].physical_page_number;
    int physical_address = calc_physical_address(physical_page_number, offset);
    if (physical_address == -1)
    {
        printf("ERROR: physical_address == -1\n");
        return;
    }

    page_table[virtual_page_number].access_count++;
    page_table[virtual_page_number].last_access_time = timer; // Update access time
    printf("%d\n", main_memory[physical_address]);
}

void write_memory(struct PageTableEntry *page_table, int *disk_memory, int *main_memory, int virtual_address, int data, int replacement_algorithm)
{
    int virtual_page_number = virtual_address / PAGE_SIZE;
    int offset = virtual_address % PAGE_SIZE;
    if (page_table[virtual_page_number].valid_bit == 0)
    {
        printf("A Page Fault Has Occurred\n");
        load_page(page_table, disk_memory, main_memory, virtual_page_number, replacement_algorithm);
    }
    int physical_page_number = page_table[virtual_page_number].physical_page_number;
    int physical_address = calc_physical_address(physical_page_number, offset);
    if (physical_address == -1)
    {
        printf("ERROR: physical_address == -1\n");
        return;
    }

    main_memory[physical_address] = data;
    page_table[virtual_page_number].dirty_bit = 1;
    page_table[virtual_page_number].last_access_time = timer; // Update access time
    page_table[virtual_page_number].access_count++;
}

void showptable(struct PageTableEntry *page_table)
{
    for (int i = 0; i < VM_PAGE_COUNT; i++) // VM_PAGE_COUNT = page table's size
    {
        printf("%d:%d:%d:%d\n", i, page_table[i].valid_bit, page_table[i].dirty_bit, page_table[i].physical_page_number);
    }
}

void showmain(int *main_memory, int physical_page_number)
{
    if (physical_page_number < MM_PAGE_COUNT)
    {
        for (int i = 0; i < PAGE_SIZE; i++)
        {
            printf("%d: %d\n", physical_page_number + i, main_memory[physical_page_number + i]);
        }
    }
    else
    {
        printf("Main memory doesn't have this page.\n");
    }
}

int main(int argc, char *argv[])
{
    int replacement_algorithm = 0; // FIFO = 0, LRU = 1
    if (argc > 1 && strcmp(argv[1], "LRU") == 0)
    {
        replacement_algorithm = 1;
    }

    int main_memory[MM_SIZE];
    int disk_memory[VM_SIZE];
    struct PageTableEntry page_table[VM_PAGE_COUNT]; // VM_PAGE_COUNT = page table's size

    init_memory(main_memory, MM_SIZE, -1);
    init_memory(disk_memory, VM_SIZE, -1);
    init_page_table(page_table, VM_PAGE_COUNT); // VM_PAGE_COUNT = page table's size

    char command[10];
    int virtual_address;
    int data;
    while (1)
    {
        printf("> ");
        scanf("%s", command);
        if (strcmp(command, "quit") == 0)
        {
            break;
        }
        else if (strcmp(command, "read") == 0)
        {
            scanf("%d", &virtual_address);
            if (virtual_address < 0 || virtual_address >= VM_SIZE)
            {
                printf("Invalid virtual address\n");
            }
            else
            {
                read_memory(page_table, disk_memory, main_memory, virtual_address, replacement_algorithm);
            }
        }
        else if (strcmp(command, "write") == 0)
        {
            scanf("%d %d", &virtual_address, &data);
            if (virtual_address < 0 || virtual_address >= VM_SIZE)
            {
                printf("Invalid virtual address\n");
            }
            else
            {
                write_memory(page_table, disk_memory, main_memory, virtual_address, data, replacement_algorithm);
            }
        }
        else if (strcmp(command, "showptable") == 0)
        {
            showptable(page_table);
        }
        else if (strcmp(command, "showmain") == 0)
        {
            int physical_page_number;
            scanf("%d", &physical_page_number);
            showmain(main_memory, physical_page_number);
        }
        else
        {
            printf("Invalid command\n");
        }
        timer++;
    }

    return 0;
}
