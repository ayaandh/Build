#include <iostream>
#include <string>
#include <vector>
#include <filesystem>
#include <cstdlib>
#include <algorithm>
#include <unordered_set>
#include <fstream>

namespace fs = std::filesystem;

struct Config {
    std::vector<std::string> sources;
    std::vector<std::string> compilerOptions;
    std::vector<std::string> runtimeArguments;
    std::string output;
    bool run = false;
    bool clean = false;
    bool recursive = false;
    bool verbose = false;
    bool dryRun = false;
    bool help = false;
    bool version = false;
};

bool isOption(const std::string& arg) {
    return arg.size() > 1 && arg[0] == '-';
}

bool hasWildcard(const std::string& value) {
    return value.find('*') != std::string::npos ||
           value.find('?') != std::string::npos;
}

bool wildcardMatch(const std::string& text, const std::string& pattern) {
    size_t t = 0;
    size_t p = 0;
    size_t star = std::string::npos;
    size_t match = 0;

    while (t < text.size()) {
        if (p < pattern.size() &&
            (pattern[p] == '?' || pattern[p] == text[t])) {
            ++t;
            ++p;
        } else if (p < pattern.size() && pattern[p] == '*') {
            star = p++;
            match = t;
        } else if (star != std::string::npos) {
            p = star + 1;
            t = ++match;
        } else {
            return false;
        }
    }

    while (p < pattern.size() && pattern[p] == '*')
        ++p;

    return p == pattern.size();
}

bool isCppSource(const fs::path& path) {
    std::string ext = path.extension().string();

    std::transform(
        ext.begin(),
        ext.end(),
        ext.begin(),
        [](unsigned char c) {
            return static_cast<char>(std::tolower(c));
        }
    );

    return ext == ".cpp" ||
           ext == ".cc" ||
           ext == ".cxx" ||
           ext == ".c++";
}

std::vector<std::string> expandPattern(
    const std::string& pattern,
    bool recursive
) {
    std::vector<std::string> result;

    if (!hasWildcard(pattern)) {
        fs::path path(pattern);

        if (fs::exists(path) &&
            fs::is_regular_file(path) &&
            isCppSource(path)) {
            result.push_back(path.string());
        }

        return result;
    }

    fs::path input(pattern);
    fs::path directory = input.parent_path();
    std::string filename = input.filename().string();

    if (directory.empty())
        directory = ".";

    if (!fs::exists(directory) || !fs::is_directory(directory))
        return result;

    std::error_code ec;

    if (recursive) {
        for (const auto& entry :
             fs::recursive_directory_iterator(directory, ec)) {
            if (ec)
                break;

            if (!entry.is_regular_file())
                continue;

            if (!isCppSource(entry.path()))
                continue;

            if (wildcardMatch(
                    entry.path().filename().string(),
                    filename)) {
                result.push_back(entry.path().string());
            }
        }
    } else {
        for (const auto& entry :
             fs::directory_iterator(directory, ec)) {
            if (ec)
                break;

            if (!entry.is_regular_file())
                continue;

            if (!isCppSource(entry.path()))
                continue;

            if (wildcardMatch(
                    entry.path().filename().string(),
                    filename)) {
                result.push_back(entry.path().string());
            }
        }
    }

    std::sort(result.begin(), result.end());
    return result;
}

std::string quote(const std::string& value) {
    std::string result = "\"";

    for (char c : value) {
        if (c == '"')
            result += "\\\"";
        else
            result += c;
    }

    result += "\"";
    return result;
}

std::string normalizeOutput(std::string output) {
    if (output.empty())
        return output;

    fs::path path(output);

    if (path.extension() != ".exe")
        path += ".exe";

    return path.string();
}

void printHelp() {
    std::cout
        << "build - simple C++ build tool\n\n"
        << "Usage:\n"
        << "  build <sources...> <output> [options]\n"
        << "  build <sources...> -o <output> [options]\n\n"
        << "Options:\n"
        << "  -o, --output <file>   Output executable\n"
        << "  -run, --run           Run after successful build\n"
        << "  --clean               Remove the output executable\n"
        << "  -r, --recursive       Search wildcard patterns recursively\n"
        << "  -v, --verbose         Print detailed information\n"
        << "  --dry-run             Show commands without executing them\n"
        << "  -h, --help            Show this help message\n"
        << "  --version             Show version\n"
        << "  --                    Everything after this is passed to the program\n\n"
        << "Compiler options can be passed directly:\n"
        << "  -Wall -Wextra -Wpedantic -O2 -g -std=c++20\n"
        << "  -Iinclude -DMACRO -lmylib\n\n"
        << "Examples:\n"
        << "  build main.cpp app\n"
        << "  build *.cpp app -Wall -Wextra\n"
        << "  build src/*.cpp app -std=c++23 -O2 -run\n"
        << "  build src/*.cpp -o bin/app -run -- hello 123\n"
        << "  build src/*.cpp app --clean\n";
}

