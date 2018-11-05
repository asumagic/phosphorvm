## VM stacks

This is a WIP document.

### Parameter passing

This is problematic because this is something particularly performance-sensitive. Allocations should be reduced as much as possible.  
Unfortunately, it doesn't seem possible to pass arguments by simply preparing local variables for the incoming function calls because those variables seem to be pushed on the stack.  
However, depending on how this is implemented, this might be a problem: if the variable's value is pushed onto the stack before the data type and instance type are pushed then we cannot reliably find the n-th parameter from the stack *if* the value does not have a fixed size. This could be fixed by padding however, which is something to consider.