/*
 * Copyright (c) 2021 Jussi Pakkanen
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <psplit.hpp>
#include <iostream>

int validate(const std::vector<std::string> &a1, const std::vector<std::string> &a2) {
    if(a1.size() != a2.size()) {
        std::cout << "Array size mismatch: " << a1.size() << " vs " << a2.size() << ".\n";
        return 1;
    }
    for(size_t i = 0; i < a1.size(); ++i) {
        if(a1[i] != a2[i]) {
            std::cout << "Index #" << i << " mismatch: " << a1[i] << " vs " << a2[i] << ".\n";
            return 1;
        }
    }
    return 0;
}

int check_splits(const std::string &input,
                 const std::vector<std::string> &truth_preserved,
                 const std::vector<std::string> &truth_drop) {
    auto preserved = psplit::split_copy(input, '\n', psplit::Empties::Preserve);
    auto dropped = psplit::split_copy(input, '\n', psplit::Empties::Drop);

    std::cout << "  preserving empties\n";
    if(validate(preserved, truth_preserved) != 0) {
        return 1;
    }
    std::cout << "  dropping empties.\n";
    return validate(dropped, truth_drop);
}

int test1() {
    std::string input{"a\nb\nc"};
    const std::vector<std::string> truth_preserve{{"a"}, {"b"}, {"c"}};
    const std::vector<std::string> truth_drop{{"a"}, {"b"}, {"c"}};

    return check_splits(input, truth_preserve, truth_drop);
}

int test2() {
    std::string input{"a\n\nb"};
    const std::vector<std::string> truth_preserve{{"a"}, {""}, {"b"}};
    const std::vector<std::string> truth_drop{{"a"}, {"b"}};

    return check_splits(input, truth_preserve, truth_drop);
}

int test3() {
    std::string input{"\na\n"};
    const std::vector<std::string> truth_preserve{{""}, {"a"}, {""}};
    const std::vector<std::string> truth_drop{{"a"}};

    return check_splits(input, truth_preserve, truth_drop);
}

int test4() {
    std::string input{"\n\n\n"};
    const std::vector<std::string> truth_preserve{{""}, {""}, {""}, {""}};
    const std::vector<std::string> truth_drop{};

    return check_splits(input, truth_preserve, truth_drop);
}

int test5() {
    std::string input{"\n"};
    const std::vector<std::string> truth_preserve{{""}, {""}};
    const std::vector<std::string> truth_drop{};

    return check_splits(input, truth_preserve, truth_drop);
}

int test6() {
    std::string input{"\n\n\nx\n\n\n"};
    const std::vector<std::string> truth_preserve{{""}, {""}, {""}, "x", {""}, {""}, {""}};
    const std::vector<std::string> truth_drop{{"x"}};

    return check_splits(input, truth_preserve, truth_drop);
}

int test7() {
    std::string input{""};
    const std::vector<std::string> truth_preserve{{""}};
    const std::vector<std::string> truth_drop{};

    return check_splits(input, truth_preserve, truth_drop);
}

int test8() {
    std::string input{"abcd"};
    const std::vector<std::string> truth_preserve{{"abcd"}};
    const std::vector<std::string> truth_drop{"abcd"};

    return check_splits(input, truth_preserve, truth_drop);
}

int test_lines() {
    std::string input1("\nhello\nworld\n");
    std::string input2("\r\nhello\r\nworld\r\n");
    const std::vector<std::string> truth{{""}, {"hello"}, {"world"}};

    auto result = psplit::split_lines_copy(input1);
    if(validate(result, truth) != 0) {
        return 1;
    }
    result = psplit::split_lines_copy(input2);
    return validate(result, truth);
}

int test_lines2() {
    std::string input1("hello\nworld");
    std::string input2("hello\r\nworld");
    const std::vector<std::string> truth{{"hello"}, {"world"}};

    auto result = psplit::split_lines_copy(input1);
    if(validate(result, truth) != 0) {
        return 1;
    }
    result = psplit::split_lines_copy(input2);
    return validate(result, truth);
}

int test_lines3() {
    std::string input1("hello\n\nworld");
    std::string input2("hello\r\n\r\nworld");
    const std::vector<std::string> truth{{"hello"}, {""}, {"world"}};

    auto result = psplit::split_lines_copy(input1);
    if(validate(result, truth) != 0) {
        return 1;
    }
    result = psplit::split_lines_copy(input2);
    return validate(result, truth);
}

int test_file() {
    std::filesystem::path path(DATADIR "input_unix.txt");
    const std::vector<std::string> truth_preserve{{"abc"}, {"def"}};

    auto result = psplit::split_file_copy(path);
    return validate(result, truth_preserve);
}

int test_file2() {
    std::filesystem::path path(DATADIR "input_dos.txt");
    const std::vector<std::string> truth_preserve{{"abc"}, {"def"}};

    auto result = psplit::split_file_copy(path);
    return validate(result, truth_preserve);
}

int test_whitespace() {
    std::string source(" hello\tthere\n everyone\r");
    const std::vector<std::string> truth_preserve{
        {""}, {"hello"}, {"there"}, {""}, {"everyone"}, {""}};
    const std::vector<std::string> truth_drop{{"hello"}, {"there"}, {"everyone"}};

    auto result = psplit::split_whitespace(source, psplit::Empties::Preserve);
    if(validate(result, truth_preserve) != 0) {
        return 1;
    }
    result = psplit::split_whitespace(source);
    return validate(result, truth_drop);
}

int main() {
    std::cout << "Test 1\n";
    if(test1() != 0) {
        return 1;
    }
    std::cout << "Test 2\n";
    if(test2() != 0) {
        return 1;
    }
    std::cout << "Test 3\n";
    if(test3() != 0) {
        return 1;
    }
    std::cout << "Test 4\n";
    if(test4() != 0) {
        return 1;
    }
    std::cout << "Test 5\n";
    if(test5() != 0) {
        return 1;
    }
    std::cout << "Test 6\n";
    if(test6() != 0) {
        return 1;
    }
    std::cout << "Test 7\n";
    if(test7() != 0) {
        return 1;
    }
    std::cout << "Test 8\n";
    if(test8() != 0) {
        return 1;
    }

    std::cout << "Test lines\n";
    if(test_lines() != 0) {
        return 1;
    }

    std::cout << "Test lines 2\n";
    if(test_lines2() != 0) {
        return 1;
    }
    std::cout << "Test lines 3\n";
    if(test_lines3() != 0) {
        return 1;
    }

    std::cout << "Test file\n";
    if(test_file() != 0) {
        return 1;
    }

    std::cout << "Test file 2\n";
    if(test_file2() != 0) {
        return 1;
    }

    std::cout << "Test whitespace\n";
    if(test_whitespace() != 0) {
        return 1;
    }
    return 0;
}
