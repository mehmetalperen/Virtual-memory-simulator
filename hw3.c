#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#define VM_SIZE 128
#define MM_SIZE 32
#define PAGE_SIZE 8
#define PAGE_NUM (VM_SIZE / PAGE_SIZE)

int timer = 0;
struct PageTableEntry
{
    int valid_bit;
    int dirty_bit;
    int virtual_page_number; // = page_table[i]
    int physical_page_number;
    int in_use; // 1 = in_use, 0 = not in use
    int last_access_time;
    int access_count; // how many times we access this
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
        page_table[i].in_use = 0;
        page_table[i].last_access_time = timer;
        page_table[i].access_count = 0;
    }
}

int calc_physical_address(int physical_page_number, int offset)
{
    int physical_address = physical_page_number * PAGE_SIZE + offset;
    return (physical_page_number < PAGE_NUM) ? physical_address : -1;
}

int find_available_el(struct PageTableEntry *page_table, int size)
{
    for (int i = 0; i < size; i++)
    {
        if (!page_table[i].in_use)
        {
            return i;
        }
    }
    return -1;
}

int find_evict_el_fifo(struct PageTableEntry *page_table, int size)
{
    int evict_el = 0;
    for (int i = 1; i < size; i++)
    {
        if (page_table[i].last_access_time < page_table[evict_el].last_access_time) // fifo: first_el.last_access_time always the least
        {
            evict_el = i;
        }
    }
    return evict_el;
}

int find_evict_el_lru(struct PageTableEntry *page_table, int size)
{
    int evict_el = 0;
    for (int i = 1; i < size; i++)
    {
        if (page_table[i].access_count < page_table[evict_el].access_count) // evict element that is being used the least
        {
            evict_el = i;
        }
    }
    return evict_el;
}

void load_page(struct PageTableEntry *page_table, int *disk_memory, int *main_memory, int virtual_page_number, int replacement_algorithm)
{
    int physical_page_number = find_available_el(page_table, PAGE_NUM);
    if (physical_page_number == -1) // no available space in RAM. Swap
    {
        if (replacement_algorithm == 0)
        {
            physical_page_number = find_evict_el_fifo(page_table, PAGE_NUM);
        }
        else
        {
            physical_page_number = find_evict_el_lru(page_table, PAGE_NUM);
        }
        // printf("swap with: %d\n", physical_page_number);
        if (page_table[physical_page_number].dirty_bit) // update disk bc file editted
        {
            int disk_page_number = page_table[physical_page_number].virtual_page_number; // physical_page_number = virtual_page_number btw
            memcpy(&disk_memory[disk_page_number * PAGE_SIZE], &main_memory[physical_page_number * PAGE_SIZE], PAGE_SIZE * sizeof(int));
            page_table[physical_page_number].dirty_bit = 0;
        }
        page_table[physical_page_number].valid_bit = 0;
        page_table[physical_page_number].in_use = 0;
    }
    else
    { // there is an available sapce in RAM. in use rn
        page_table[physical_page_number].in_use = 1;
    }
    memcpy(&main_memory[physical_page_number * PAGE_SIZE], &disk_memory[virtual_page_number * PAGE_SIZE], PAGE_SIZE * sizeof(int)); // load file to RAM
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
    for (int i = 0; i < PAGE_NUM; i++)
    {
        printf("%d:%d:%d:%d\n", i, page_table[i].valid_bit, page_table[i].dirty_bit, page_table[i].physical_page_number);
    }
}
int main(int argc, char *argv[])
{
    int replacement_algorithm = 0; // FIFO = 0, LRU = 1
    if (argc > 1 && strcmp(argv[1], "LRU") == 0)
    {
        replacement_algorithm = 1;
    }

    int virtual_memory[VM_SIZE];
    int main_memory[MM_SIZE];
    int disk_memory[VM_SIZE];
    struct PageTableEntry page_table[PAGE_NUM];

    init_memory(virtual_memory, VM_SIZE, -1);
    init_memory(main_memory, MM_SIZE, -1);
    init_memory(disk_memory, VM_SIZE, -1);
    init_page_table(page_table, PAGE_NUM);

    char command[10];
    int virtual_address;
    int data;
    while (1)
    {
        printf("> ");
        scanf("%s", command);
        if (strcmp(command, "exit") == 0) // Not sure how we exit the program
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
        else
        {
            printf("Invalid command\n");
        }
        timer++;
    }

    return 0;
}
