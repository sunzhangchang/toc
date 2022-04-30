#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <iostream>
#include <map>
#include <string>
#include <type_traits>
#include <vector>

namespace __toc_builtin {
template<class T, class...Args, std::enable_if_t<std::is_integral<T>::value, bool> = true> void read(T & x, Args&...args);
template<class T, std::enable_if_t<std::is_integral<T>::value, bool> = true> void os_print(T x);
template<class T, std::enable_if_t<std::is_integral<T>::value, bool> = true> void os_println(T x);
template<class T, std::enable_if_t<std::is_integral<T>::value, bool> = true> void os_printsp(T x);
template<class T, std::enable_if_t<std::is_integral<T>::value, bool> = true> void os_read_int(T & x);

template<class T, std::enable_if_t<std::is_integral<T>::value, bool> = true>
void os_read_int(T & x) {
    x = 0;
    bool f = false;
    char c = getchar();
    while (!isdigit(c)) f |= c == '-', c = getchar();
    while (isdigit(c)) x = (x << 3) + (x << 1) + (c ^ 48), c = getchar();
    x = (f ? -x : x);
}

template<class T, std::enable_if_t<std::is_integral<T>::value, bool> = true>
void os_print(T x) {
    if (x < 0) putchar('-'), x = -x;
    if (x > 9) os_print(x / 10);
    putchar(x % 10 + '0');
}

template<class T, std::enable_if_t<std::is_integral<T>::value, bool> = true>
void os_println(T x) {
    os_print(x);
    putchar('\n');
}

template<class T, std::enable_if_t<std::is_integral<T>::value, bool> = true>
void os_printsp(T x) {
    os_print(x);
    putchar(' ');
}

template<class T, class...Args, std::enable_if_t<std::is_integral<T>::value, bool> = true>
void os_read_int(T & x, Args&...args) {
    os_read_int(x);
    os_read_int(args...);
}

}

#define fre(NAME) freopen(#NAME".in", "r", stdin); freopen(#NAME".out", "w", stdout)

const size_t N = 1e3 + 10;

const int dx[] = {0, 0, -1, 1};
const int dy[] = {-1, 1, 0, 0};

size_t n, m, K;
int a[N][N], f[N][N]; // "global:" is optional in this version.
std::vector<int> v[N][N];
std::map<std::vector<int>, int> mp;
int64_t ans;

auto dfs(int x, int y, int c) -> void {
    f[x][y] = c;
    for (int i = 0; i < 4; ++ i) {
        auto xx = x + dx[i];
        auto yy = y + dy[i];
        if (xx <= 0 || xx > n || yy <= 0 || yy > m) {
            continue;
        }
        if (a[xx][yy] || f[xx][yy]) {
            continue;
        }
        dfs(xx, yy, c);
    }
}

auto solve() -> void {
    __toc_builtin::os_read_int(n, m, K);
    for (uint32_t i = 1; i <= n; ++ i) {
        for (uint32_t j = 1; j <= m; ++ j) {
            __toc_builtin::os_read_int(a[i][j]);
        }
    }
    auto col = int{0};
    for (uint32_t i = 1; i <= n; ++ i) {
        for (uint32_t j = 1; j <= m; ++ j) {
            if (!a[i][j] && !f[i][j]) {
                dfs(i, j, ++ col);
            }
        }
    }
    for (uint32_t i = 1; i <= n; ++ i) {
        for (uint32_t j = 1; j <= m; ++ j) {
            if (!a[i][j]) {
                continue;
            }
            auto u = std::vector<int>{};
            for (uint32_t k = 0; k < 4; ++ k) {
                auto x = i + dx[k];
                auto y = j + dy[k];
                if (f[x][y]) {
                    u.emplace_back(f[x][y]);
                }
            }
            std::sort(u.begin(), u.end());
            for (size_t k = 0; k < u.size(); ++ k) {
                if (k == 0 || u[k] != u[k - 1]) {
                    v[i][j].emplace_back(u[k]);
                }
            }
            auto o = (1 << v[i][j].size());
            for (uint32_t s = 1; s < o; ++ s) {
                auto w = std::vector<int>{};
                w.emplace_back(a[i][j]);
                for (size_t k = 0; k < v[i][j].size(); ++ k) {
                    if (s >> k & 1) {
                        w.emplace_back(v[i][j][k]);
                    }
                    ans += ((w.size() & 1) ? -1 : 1) * mp[w];
                    ++ mp[w];
                }
            }
            ([&]() {
                if (i > 1 && a[i][j] == a[i - 1][j]) {
                    ++ ans;
                    for (const auto & x : v[i][j]) {
                        for (const auto & y : v[i - 1][j]) {
                            if (x == y) {
                                -- ans;
                                return;
                            }
                        }
                    }
                }
            })();
            ([&]() {
                if (j > 1 && a[i][j] == a[i][j - 1]) {
                    ++ ans;
                    for (const auto & x : v[i][j]) {
                        for (const auto & y : v[i][j - 1]) {
                            if (x == y) {
                                -- ans;
                                return;
                            }
                        }
                    }
                }
            })();
        }
    }
    __toc_builtin::os_println(ans);
}

auto main() -> int {
    fre(link);
    auto T = uint32_t{1};
    std::string a;
    std::cin >> a;
    const auto b = 1;
    // __toc_builtin::os_read_int(T);
    while (T --) {
        solve();
    }
    return 0;
}
