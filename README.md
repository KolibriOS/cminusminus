## C--

C-- is a programming language occupying an intermediate position
between Assembler and C.  It is ideally suited for writing small
programs, drivers, or interrupt handlers. In order to work
with C--  you should know Assembler and C.  C-- can now
generate 32-bit code for DOS, Windows, and KolibriOS.

The author of SPHINX C--  is Peter Cellik (CANADA).
The author's latest version of SPHINX C-- is v0.203 of October 28, 1996.
Unfortunately the author has abandoned further development of the language.
The language and its source code were declared an orphan and are freely
given to anyone anywhere to use as they wish.
On this page you will find the latest (but no longer the author's)
version of the language, libraries, sample programs, a description
of the language and libraries in Norton Guide format, and lots of other
useful information on C--.

Source code from last supporter (in Russia) by Michael Shecker.

Currently it's ported to the modern compilers for Linux.

## Building

With GNU Make:
```sh
mkdir build
cd build
cmake ..
make
```

With Ninja:
```sh
mkdir build
cd build
cmake -G Ninja ..
ninja
```
