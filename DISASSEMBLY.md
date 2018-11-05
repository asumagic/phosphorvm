## Bytecode disassembly

Bytecode disassembly allows to view the GameMaker:Studio bytecode in a human-readable form.

Example of a disassembled fibonacci function bytecode:

```
Disassembly of 'gml_Script_script_fibo': 30 blocks (120 bytes)
$00000000: pushspc        argument0           ; $c305ffff'a000001e 
$00000002: push.i16       0                   ; $840f0000 
$00000003: cmp.i32.var    ==                  ; $15520300 
$00000004: bf                                 ; $b8000004 
$00000005: push.i16       0                   ; $840f0000 
$00000006: conv.i32.var                       ; $07520000 
$00000007: ret.var                            ; $9c050000 
$00000008: pushspc        argument0           ; $c305ffff'a000001e 
$0000000a: push.i16       1                   ; $840f0001 
$0000000b: cmp.i32.var    ==                  ; $15520300 
$0000000c: bf                                 ; $b8000004 
$0000000d: push.i16       1                   ; $840f0001 
$0000000e: conv.i32.var                       ; $07520000 
$0000000f: ret.var                            ; $9c050000 
$00000010: pushspc        argument0           ; $c305ffff'a000001e 
$00000012: push.i16       2                   ; $840f0002 
$00000013: sub.i32.var                        ; $0d520000 
$00000014: call.i32       script_fibo         ; $d9020001'00000004 
$00000016: pushspc        argument0           ; $c305ffff'a000001e 
$00000018: push.i16       1                   ; $840f0001 
$00000019: sub.i32.var                        ; $0d520000 
$0000001a: call.i32       script_fibo         ; $d9020001'00000004 
$0000001c: add.var.var                        ; $0c550000 
$0000001d: ret.var                            ; $9c050000 ```

The first hexadecimal value (e.g. `$0000001d`) represents the offset, in **blocks** (32-bit values), of the opcode appearing on the right.

The following name is the mnemonic for the instruction. The mnemonic is unique for an instruction ID, e.g. `pushspc` matches opcode `$C3` (as can be seen on line `$00000008`).  
Period-separated names following the instruction names are operand types. Fr oinstance, `cmp.i32.var` pops a `i32` then a `var` and compares them.

The values after this are the instruction operands. `pushspc argument0` means an instruction named `pushspc` that has `argument0` as an operand, which is deduced from an ID (strings are denoted with `"`).

At the end of the line, the semicolon `;` indicates a comment, which gives complementary information, useful for debugging or to find out useful data that is not shown in the disassembly.  
The opcode for the current line's instruction is always displayed in hexadecimal as `u32`s, in big-endian.  
It is possible for the disassembler to include extra information in the comment, for example to warn the user about an improperly disassembled instruction.

Instructions that failed to disassemble are marked between angle brackets (`<>`) and appears as red in the console (if supported/enabled by the \{fmt\} library on your platform), e.g. `<bad>` or `<unimpl>`.