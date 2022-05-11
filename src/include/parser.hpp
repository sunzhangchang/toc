#pragma once

#include <utility>

#include <folly/String.h>
#include <folly/Hash.h>

#define USE_BOOST
#include "vegex.hpp"
#undef USE_BOOST

#include "topper.hpp"
#include "logger.hpp"
#include "utils.hpp"

namespace toc {
    const flat_map<string, std::pair<string, small_vector<string, 16>>> builtin_type = {
        {"let", {"auto", {}}},

        {"i8", {"int8_t", {"<cstdint>"}}},
        {"i16", {"int16_t", {"<cstdint>"}}},
        {"i32", {"int", {}}},
        {"i64", {"int64_t", {"<cstdint>"}}},
        {"isize", {"ssize_t", {"<cstdint>"}}},

        {"u8", {"uint8_t", {"<cstdint>"}}},
        {"u16", {"uint16_t", {"<cstdint>"}}},
        {"u32", {"uint32_t", {"<cstdint>"}}},
        {"u64", {"uint64_t", {"<cstdint>"}}},
        {"usize", {"size_t", {"<cstdint>"}}},

        {"lambda", {"[&]", {}}},

        {"conster", {"const auto", {}}},
        {"readonly", {"const auto &", {}}},

        {"Vec", {"std::vector", {"<vector>"}}},
        {"String", {"std::string", {"<string>"}}},
        {"Map", {"std::map", {"<map>"}}},
        {"UnorderedMap", {"std::unordered_map", {"<unordered_map>"}}},
        {"Set", {"std::set", {"<set>"}}},
        {"Multiset", {"std::multiset", {"<set>"}}},
        {"UnorderedSap", {"std::unordered_set", {"<unordered_set>"}}},
        {"HashMap", {"__gnu_pbds::gp_hash_table", {"<ext/pb_ds/hash_policy.hpp>", "<ext/pb_ds/assoc_container.hpp>"}}},
    };

    struct BuiltinFunciont {
        string name;
        small_vector<std::pair<string, string>, 8> main;
        small_vector<string, 16> deps;
        small_vector<string, 16> headers;
    };

    // todo: Replication elimination 复制消除
    const flat_map<string, BuiltinFunciont> builtin_funcs = {
        {
            "os.readInt",
            {
                "os_read_int",
                {
                    {
                        "template<class T, std::enable_if_t<std::is_integral<T>::value, bool> = true> void os_read_int(T & x);",
                        "template<class T, std::enable_if_t<std::is_integral<T>::value, bool> = true>\n"
                        "void os_read_int(T & x) {\n"
                        "    x = 0;\n"
                        "    bool f = false;\n"
                        "    char c = getchar();\n"
                        "    while (!isdigit(c)) f |= c == '-', c = getchar();\n"
                        "    while (isdigit(c)) x = (x << 3) + (x << 1) + (c ^ 48), c = getchar();\n"
                        "    x = (f ? -x : x);\n"
                        "}"
                    },
                    {
                        "template<class T, class...Args, std::enable_if_t<std::is_integral<T>::value, bool> = true> void os_read_int(T & x, Args&...args);",
                        "template<class T, class...Args, std::enable_if_t<std::is_integral<T>::value, bool> = true>\n"
                        "void os_read_int(T & x, Args&...args) {\n"
                        "    os_read_int(x);\n"
                        "    os_read_int(args...);\n"
                        "}"
                    }
                },
                {},
                {"<cstdio>", "<type_traits>"}
            }
        },
        {
            "os.print",
            {
                "os_print",
                {
                    {
                        "template<class T, std::enable_if_t<std::is_integral<T>::value, bool> = true> void os_print(T x);",
                        "template<class T, std::enable_if_t<std::is_integral<T>::value, bool> = true>\n"
                        "void os_print(T x) {\n"
                        "    if (x < 0) putchar('-'), x = -x;\n"
                        "    if (x > 9) os_print(x / 10);\n"
                        "    putchar(x % 10 + '0');\n"
                        "}"
                    },
                },
                {},
                {"<cstdio>", "<type_traits>"}
            }
        },
        {
            "os.printsp",
            {
                "os_printsp",
                {
                    {
                        "template<class T, std::enable_if_t<std::is_integral<T>::value, bool> = true> void os_printsp(T x);",
                        "template<class T, std::enable_if_t<std::is_integral<T>::value, bool> = true>\n"
                        "void os_printsp(T x) {\n"
                        "    os_print(x);\n"
                        "    putchar(' ');\n"
                        "}"
                    },
                },
                {"os.print"},
                {"<cstdio>", "<type_traits>"}
            }
        },
        {
            "os.println",
            {
                "os_println",
                {
                    {
                        "template<class T, std::enable_if_t<std::is_integral<T>::value, bool> = true> void os_println(T x);",
                        "template<class T, std::enable_if_t<std::is_integral<T>::value, bool> = true>\n"
                        "void os_println(T x) {\n"
                        "    os_print(x);\n"
                        "    putchar('\\n');\n"
                        "}"
                    },
                },
                {"os.print"},
                {"<cstdio>", "<type_traits>"}
            }
        },
    };

