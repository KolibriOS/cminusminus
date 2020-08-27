# Use the C or C++ compiler
Compiler                = wcl386

# compiler options for code that has to run from an interrupt:
Compiler_Options       = /5r /zp1 /c /s /fp3 /ox /ot /y /cc++ /d_WC_ /mf
#/oxl+ /j /s /zu /Iinclude

Linker_options          = option stack=65536 op stub=zrdx.exe op c

# Compile using dos4g or pmodew
System                  = dos4g

Exe_file                = c--.exe

####################
## Makefile rules ##
####################

all        : c--.exe

c--.exe : main.obj toka.obj tokb.obj tokc.obj toke.obj tokr.obj &
          errors.obj debug.obj outobj.obj outpe.obj disasm.obj switch.obj &
	outle.obj pointer.obj new_type.obj class.obj res.obj optreg.obj libobj.obj

             *wlink system $(System) $(Linker_Options) name c--.exe &
             file {$<}

clean
        del *.obj
        del *.err
        del *.bak

# next are the exceptions that don't have to be compiled with the /zu option
# since they will never be called from a interrupt. hey.. I _tried_ to find
# a clean solution..

main.obj :  main.cpp
                $(Compiler) $(Compiler_Options) $<

toka.obj :  toka.cpp
                $(Compiler) $(Compiler_Options) $<

tokb.obj :  tokb.cpp
                $(Compiler) $(Compiler_Options) $<

tokc.obj :  tokc.cpp
                $(Compiler) $(Compiler_Options) $<

toke.obj :  toke.cpp
                $(Compiler) $(Compiler_Options) $<

tokr.obj :  tokr.cpp
                $(Compiler) $(Compiler_Options) $<

errors.obj :errors.cpp
                $(Compiler) $(Compiler_Options) $<

debug.obj : debug.cpp
                $(Compiler) $(Compiler_Options) $<

outobj.obj : outobj.cpp
                $(Compiler) $(Compiler_Options) $<

outpe.obj : outpe.cpp
                $(Compiler) $(Compiler_Options) $<

disasm.obj : disasm.cpp
                $(Compiler) $(Compiler_Options) $<

switch.obj : switch.cpp
                $(Compiler) $(Compiler_Options) $<

outle.obj : outle.cpp
                $(Compiler) $(Compiler_Options) $<

pointer.obj : pointer.cpp
                $(Compiler) $(Compiler_Options) $<

new_type.obj : new_type.cpp
                $(Compiler) $(Compiler_Options) $<

class.obj : class.cpp
                $(Compiler) $(Compiler_Options) $<

res.obj : res.cpp
                $(Compiler) $(Compiler_Options) $<

optreg.obj : optreg.cpp
                $(Compiler) $(Compiler_Options) $<

libobj.obj : libobj.cpp
                $(Compiler) $(Compiler_Options) $<

.cpp.obj:
        $(Compiler) $(Compiler_Options) $<

.asm.obj:
        wasm -3pr $<
