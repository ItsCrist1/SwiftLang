#include "ICmd.h"

std::string ICmd::getFlags(const std::vector<std::string>& args) {
    std::string flags;

    for(const std::string& arg : args)
        if(arg.size() >= 2 && arg[0] == '-')
            for(size_t i=1; i < arg.size(); ++i)
                flags.push_back(arg[i]);

    return flags;
}

std::vector<std::string> ICmd::getDataArgs(const std::vector<std::string>& args) {
    std::vector<std::string> dataArgs;
    dataArgs.reserve(args.size());

    for(const std::string& arg : args)
        if(!arg.empty() && arg[0] != '-')
            dataArgs.push_back(arg);

    return dataArgs;
}

std::string ICmd::resolvePath(std::string path) {
    if(path.empty() || path[0] == '~')
        path = std::string(getenv("HOME")) + (path.empty() ? "" : path.substr(1));
    
    return std::filesystem::weakly_canonical(path).string();
}

void ICmd::changePath(const std::vector<std::string>& args, Context& context) {
    context.CurrentPath = resolvePath(!args.empty() ? args[0] : "");
    chdir(context.CurrentPath.c_str());
}

std::string ICmd::getReadableSize(double byteCount) {
    constexpr std::array UNITS = { "B", "KB", "MB", "GB", "TB", "PB" };
    constexpr int PRECISION = 2;

    size_t i=0;

    for(; byteCount >= 1024 && i < UNITS.size()-1; ++i)
        byteCount /= 1024;

    std::ostringstream out;
    out << std::fixed << std::setprecision(i == 0u ? 0 : PRECISION) << byteCount << " " << UNITS[i];
    return out.str();
}

double ICmd::getFileSize(const std::filesystem::path& path) {
    return static_cast<double>(std::filesystem::file_size(path));
}

double ICmd::getDirectorySize(const std::filesystem::path& path) {
    double byteCount = 0.0;

    for(const auto& e : std::filesystem::directory_iterator(path))
        if(e.is_regular_file())
            byteCount += getFileSize(e.path());
        else if(e.is_directory())
            byteCount += getDirectorySize(e.path());

    return byteCount;
}
