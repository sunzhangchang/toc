#pragma once

#include "topper.hpp"

namespace toc {
    enum class log_type { info, warn, error };

    struct logger {
    private:
        static void log(log_type tp, const string & msg) {
            string head;
            switch (tp) {
                case log_type::info: head = "[info]"; break;
                case log_type::warn: head = "[warn]"; break;
                case log_type::error: head = "[error]"; break;
                default: break;
            }
            puts((head + ' ' + msg).c_str());
        }

    public:
        static void info(const string & msg) {
            log(log_type::info, msg);
        }

        static void warn(const string & msg) {
            log(log_type::warn, msg);
        }

        static void error(const string & msg) {
            log(log_type::error, msg);
            exit(1);
        }
    };

} // namespace toc

#ifdef DEBUG
#define debug(str) toc::logger::info(string{"------------------ "} + str)
#else
#define debug(str)
#endif