    flat_set<string> refs_builtin_funcs;

    // ! At present, the parser is weak.
    class Parser {
        string code;
        string::iterator st, ed;
        string result;
        int block_level = 0;

        flat_set<string> included;
        flat_set<std::pair<string, string>> referenced_funcs;

    public:
        Parser() = default;
        Parser(const string & _code) : code(_code), st(code.begin()), ed(code.end()) {}


    private:
        string::size_type size() {
            return ed - st;
        }

        std::pair<bool, string> Whitespaces_no(string::iterator & s, string::iterator & e) {
            string res = "";
            while (s < e) {
                if (isgraph(*s)) {
                    return {true, res};
                } else {
                    res += *s;
                }
                ++ s;
            }
            return {true, res};
        }

        std::pair<bool, string> Whitespaces_no() {
            return Whitespaces_no(st, ed);
        }

        bool Whitespaces(string::iterator & s, string::iterator & e) {
            auto res = Whitespaces_no(s, e);
            if (res.first) {
                result += res.second;
            }
            return res.first;
        }

        bool Whitespaces() {
            return Whitespaces(st, ed);
        }

        std::pair<bool, string> match_symbol_no(const string & str) {
            if (size() >= str.size()) {
                string res = "";
                for (size_t i = 0; i < str.size(); ++ i) {
                    if (*(st + i) != (str[i])) {
                        return {false, ""};
                    } else {
                        res += *(st + i);
                    }
                }
                st += str.size();
                return {true, res};
            }
            return {false, ""};
        }

        bool match_symbol(const string & str) {
            auto res = match_symbol_no(str);
            if (res.first) {
                result += res.second;
            }
            return res.first;
        }

        std::pair<bool, string> match_token_no(string::iterator & s, string::iterator & e, const string & str) {
            if (size() >= str.size()) {
                string res = "";
                for (size_t i = 0; i < str.size(); ++ i) {
                    if (*(s + i) != (str[i])) {
                        return {false, ""};
                    } else {
                        res += *(s + i);
                    }
                }
                if ((s + str.size() < ed) && is_word_body(*(s + str.size()))) {
                    return {false, ""};
                }
                s += str.size();
                return {true, res};
            }
            return {false, ""};
        }

        std::pair<bool, string> match_token_no(const string & str) {
            return match_token_no(st, ed, str);
        }

        bool match_token(const string & str) {
            auto res = match_token_no(str);
            if (res.first) {
                result += res.second;
            }
            return res.first;
        }

