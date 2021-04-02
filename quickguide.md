# User guide for the `psplit` C++ library

Psplit is a simple library that provides helper functions for
splitting strings. The library aims to be practical and easy to use
rather than supporting every possible setup.

The splitting algorithms mimic the behaviour of Python's splitting
functionality whenever possible.

## Installing and using

Psplit is a single header library. Projects that build with the Meson
build system can easily use it as a [Meson
subproject](https://nibblestew.blogspot.com/2021/03/writing-library-and-then-using-it-as.html). Other
build systems can use it by, for example, copying the `psplit.hpp`
inside their own project.

# Data model

The basic model of the library is the following:

> `std::string_view`s go in, `std::vector<std::string_view>`s come out

This makes it possible to create split text without needing to copy
the bytes. This has the downside that it is the user's responsibility
to keep the original data around for as long as they are using the
result.

Psplit also provides a version of all splitting functions that return
copies of the original data in `std::vector<std::string>`. These
functions are distinguished by having the suffix `_copy` in their
name. So for example there is a `split` function that returns string
views and a `split_copy` function that behaves otherwise identically
but returns full blown strings instead.

## Basic usage

### Whitespace splitting

Perhaps the most common case is splitting text by whitespace.

```cpp
std::string input("one\ntwo    three");
std::vector<std::string_view> words = psplit::split_whitespace(input);
// words contains "one", "two", "three"
```

The behaviour is identical to Python:

```python
>>> 'one\ntwo  three'.split()
['one', 'two', 'three']
```

### Line splitting

Another common task is splitting text into lines. This is not as easy
as it sounds, as input data can contain either unix-style line endings
(`\n`) or dos-style line endings (`\r\n`). The line splitter in Psplit
handles both transparently:

```cpp
std::string lines("line1\n\nline3");
std::vector<std::string_view> lines = psplit::split_lines(lines);
// lines contains "line1", "", "line3"
```

As can be seen, this function preserves empty lines. Compared to
Python, there is one notable difference. If you do this:

```python
for line in open(filename):
   # use lines here
```

then the `line` variable will contain the end-of-line
character. Psplit's line splitter removes that character. This is done
so that regardless of whether you data has unix or dos line endings,
the output is identical.

### File splitting

Psplit has a function to split files into lines.

```cpp
std::vector<std::string> lines = split_file_copy("datafile.txt");
```

This function will use memory mapped files behind the scenes for
efficiency. Because of this it will always return a copy of the data.

### Splitting by substring

Data that is packed with a multi-character separator can be split like this:

```cpp
std::string data("oneBREAKtwoBREAKthreeBREAK");
std::vector<std::string_view> words = split_substr(data, "BREAK");
// words contains "one", "two", "three"
```

### Splitting by character classes

Sometimes the data can contain multiple different separator. This is a
job for the basic `split` function.

```cpp
std::string data("one,two;three-four");
std::vector<std::string_view> words = psplit::split(data "-;,");
// words contains "one", "two", "three", "four"
```

## Full reference

### Enums

```cpp
enum class Empties : char {
    Preserve,  // Keep empty words in the output
    Drop };    // Discard empty words from output
```

The difference between the two is what happens when splitting would
create an empty string:

```cpp
std::string data(",one,two,,three,");
auto all      = psplit::split(data, ",", psplit::Empties::Drop);
auto filtered = psplit::split(data, ",", psplit::Empties::Keep);

// all contains      "", "one", "two", "", "three", ""
// filtered contains "one", "two", "three"
```

### Functions

```cpp
std::vector<std::string_view> split(std::string_view input,
                                    std::string_view split_chrs,
                                    const Empties e = Empties::Drop) noexcept
```

Splits the given input. All characters in `split_chrs` are considered
splitting characters and everything else is payload. To split text on
comma, for example, you'd call it like this `auto result =
psplit::split(input, ",");`

```cpp
std::vector<std::string_view> split_substr(std::string_view input,
                                           const std::string_view split_sub,
                                           const Empties e = Empties::Drop) noexcept
```

Splits text on places that contain the given substring.

```cpp
std::vector<std::string> split_whitespace(std::string_view input,
                                          const Empties e = Empties::Drop) noexcept
```

Splits the input data on ASCII whitespace, that is, on space, tab,
'\r' and '\n'.

```cpp
std::vector<std::string_view> split_lines(std::string_view data) noexcept
```

Splits the input text into lines.  Handles `\r\n` as a single line
separator, so the input can contain either unix or dos line
endings. The output lines do not contain the line ending character.

```cpp
std::vector<std::string> split_file_copy(const std::filesystem::path &path) noexcept
```

Splits the contents of the given file into lines. Note that there is
no version of this function that would return a view.

```cpp
std::vector<std::string> split_copy(std::string_view input,
                                    char split_chr = '\n',
                                    const Empties e = Empties::Drop) noexcept
```

Like `split` but returns a copy of the data.

```cpp
std::vector<std::string> split_substr_copy(std::string_view input,
                                           std::string_view split_sub,
                                           const Empties e = Empties::Drop) noexcept
```

Like `split_substr` but returns a copy of the data.

```cpp
std::vector<std::string> split_lines_copy(std::string_view data) noexcept
```

Like `split_lines` but returns a copy of the data.
