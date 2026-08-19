
This program implements a Virtual Memory Manager in C.
It supports both basic translation (no demand paging) and extended translation (with demand paging).

Files Included:
- vm_manager.c
- vm (executable)

COMPILATION
-----------

To compile the program, run:

    gcc vm_manager.c -o vm

This creates the executable file named "vm".

EXECUTION
---------

The program requires 4 command line arguments:

    ./vm <init_file> <input_file> <output_file> <dp_flag>

Where:
- init_file   = initialization file
- input_file  = file containing virtual addresses
- output_file = file where physical addresses are written
- dp_flag     = 0 (no demand paging)
                1 (with demand paging)

EXAMPLES
--------

Without demand paging:

    ./vm init-no-dp.txt input-no-dp.txt output-no-dp.txt 0

With demand paging:

    ./vm init-dp.txt input-dp.txt output-dp.txt 1


The program writes all translated physical addresses to the specified output file.
If a translation fails, -1 is written.