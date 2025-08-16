# Custom Memory Allocator in C

> A simple thread-safe dynamic memory allocator implemented in C, mimicking `malloc`, `calloc`, `realloc`, and `free`.

[![Language: C](https://img.shields.io/badge/language-C-blue.svg)](#)
[![Build](https://img.shields.io/badge/build-Makefile-success.svg)](#)

---

## 📖 Overview

This project implements a **custom memory allocator** using `sbrk()` for heap management.  
It is designed for **educational purposes**, showing how standard functions like `malloc`, `calloc`, `realloc`, and `free` can be re-implemented at a low level.

The allocator maintains a linked list of allocated blocks, supports reusing freed memory, and ensures thread safety with a global mutex.

---

## ✨ Features

- `alloc_mem(size)` → equivalent to `malloc(size)`
- `set_mem_zero(num, size)` → equivalent to `calloc(num, size)`
- `re_alloc(ptr, size)` → equivalent to `realloc(ptr, size)`
- `release(ptr)` → equivalent to `free(ptr)`
- `print_mem_list()` → debug utility that prints the memory block list
- Thread-safe via `pthread_mutex_t`
- Minimal, modular design with clear separation of **API header**, **implementation**, and **demo**

---

## 📂 Project Structure

├── mem_alloc.c # Implementation of the allocator
├── mem_alloc.h # Public API (function prototypes)
├── main.c # Demo program using the allocator
└── Makefile # Build instructions


---

## ⚙️ Getting Started

### 🔹 Prerequisites
- GCC or Clang
- A Unix-like environment (Linux/macOS recommended)

### 🔹 Build & Run

```bash
# Compile the demo
make

# Run
./mem_alloc_demo


