#pragma once

#include <cctype>
#include <cstdio>

#include "topper.hpp"
#include "logger.hpp"

namespace toc {
    inline bool is_word_head(char c) {
        return (isalpha(c) || (c == '_'));
    }

    inline bool is_word_body(char c) {
        return (isalpha(c) || isdigit(c) || (c == '_'));
    }

    string readline(FILE * fp) {
        string res = "";
        char c;
        while (~(c = fgetc(fp))) {
            if (c == '\n') {
                return res;
            } else if (c == '\r') {
                c = fgetc(fp);
                if (c == '\n') {
                    return res;
                } else {
                    logger::error("Error on line separator!");
                }
            } else {
                res += c;
            }
        }
        return res;
    }
} // namespace toc
