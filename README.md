# Spirographicals

[![Build Status](https://github.com/b-quarterly/spirographicals/actions/workflows/main.yml/badge.svg)](https://github.com/b-quarterly/spirographicals/actions/workflows/main.yml)
[![License](https://img.shields.io/badge/License-SPBL%201.0-blue.svg)](LICENSE.md)
[![PyPI version](https://badge.fury.io/py/spirographicals.svg)](https://badge.fury.io/py/spirographicals)

Spirographicals is a high-performance, multi-language graphing library designed for creating beautiful spirograph-style visualizations and charts. It leverages a C++ core for rendering speed, a Rust layer for memory safety and complex logic, and a user-friendly Python API for ease of use.

## Features

- **High-Performance Backend:** Core rendering written in C++ for maximum speed.
- **Safe by Design:** Data and logic layer built in Rust to prevent common memory-related bugs.
- **Pythonic API:** An easy-to-use and familiar API for data scientists and developers.
- **Customizable Visuals:** Extensive options for creating unique, spirograph-inspired art.

## Installation

You will be able to install `spirographicals` directly from PyPI once it is published:

```bash
pip install spirographicals
````

## Quick Start

Here is a basic example of how to generate a simple plot with `spirographicals`:

```python
# api/examples/simple_spirograph.py
import spirographicals.pyplot as plt
import numpy as np

# Generate data for a simple hypotrochoid
R = 10
r = 6
d = 8
t = np.linspace(0, 2 * np.pi * (r / np.gcd(r,R)), 1000)

x = (R - r) * np.cos(t) + d * np.cos(((R - r) / r) * t)
y = (R - r) * np.sin(t) - d * np.sin(((R - r) / r) * t)

# Use the spirographicals API
fig, ax = plt.subplots(figsize=(8, 8), facecolor='black')
ax.plot(x, y, color='cyan', linewidth=1.5)
ax.set_title("Hypotrochoid", color='white')

# This will eventually render the image using the Rust/C++ backend
fig.show()
```

## Building from Source

To build `spirographicals` from the source repository, you will need a complete development environment for C++, Rust, and Python.

1.  **Clone the repository:**

    ```bash
    git clone [https://github.com/b-quarterly/spirographicals.git](https://github.com/b-quarterly/spirographicals.git)
    cd spirographicals
    ```

2.  **Build the project:**
    The `Makefile` provides a simple interface to build all components in the correct order.

    ```bash
    make build
    ```

3.  **Install for development:**
    To install the package in an editable mode for active development:

    ```bash
    make install-dev
    ```

## Running Tests

To run the complete test suite (C++, Rust, Python):

```bash
make test
```

## License

This project is licensed under the Spirographicals Perpetual & Benevolent License (SPBL), Version 1.0. See the [LICENSE.md](LICENSE.md) file for details.

Copyright (c) 2025 Aitzaz Imtiaz
