#pragma once

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

#ifdef _WIN32
#include <windows.h>
#else
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#endif

#include <string_view>
#include <vector>
#include <string>
#include <filesystem>

namespace psplit {

enum class Empties : char { Preserve, Drop };

#ifdef _WIN32
class MmapFile final {
public:
    explicit MmapFile(const std::filesystem::path &fname) {
        filehandle = CreateFile(fname.wstring().c_str(),
                                GENERIC_READ,
                                0,
                                nullptr,
                                OPEN_EXISTING,
                                FILE_ATTRIBUTE_NORMAL,
                                nullptr);
        mappinghandle = CreateFileMapping(filehandle, nullptr, PAGE_READONLY, 0, 0, nullptr);
    }

    std::string_view view() const noexcept {
        LARGE_INTEGER fs;
        GetFileSizeEx(filehandle, &fs);
        auto *addr = static_cast<char *>(MapViewOfFile(mappinghandle, FILE_MAP_READ, 0, 0, 0));
        if(!addr) {
            return std::string_view{};
        }
        return std::string_view(addr, fs.QuadPart);
    }

    ~MmapFile() { unmap(); }

    MmapFile() = delete;
    MmapFile(const MmapFile &) = delete;
    MmapFile(MmapFile &&o) noexcept : filehandle(o.filehandle), mappinghandle(o.mappinghandle) {
        o.filehandle = o.mappinghandle = nullptr;
    }
    MmapFile &operator=(const MmapFile &) = delete;
    MmapFile &operator=(MmapFile &&o) noexcept {
        if(this != &o) {
            unmap();
            filehandle = o.filehandle;
            mappinghandle = o.mappinghandle;
            o.filehandle = o.mappinghandle = nullptr;
        }
        return *this;
    }

private:
    void unmap() {
        if(mappinghandle) {
            CloseHandle(mappinghandle);
            CloseHandle(filehandle);
            filehandle = mappinghandle = nullptr;
        }
    }
    HANDLE filehandle;
    HANDLE mappinghandle;
};

#else

class MmapFile final {
public:
    explicit MmapFile(const std::filesystem::path &fname) {
        // FIXME, handle failures
        fd = open(fname.c_str(), O_RDONLY | O_CLOEXEC);
        map_size = lseek(fd, 0, SEEK_END);
        map = mmap(nullptr, map_size, PROT_READ, MAP_PRIVATE, fd, 0);
    }

    std::string_view view() const noexcept {
        return std::string_view(static_cast<char *>(map), map_size);
    }

    ~MmapFile() { unmap(); }

