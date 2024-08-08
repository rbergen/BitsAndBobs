# Simple sieve

This directory contains a simple prime sieve implemented in K&R C that was originally developed by [davepl](https://github.com/davepl) for BSD2.11 on the PDP-11.

This version has been changed to run on MS-DOS. It's been confirmed to compile with Borland C++ 5.02, and run when the compiler is told to only output machine code that's compatible with the Intel 8086.

The two included executables are:

- sieve86.exe: A build of sieve.c that contains only Intel 8086 processor instructions
- sieve386.exe: A build of sieve.c that has been optimized for the Intel 80386