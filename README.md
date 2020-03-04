# angelscript-disassembler

This is a quick & dirty disassembler based on AngelScript's `asbuild` sample.

What this might be useful for:
- Tracking down compiler or VM bugs
- Help understanding the bytecode by looking at concrete examples (that is the reason this was made)

What this is **CANNOT** do:
- Disassemble bytecode files compiled by `asbuild` (this is what it should probably be doing, but this was faster)
- Do anything more than print out a primitive, probably sometimes broken textual representation of the AngelScript bytecode 

## Example output

```
Now disassembling bytecode.

Disassembly for void no_op():
Variables:
Code:
@0x0000 RET 0

Disassembly for void for_loop():
Variables:
- int i
Code:
@0x0000 SetV4 1 0
@0x0002 JMP 4
@0x0004 SUSPEND 
@0x0005 CALL 222
@0x0007 IncVi 1
@0x0008 CMPIi 1 10
@0x000a JS -8
@0x000c RET 0

Disassembly for void simple_if(int a, int b):
Variables:
- int a
- int b
Code:
@0x0000 CMPi 0 -1
@0x0002 JNP 2
@0x0004 CALL 222
@0x0006 RET 2

Disassembly for int return_global():
Variables:
Code:
@0x0000 CpyGtoV4 1 281827164050946
@0x0003 CpyVtoR4 1
@0x0004 RET 0
```

## Usage

Throw this in place of the `asbuild` directory in the AngelScript sdk ([SVN](https://sourceforge.net/projects/angelscript/), [unofficial GitHub mirror](https://github.com/codecat/angelscript-mirror)) (hence the 'quick & dirty') and build it.
