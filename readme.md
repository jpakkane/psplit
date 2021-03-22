# Pythonesque string splitting for C++

This library aims to provide functions to split strings in roughly the
same way as Python's string splitting works.

## Features

- splits by character

- smart line splitting

- helper code to process files that uses `mmap` transparently

- the API takes only `std::string_view`s so easily works with pretty
  much any data store without the need to copy data or write
  converters

## Missing features

- This should really be implemented via coroutines

- return types should be either `std::string_view`s for performance or
  `std::string`s if a copy of the input data is needed

- Split by substring

- Split by group of chars (e.g. whitespace)

- Not hardened against file system shenanigans

- API is neither complete nor stable, do not depend on it being stable