        std::pair<bool, string> next_token_no() {
            if (!is_word_head(*st)) {
                return {false, ""};
            }
            string res = "";
            res += *st;
            ++ st;
            while (st < ed) {
                if (!is_word_body(*st)) {
                    return {true, res};
                } else {
                    res += *st;
                }
                ++ st;
            }
            return {true, res};
        }

        bool next_token() {
            auto res = next_token_no();
            if (res.first) {
                result += res.second;
            }
            return res.first;
        }

        // std::pair<bool, string> match_string_no() {
        //     if (match_symbol_no("\"")) {
        //         ;
        //     }
        //     return {false, ""};
        // }

        // bool match_string() {
        //     auto res = match_string_no();
        //     if (res.first) {
        //         result += res.second;
        //     }
        //     return res.first;
        // }

        // std::pair<bool, string> match_char() {
        //     ;
        // }

        std::pair<bool, string> match_eol_no() {
            string res = "";
            while (true) {
                if (st == code.end()) {
                    return {true, res};
                }
                if (((*st) == '\n') || ((*st) == '\r')) {
                    res += *st;
                    ++ st;
                } else {
                    return {true, res};
                }
            }
            return {true, res};
        }

        bool match_eol() {
            auto res = match_eol_no();
            if (res.first) {
                result += res.second;
            }
            return res.first;
        }

        string match_to_eol_no(string::iterator & s, string::iterator & e) {
            string res = "";
            while (true) {
                if (s == e) {
                    return res;
                }
                if (((*s) == '\n') || ((*s) == '\r')) {
                    return res;
                } else {
                    res += *s;
                    ++ s;
                }
            }
            return res;
        }

        string match_to_eol_no() {
            return match_to_eol_no(st, ed);
        }

        void match_to_eol() {
            result += match_to_eol_no();
        }

        bool is_begin_of_line() {
            return (st == code.begin()) || (*std::prev(st) == '\n') || (*std::prev(st) == '\r');
        }

        std::pair<bool, string> parse_header_no(string::iterator & s, string::iterator & e) {
            Whitespaces_no();
            if (match_symbol_no("<").first) {
                string res = "<";
                if (match_symbol_no(">").first) {
                    return {true, res + ">"};
                }
                res += *s;
                ++ s;
            }
            return {false, res};
        }

        bool parse_header() {
            auto res = parse_header_no(st, ed);
            if (res.first) {
                result += res.second;
            }
            return res.first;
        }

        bool Precompile() {
            Whitespaces();
            if (match_symbol_no("#").first) {
                logger::info("Parsing precompile command.");
                Whitespaces();
                if (match_token_no("include").first) {
                    logger::warn("Please use \"import <somthing>\" instead of \"#include <something>\"!");
                    Whitespaces();
                    included.emplace(parse_header_no().second);
                } else {
                    result += '#';
                    match_to_eol();
                }
            }
            return false;
        }

        bool Import() {
            Whitespaces();
            if (match_token_no("import").first) {
                logger::info("Parsing import.");
                // result += "#include";
                Whitespaces();
                included.emplace(parse_header_no().second);
                return true;
            }
            return false;
        }

        std::pair<bool, string> next_expression_no(string::iterator & s, string::iterator & e) {
            string res = "";
            while (s < e) {
                if (*s == ';') {
                    res += *st;
                    ++ s;
                    return {true, res};
                } else {
                    res += *s;
                    ++ s;
                }
            }
            return {false, res};
        }

        std::pair<bool, string> next_expression_no() {
            return next_expression_no(st, ed);
        }

        bool next_expression(string::iterator & s, string::iterator & e) {
            auto res = next_expression_no(s, e);
            if (res.first) {
                result += res.second;
            }
            return res.first;
        }

        bool If() {
            return false;
        }

        bool Let(string exp) {
            auto s = exp.begin();
            auto e = exp.end();
            Whitespaces(s, e);
            if (match_token_no(s, e, "let").first) {
                // logger::info("Parsing \"let\".");
                debug(exp);
                exp.replace(0, 3, "auto");
                result += exp;
                Whitespaces(s, e);
                return true;
            }
            return false;
        }

