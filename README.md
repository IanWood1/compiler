# Compiler

## Todos:

- [X] Changes to arrays aka int64[] to int64[N] (N is a compile time constant)
    - [X] Change syntax of array creation to make more clear that they are stack arrays
    - [X] arrays must have a compile time size
- [X] Pass by value vs pass by reference
    - [X] Pass by value (and return by value) creates a copy of data
    - [X] By reference doesnt, just copies internal pointer.
- [ ] **Create a more extensible logging system class**
- [ ] **Turning on warnings as errors!**
- [ ] Fix references: they currently are reassignable (oof). Should act more like C++ references.
- [ ] Fix/Update internal type system
- [ ] Update build configs
    - [ ] cmake changes
    - [ ] compiler.c should actually take command line args (other than filename)
    - [ ] MORE and BETTER tests
- [ ] Flush out structs
    - [ ] How are structs created/destroyed?
    - [ ] How to access members?
    - [ ] How to access/define functions associated with structs?
- [ ] Add better type checking
    - [ ] Check to see if modified values are mutable
    - [ ] Make sure correct types are used in expressions (started)
- [ ] Compiler driver (and modules). Needs to be flushed out before starting work

## Types

Below are examples of the currently supported types:

| Example   | Explanation                          |
|-----------|--------------------------------------|
| int64     | normal integer 64 bits wide          |
| int64&    | reference to an integer              |
| int64[N]  | integer array of length N            |
| int64[N]& | reference to integer array of length |