    MmapFile() = delete;
    MmapFile(const MmapFile &) = delete;
    MmapFile(MmapFile &&o) noexcept : fd(o.fd), map_size(o.map_size), map(o.map) {
        o.fd = 0;
        o.map_size = 0;
        o.map = nullptr;
    }
    MmapFile &operator=(const MmapFile &) = delete;
    MmapFile &operator=(MmapFile &&o) noexcept {
        if(this != &o) {
            unmap();
            fd = o.fd;
            map_size = o.map_size;
            map = o.map;
            o.fd = 0;
            o.map_size = 0;
            o.map = nullptr;
        }
        return *this;
    }

private:
    void unmap() {
        if(map) {
            munmap(map, map_size);
            close(fd);
            fd = 0;
            map_size = 0;
            map = nullptr;
        }
    }
    int fd;
    size_t map_size;
    void *map;
};

#endif

inline void
add_piece(std::vector<std::string_view> &words, std::string_view substr, const Empties e) noexcept {
    if(e == Empties::Drop && substr.empty()) {
        return;
    }
    words.push_back(substr);
}

inline std::vector<std::string_view> split(std::string_view input,
                                           const std::string_view split_chrs,
                                           const Empties e = Empties::Drop) noexcept {
    std::vector<std::string_view> words;
    std::string_view::size_type current = 0;
    if(input.empty()) {
        add_piece(words, input.substr(0, 0), e);
        return words;
    }
    if(split_chrs.empty()) {
        for(size_t i = 0; i < input.size(); ++i) {
            add_piece(words, input.substr(i, 1), e);
        }
        return words;
    }
    const auto input_size = input.size();
    while(current != std::string_view::npos && current != input_size) {
        if(split_chrs.find(input[current]) != std::string_view::npos) {
            if(current == 0) {
                add_piece(words, input.substr(current, 0), e);
            } else {
                if(split_chrs.find(input[current - 1]) != std::string_view::npos) {
                    add_piece(words, input.substr(current, 0), e);
                }
            }
            ++current;
        } else {
            auto end = input.find_first_of(split_chrs, current);
            if(end == std::string_view::npos) {
                add_piece(words, input.substr(current, end), e);
            } else {
                add_piece(words, input.substr(current, end - current), e);
            }
            current = end;
        }
    }
    if(split_chrs.find(input.back()) != std::string_view::npos) {
        add_piece(words, input.substr(input_size, 0), e);
    }
    return words;
}

inline std::vector<std::string_view> split_substr(std::string_view input,
                                                  const std::string_view split_sub,
                                                  const Empties e = Empties::Drop) noexcept {
    if(input.empty() || split_sub.empty()) {
        return split(input, split_sub, e);
    }
    std::vector<std::string_view> words;
    while(!input.empty()) {
        auto loc = input.find(split_sub);
        auto cur_word = input.substr(0, loc);
        add_piece(words, cur_word, e);
        if(loc == std::string_view::npos) {
            input = input.substr(0, 0);
        } else if(loc + split_sub.length() == input.length()) {
            input = input.substr(0, 0);
            add_piece(words, input, e);
        } else {
            input = input.substr(loc + split_sub.length(), std::string_view::npos);
        }
    }
    return words;
}

inline std::vector<std::string> split_copy(std::string_view input,
                                           std::string_view split_chrs,
                                           const Empties e = Empties::Drop) noexcept {
    auto views = split(input, split_chrs, e);
    std::vector<std::string> copies;
    copies.reserve(views.size());
    for(const auto &v : views) {
        copies.emplace_back(v);
    }
    return copies;
}

inline std::vector<std::string> split_substr_copy(std::string_view input,
                                                  std::string_view split_sub,
                                                  const Empties e = Empties::Drop) noexcept {
    auto views = split_substr(input, split_sub, e);
    std::vector<std::string> copies;
    copies.reserve(views.size());
    for(const auto &v : views) {
        copies.emplace_back(v);
    }
    return copies;
}

inline std::vector<std::string> split_copy(std::string_view input,
                                           char split_chr = '\n',
                                           const Empties e = Empties::Drop) noexcept {
    char buf[2] = {split_chr, '\0'};
    return split_copy(input, buf, e);
}

inline std::vector<std::string> split_whitespace(std::string_view input,
                                                 const Empties e = Empties::Drop) noexcept {
    const char *whitespaces = " \n\r\t";
    return split_copy(input, whitespaces, e);
}

inline std::vector<std::string_view> split_lines(std::string_view data) noexcept {
    std::vector<std::string_view> lines;
    std::string_view newlines("\r\n");
    size_t current = 0;
    if(data.empty()) {
        return lines;
    }
    while(current < data.size()) {
        auto end = data.find_first_of(newlines, current);
        if(current == end) {
            lines.emplace_back("");
        } else if(end == std::string_view::npos) {
            lines.emplace_back(data.substr(current, end));
            end = data.size() - 1;
        } else {
            lines.emplace_back(data.substr(current, end - current));
        }

        if(data[end] == '\r' && end + 1 < data.size() && data[end + 1] == '\n') {
            current = end + 2;
        } else {
            current = end + 1;
        }
    }
    return lines;
}

inline std::vector<std::string> split_lines_copy(std::string_view data) noexcept {
    std::vector<std::string> lines;
    auto line_views = split_lines(data);
    lines.reserve(line_views.size());
    for(const auto &s : line_views) {
        lines.emplace_back(s);
    }
    return lines;
}

inline std::vector<std::string> split_file_copy(const std::filesystem::path &path) noexcept {
    MmapFile mf(path);
    return split_lines_copy(mf.view());
}
} // namespace psplit