        bool Expression() {
            // logger::info("Parsing expression.");
            Whitespaces();
            auto res = next_expression_no();
            if (res.first) {
                if (Let(res.second)) {
                } else {
                    result += res.second;
                }
                Whitespaces();
                return true;
            }
            logger::error("Expression missing semicolon !");
            return false;
        }

        bool inner_statement() {
            // logger::info("Parsing inner statement.");
            while (true) {
                Whitespaces();
                if (match_symbol("}")) {
                    // logger::info("Meeting '}'.");
                    -- block_level;
                    match_eol();
                    return true;
                } else if (Expression()) {
                    match_eol();
                    continue;
                } else {
                    match_to_eol();
                    match_eol();
                    continue;
                }
            }
            return false;
        }

        bool Block() {
            if (match_symbol("{")) {
                // logger::info("Parsing block.");
                ++ block_level;
                Whitespaces();
                if (inner_statement()) {
                    return true;
                } else {
                    logger::error("Block missing '}' !");
                    return false;
                }
            };
            return false;
        }

        bool Fun() {
            Whitespaces();
            if (match_token_no("fun").first) {
                // logger::info("Parsing function.");
                result += "auto"; // todo: automatically detect function type
                Whitespaces();
                auto fun_name = next_token_no();
                if (fun_name.first) {
                    result += fun_name.second;
                } else {
                    fun_name.second = "<nameless-function>";
                }
                if (match_symbol("(")) {
                } else {
                    logger::error("Function missing '(' !");
                }
                Whitespaces();
                while (next_token()) {
                    Whitespaces();
                    if (match_symbol(",")) {
                        Whitespaces();
                        continue;
                    }
                }
                if (match_symbol(")")) {
                } else {
                    logger::error("Function missing ')' !");
                }
                Whitespaces();
                debug(*st);
                if (Block()) {
                } else {
                    if (match_symbol("->")) {
                        Whitespaces();
                        next_token();
                        Whitespaces();
                        return Block();
                    } else {
                        logger::error("Block missing '->' or '{' !");
                    }
                }
            }
            return false;
        }

        bool Global() {
            Whitespaces();
            if (match_token_no("global").first) {
                // logger::info("Parsing global variables.");
                Whitespaces_no();
                if (match_symbol_no(":").first) {
                    Whitespaces_no();
                    if (next_token()) {
                        Whitespaces();
                    } else {
                        logger::error("Missing token after ':' !");
                    }
                } else {
                    logger::error("Global variable missing explicit type annotation !");
                }
            }
            return false;
        }

        bool statement() {
            //            logger::info("Parsing statement.");
            if (Precompile()) {
                match_eol();
                return true;
            } else if (Import()) {
                match_eol();
                return true;
            } else if (Fun()) {
                match_eol();
                return true;
            } else if (Global()) {
                match_eol();
                return true;
            } else {
                match_to_eol();
                match_eol();
                return true;
            }
            return false;
        }

        bool is_operators(char c) {
            return ((c == '!') || (c == '%') || (c == '^') || (c == '&') || (c == '*') || (c == '-') || (c == '+') || (c == '=') || (c == '/') || (c == '<') || (c == '>') || (c == '~') || (c == '|'));
        }

        bool is_delimiters(char c) {
            return (!is_word_head(c) && !is_word_body(c));
            // return (!isgraph(c) || is_operators(c) || (c == ',') || (c == '.') || (c == '?') || (c == ':') || (c == '(') || (c == ')') || (c == '[') || (c == ']') || (c == '{') || (c == '}') || (c == ';'));
        }

        std::pair<bool, string> Delimiters() {
            string res = "";
            while (true) {
                if (st >= ed) {
                    return {true, res};
                }
                if (is_delimiters(*st)) {
                    res += *st;
                    ++ st;
                    return {true, res};
                } else {
                    return {false, ""};
                }
            }
            return {false, ""};
        }

