# Virtual Memory Simulator

## Overview
This project simulates a virtual memory system, handling the complexities of page replacement, memory allocation, and disk interaction. It supports basic memory operations such as read and write to a simulated virtual address space, managing the transfer of pages between disk and main memory. The simulator implements two page replacement algorithms: FIFO (First-In-First-Out) and LRU (Least Recently Used), selectable via command-line arguments.

## Getting Started
To launch the simulator:

1. Compile the source code with GCC version 11.3.0: `gcc -o vmsim hw.c`.
2. Run the simulator using one of the following commands:
   - Without arguments (defaults to FIFO): `./vmsim`
   - With FIFO algorithm: `./vmsim FIFO`
   - With LRU algorithm: `./vmsim LRU`

## Features
- **Memory System Parameters:** Simulates a virtual memory system with 128 addresses, main memory of 32 addresses, and 16 total pages.
- **User Commands:** Supports commands for reading and writing to virtual addresses, displaying the contents of main memory and the page table, and quitting the simulator.
- **Page Replacement Algorithms:** Implements FIFO and LRU algorithms for managing page faults and memory allocation.

## Commands
- `read <virtual_addr>`: Prints the content at the specified virtual address. Indicates a page fault if the page is not in main memory.
- `write <virtual_addr> <num>`: Writes a number to the specified virtual address. Indicates a page fault if necessary.
- `showmain <ppn>`: Displays the content of a physical page number in main memory.
- `showptable`: Prints the page table entries, including valid and dirty bits.
- `quit`: Exits the simulator.

## Page Replacement
- **FIFO:** Replaces the oldest page in main memory upon a page fault.
- **LRU:** Replaces the least recently used page in main memory upon a page fault.
- Pages are transferred between disk and main memory as needed, maintaining the valid and dirty bits accurately.

## Testing
- 100% line coverage is required. Use the provided `autocov.py` script with your test cases to ensure full coverage.
- Test data must be stored in `.run` files, and all tests must be zipped with the source code into a single file (`hw.zip`) for submission.

## Submission Instructions
1. Zip your `hw.c` source file and all `.run` test files into `hw.zip`.
2. Ensure your program compiles and runs on `openlab.ics.uci.edu` using gcc 11.3.0 without any additional compiler flags.
3. Submit the zip file through Gradescope, adhering to the specified naming conventions and including a comment with your name and ID.

## Example Usage
Here are examples of running the simulator with both FIFO and LRU algorithms, demonstrating how to interact with the system and view memory states:

### FIFO Algorithm
```
$ ./a.out FIFO
showptable
read 9
write 9 201
showmain 0
quit
```

### LRU Algorithm
```
$ ./a.out LRU

write 10 202
write 31 403
read 72
showmain 1
quit```
