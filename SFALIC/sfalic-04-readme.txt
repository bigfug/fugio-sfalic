SFALIC version 04

The homepage of the implementation is: http://sun.iinf.polsl.gliwice.pl/~rstaros/sfalic/

Compilation instructions:

To compile the implementation just unzip the sources to an empty directory and compile all the *.c files, using the following compiler options:
- maximum speed optimization (maximixe speed, whole program optimization/inter-procedural optimization)
- define NDEBUG,
- for a BigEndian CPU (UltraSPARC) define BE_MACHINE (not obligatory),
- for a LittleEndian CPU (IA-32) do not define BE_MACHINE.
You may try defining ALT_STATCOMPRESSROWWM - on some sytems (sun studio 10 compiler/UltraSPARC IIIi) it improves the compression speed.

Changes from version 03:

- removed options -evol 6, -pred 9, and -maxclen 32,
- compressed file format interchangeable with version 03 (except for removed options),
- optimized decoding,
- commented in English.

Commandline syntax:

    sfalic c|d infile outfile [switches]
       c - compress
       d - decompress
       image format: PGM P5
       switches:
         -echo      -- echo processed cmdline and input file header
         -noheader  -- skip writting output file header (for testing only)
         -generic8bpp -- (de)compress 8bpp imgs using generic 16bpp routines
                       (slower, for testing only)
       compression switches:
         -decoronly -- decorrelate only (skip stat. compression)
         -pred pred -- pred number: -1 to 8, def.: 8
                       -1: raw binary copy 
                       0-7: Loslles JPEG predictors,
                       8: (3A+3B-2C)/4
         -evol evol -- model structure: 0 to 5, def.: 3
                       1: consecutive bucket sizes: 1 1 1 2 2 4 4 8 8 ...
                       3: 1 2 4 8 16 32 64 ...
                       5: 1 4 16 64 256 1024 4096 16384 65536
                       0, 2, 4: obsolete
         -trigger trigger -- bucket trigger: 0 to 2000, def.: 0
                       threshold for halving bucket counters
                       must be an integer multiple of 10
                       0: auto select trigger for evol and wait mask
         -maxclen maxclen -- codeword length limit: img. depth+1 to 31, def.: 26
         -wm wmistart wmimax wminext -- model update frequency controll 
                 wmistart -- starting frequency index: 0 to wmimax, def.: 0
                       0: 100%, 1: 66%, 2: 40%, 3: 22.2%, 4: 11.8%,
                       5: 6.06%, 6: 3.08%, 7: 1.55%, 8: 0.778%, ...
                 wmimax -- target frequency index: wmistart to 15, def.: 6
                 wminext -- number of pixels encoded before increasing
                       frequency index: 1 to 100000000, def.: 2048
                       must be an integer power of 2