bool parseArguments(int argc, char* argv[], Config& config) {
    std::vector<std::string> positional;
    bool runtimeMode = false;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];

        if (runtimeMode) {
            config.runtimeArguments.push_back(arg);
            continue;
        }

        if (arg == "--") {
            runtimeMode = true;
            continue;
        }

        if (arg == "-h" || arg == "--help") {
            config.help = true;
            continue;
        }

        if (arg == "--version") {
            config.version = true;
            continue;
        }

        if (arg == "-run" || arg == "--run") {
            config.run = true;
            continue;
        }

        if (arg == "--clean") {
            config.clean = true;
            continue;
        }

        if (arg == "-r" || arg == "--recursive") {
            config.recursive = true;
            continue;
        }

        if (arg == "-v" || arg == "--verbose") {
            config.verbose = true;
            continue;
        }

        if (arg == "--dry-run") {
            config.dryRun = true;
            continue;
        }

        if (arg == "-o" || arg == "--output") {
            if (i + 1 >= argc) {
                std::cerr << "build: missing output after " << arg << '\n';
                return false;
            }

            config.output = argv[++i];
            continue;
        }

        if (arg.rfind("--output=", 0) == 0) {
            config.output = arg.substr(9);
            continue;
        }

        if (isOption(arg)) {
            config.compilerOptions.push_back(arg);
            continue;
        }

        positional.push_back(arg);
    }

    if (config.output.empty() && positional.size() >= 2) {
        config.output = positional.back();
        positional.pop_back();
    }

    for (const auto& item : positional) {
        auto expanded = expandPattern(item, config.recursive);

        if (!expanded.empty()) {
            config.sources.insert(
                config.sources.end(),
                expanded.begin(),
                expanded.end()
            );
        } else if (
            fs::exists(item) &&
            fs::is_regular_file(item) &&
            isCppSource(item)
        ) {
            config.sources.push_back(item);
        } else if (!hasWildcard(item)) {
            std::cerr << "build: source not found: " << item << '\n';
            return false;
        }
    }

    std::sort(config.sources.begin(), config.sources.end());

    config.sources.erase(
        std::unique(
            config.sources.begin(),
            config.sources.end()
        ),
        config.sources.end()
    );

    return true;
}

void printVersion() {
    std::cout << "build 2.0.0\n";
}

bool removeOutput(const std::string& output) {
    std::error_code ec;

    if (!fs::exists(output))
        return true;

    if (!fs::remove(output, ec)) {
        std::cerr << "build: cannot remove "
                  << output
                  << ": "
                  << ec.message()
                  << '\n';
        return false;
    }

    std::cout << "removed " << output << '\n';
    return true;
}

bool ensureOutputDirectory(const fs::path& output) {
    if (!output.has_parent_path())
        return true;

    std::error_code ec;

    fs::create_directories(output.parent_path(), ec);

    if (ec) {
        std::cerr
            << "build: cannot create output directory: "
            << ec.message()
            << '\n';

        return false;
    }

    return true;
}

std::string buildCommand(const Config& config) {
    std::string command = "g++";

    for (const auto& source : config.sources)
        command += " " + quote(source);

    command += " -o " + quote(config.output);

    for (const auto& option : config.compilerOptions)
        command += " " + option;

    return command;
}

std::string runCommand(const Config& config) {
    std::string command = quote(config.output);

    for (const auto& argument : config.runtimeArguments)
        command += " " + quote(argument);

    return command;
}

int main(int argc, char* argv[]) {
    Config config;

    if (argc == 1) {
        printHelp();
        return 0;
    }

    if (!parseArguments(argc, argv, config))
        return 1;

    if (config.help) {
        printHelp();
        return 0;
    }

    if (config.version) {
        printVersion();
        return 0;
    }

    if (config.output.empty()) {
        std::cerr << "build: no output specified\n";
        std::cerr << "build: use -o <output> or place the output after the sources\n";
        return 1;
    }

    config.output = normalizeOutput(config.output);

    if (config.clean)
        return removeOutput(config.output) ? 0 : 1;

    if (config.sources.empty()) {
        std::cerr << "build: no C++ source files found\n";
        return 1;
    }

    if (!ensureOutputDirectory(config.output))
        return 1;

    if (config.verbose) {
        std::cout << "Sources:\n";

        for (const auto& source : config.sources)
            std::cout << "  " << source << '\n';

        std::cout << "\nOutput:\n"
                  << "  " << config.output << "\n\n";
    }

    std::string command = buildCommand(config);

    std::cout << command << '\n';

    if (config.dryRun) {
        if (config.run)
            std::cout << runCommand(config) << '\n';

        return 0;
    }

    int result = std::system(command.c_str());

    if (result != 0) {
        std::cerr << "build: compilation failed\n";
        return result;
    }

    std::cout << "build: successfully built "
              << config.output
              << '\n';

    if (!config.run)
        return 0;

    std::cout << "build: running "
              << config.output
              << '\n';

    result = std::system(runCommand(config).c_str());

    if (result != 0) {
        std::cerr << "build: program exited with code "
                  << result
                  << '\n';

        return result;
    }

    return 0;
}
