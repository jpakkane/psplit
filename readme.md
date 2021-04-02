# Pythonesque string splitting for C++

This library aims to provide C++ functions to split strings in roughly
the same way as Python's string splitting works.

For documentation read the [quick quide](quickguide.md).

## Features

- splits by character

- split by group of chars (e.g. whitespace)

- smart line splitting

- whitespace splitter helper function

- helper code to process files that uses `mmap` transparently

- Split by substring

- the API takes only `std::string_view`s so easily works with pretty
  much any data store without the need to copy data or write
  converters

- return types are either `std::string_view`s for performance or
  `std::string`s if a copy of the input data is needed

## Missing features

- This should really be implemented via coroutines

- Not hardened against file system shenanigans

- API is neither complete nor stable, do not depend on it being stable
