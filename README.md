# Homeworks for "Algorithms and data structures" course

## Building

To build homeworks use CMake:

```bash
cd <homework>
cmake -B build
cmake --build build
```

## Testing

Programs are tests by themselves so just use ctest:

```bash
cd build
ctest
```

## Homework 1 - binary tree

Homework requirements:

* Create binary tree class
* Fill tree with random values
* Randomly remove values from filled tree

## Homework 2 - stable selection sort

Homework requirements:

* Create singly linked list of playing cards (French suit) that represents the deck
* Shuffle the deck with custom algorithm
* Create stable selection sort algorithm to sort shuffled deck
