#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define VM_SIZE 128
#define MM_SIZE 32
#define PAGE_SIZE 8
#define PAGE_NUM (VM_SIZE / PAGE_SIZE)

struct PageTableEntry
{
    int valid_bit;
    int dirty_bit;
    int page_number;
    int physical_page_number;
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
        page_table[i].page_number = i;
        page_table[i].physical_page_number = -1; // -1 no page in main memory
    }
}

int calc_physical_address(int physical_page_number, int offset)
{
    int physical_address = physical_page_number * PAGE_SIZE + offset;
    printf("physical_address: %d, physical_page_number: %d, offset: %d \n", physical_address, physical_page_number, offset);
    return (physical_address >= 0 && physical_page_number < MM_SIZE) ? physical_address : -1;
}

void read_memory(int *virtual_memory, struct PageTableEntry *page_table, int *main_memory, int virtual_address)
{
    int page_number = virtual_address / PAGE_SIZE;
    int offset = virtual_address % PAGE_SIZE;
    if (page_table[page_number].valid_bit == 0)
    {
        printf("need to implement load\n");
    }
    int physical_page_number = page_table[page_number].physical_page_number;
    int physical_address = calc_physical_address(physical_page_number, offset);
    if (physical_address == -1)
    {
        printf("ERROR: physical_address == -1");
        return;
    }

    printf("%d\n", main_memory[physical_address]);
}

void write_memory(int *virtual_memory, struct PageTableEntry *page_table, int *main_memory, int virtual_address, int data)
{
    int page_number = virtual_address / PAGE_SIZE;
    int offset = virtual_address % PAGE_SIZE;
    if (page_table[page_number].valid_bit == 0)
    {
        printf("need to implement load\n");
    }
    int physical_page_number = page_table[page_number].physical_page_number;
    int physical_address = calc_physical_address(physical_page_number, offset);
    if (physical_address == -1)
    {
        printf("ERROR: physical_address == -1");
        return;
    }

    main_memory[physical_address] = data;
    page_table[page_number].dirty_bit = 1;
}

int main()
{
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
        if (strcmp(command, "exit") == 0)
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
                read_memory(virtual_memory, page_table, main_memory, virtual_address);
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
                write_memory(virtual_memory, page_table, main_memory, virtual_address, data);
            }
        }
        else
        {
            printf("Invalid command\n");
        }
    }

    return 0;
}
