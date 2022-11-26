# Concepts-of-Multi-Threading-and-Networking

## Multi-Threading

### Question-1

- Implementation has been done using threads in pthread library.
- Compilation requires -lpthread flag : gcc -lpthread q1.c
- The test case input is as mentioned in the question

- Implementation uses semaphore for the number of washing machines free, which is initialized to the number of washing machines initially.
- At every event, the time instant from the start of execution is printed.
- Color coding is used for specific events.
- No mutex locks are used as the semaphore automatically locks and unlocks based on availability of the value.

### Question-2

