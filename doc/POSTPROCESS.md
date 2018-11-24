## Bytecode post-processing in PhosphorVM

In order to simplify execution of the GameMaker:Studio bytecode in PhosphorVM, the program modifies the bytecode before execution.

### Variable and function reference preprocessing

The `data.win` format is awkward when it comes to `VARI` and `FUNC`. These declare linked lists to the next occurrence for each of them.

PhosphorVM replace the `next_occurrence` field in each of them by an id for each of those.