#include <vector>
#include <string>
#include <cstdio>
#include <sstream>
#include <process.h>
#include <filesystem>

#include <folly/FileUtil.h>

#include "include/topper.hpp"
#include "include/logger.hpp"
#include "include/utils.hpp"
#include "include/parser.hpp"

void print_help() {
    puts("usage: toc <options> <file_path>");
    puts("");
    puts("options:");
    puts("    -h --help                 Show help.");
    puts("    -v --version              Show version.");
    puts("    -O --obfuscate            Obfuscator is coming soon.");
    puts("    -m --minify               Minifying is coming soon.");
    exit(0);
}

void print_version() {
    puts("toc ver 0.1.0");
    exit(0);
}

int main(int argc, char const *argv[]) {
    std::filesystem::path file_path;

    // todo: obfuscator
//    bool obfuscate = false;

    // todo: minify
//    bool minify = false;

    if (argc == 1) {
        print_help();
        //        puts("");
        //        puts("Please input toc source file path:");
        //        file_path = toc::readline(stdin).c_str();
        //        puts(file_path.string().c_str());
    } else {
        for (int i = 1; i < argc; ++ i) {
            toc::string now = argv[i];
            if ((now == "-h") || (now == "--help")) {
                print_help();
            } else if ((now == "-v") || (now == "--version")) {
                print_version();
            }
        }
        for (int i = 1; i < argc; ++ i) {
            toc::string now = argv[i];
            if ((now == "-h") || (now == "--help")) {
            } else if ((now == "-v") || (now == "--version")) {
            } else if ((now == "-O") || (now == "--obfuscate")) {
                toc::logger::warn("Obfuscator is coming soon!");
            } else if ((now == "-m") || (now == "--minify")) {
                toc::logger::warn("Minifying is coming soon!");
            } else {
                if (now[0] == '-') {
                    toc::logger::warn(toc::string{"Unknown option \""} + now + "\"!");
                } else {
                    file_path = now.c_str();
                    if (i < argc - 1) {
                        toc::logger::warn("Option should precede the file path! The following will be ignored!");
                    }
                    break;
                }
            }
        }
    }

    if (!std::filesystem::exists(file_path)) {
        toc::logger::error("file dose not exist!");
    }

    toc::string code;

    {
        if (folly::readFile(file_path.string().c_str(), code)) {
            toc::logger::info(toc::string{"Read toc source file \""} + file_path.string().c_str() + "\" successfully.");
        } else {
            toc::logger::info(toc::string{"Read toc source file \""} + file_path.string().c_str() + "\" unsuccessfully.");
        }
        // auto fp = fopen(file_path.string().c_str(), "r");
        // char c;
        // while (~(c = fgetc(fp))) {
        //     code += c;
        // }
        // puts(code.c_str());
        // fclose(fp);
    }

    //     {
    //         toc::string code{R"(import <bits/stdc++.h>
    // #include <set>

    // using namespace std;

    // fun solve() -> void {
    //     println(123);
    // }



    // fun main() -> i32 {
    //     let T = i32{1};
    //     read(T);
    //     while (T --) {
    //         solve();
    //         let i3231 = i32{};
    //         let j123i32 = i32{};
    //     }
    //     return 0;
    // })"};
    //     }

    toc::string result = toc::Parser::parse(code);

    //    puts(result.c_str());

    // if (obfuscate) {}

    {
        auto output_path = file_path.replace_extension(".cpp");

        toc::logger::info(toc::string{"output generated cpp file \""} + output_path.string().c_str() + "\"");

        auto fp = fopen(output_path.string().c_str(), "w");
        fprintf(fp, "%s", result.c_str());
        fclose(fp);
    }

    return 0;
}