        void ins_func(const BuiltinFunciont & func) {
            for (const auto & dep : func.deps) {
                ins_func(builtin_funcs.at(dep));
            }
            for (const auto & fn : func.main) {
                referenced_funcs.emplace(fn.first, fn.second);
            }
            for (const auto & x : func.headers) {
                included.emplace(x);
            }
        }

        void replace_builtin_functions() {
            logger::info("Replacing builtin function");
            for (const auto & func : builtin_funcs) {
                string outter = func.first;
                // todo: regex
                auto pos = outter.find(".", 0);
                while (pos != string::npos) {
                    outter.replace(pos, 1, "\\.");
                    pos += 2;
                    if (pos >= outter.size()) {
                        break;
                    }
                    pos = outter.find(".", pos);
                }
                // auto t = verex::verex().add(R"(\.)").search_global().search_one_line(false);
                // std::cout << t << std::endl;
                // outter = t.replace(outter.toStdString(), "\\.");
                // std::cerr << outter << std::endl;
                auto expr = verex::verex().add((string{"(?<=^|(?<=\\W))("} + outter + string{")(?=\\W)"}).toStdString()).search_global().search_one_line(false);
                if (expr.test(result.toStdString())) {
                    result = expr.replace(result.toStdString(), (string{"__toc_builtin::"} + func.second.name).toStdString());
                    refs_builtin_funcs.emplace(func.first);
                }
            }
            for (const auto & outter : refs_builtin_funcs) {
                ins_func(builtin_funcs.at(outter));
            }
        }

        void replace_builtin_type() {
            logger::info("Replacing builtin type");
            for (const auto & tp : builtin_type) {
                auto expr = verex::verex().add((string{"(?<=^|(?<=\\W))("} + tp.first + string{")(?=\\W)"}).toStdString()).search_global().search_one_line(false);
                if (expr.test(result.toStdString())) {
                    result = expr.replace(result.toStdString(), tp.second.first.toStdString());
                    for (const auto & x : tp.second.second) {
                        included.emplace(x);
                    }
                }
            }
        }

        // todo: Replication elimination 复制消除
        void insert_includes() {
            logger::info("Insert includes");
            string inc = "";
            for (const auto & x : included) {
                inc += string{"#include "}.append(folly::trimWhitespace(x).toString()) + '\n';
            }
            result = inc + '\n' + result;
        }

        void insert_functions() {
            logger::info("Insert builtin functions");
            if (referenced_funcs.size() == 0) {
                logger::info("No builtin functions are referenced.");
                return;
            }
            string inc = "namespace __toc_builtin {\n";
            const string tab = "    ";
            for (const auto & func : referenced_funcs) {
                inc += folly::trimWhitespace(func.first).toString() + "\n";
            }
            inc += '\n';
            for (const auto & func : referenced_funcs) {
                inc += folly::trimWhitespace(func.second).toString() + "\n\n";
            }
            inc.pop_back();
            result = inc + "}\n\n" + result;
        }

        void trim_whitelines() {
            logger::info("Trimming whitelines");
            auto expr = verex::verex().linebreak().add("( |\\t)*").linebreak().add("( |\\t)*").linebreak().search_global().search_one_line(false);
            while (expr.test(result.toStdString())) {
                result = expr.replace(result.toStdString(), "\n\n");
            }
            result = folly::trimWhitespace(result).toString();
            result += '\n';
        }

    public:
        string parse() {
            while (st < ed) {
                if (statement()) {
                } else {
                    logger::error("Parsing statement error!");
                }
            }
            replace_builtin_functions();
            insert_functions();
            replace_builtin_type();
            insert_includes();
            trim_whitelines();
            return result;
        }

        static string parse(const string & code) {
            return Parser(code).parse();
        }
    };
} // namespace toc
