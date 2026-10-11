#include "fileutils.h"

namespace FileUtils {

QString humanReadableSize(qint64 bytes) {
    static const char* units[] = {"B", "KiB", "MiB", "GiB", "TiB"};
    double v = static_cast<double>(bytes);
    int i = 0;
    while (v >= 1024.0 && i < 4) { v /= 1024.0; ++i; }
    return QString::number(v, 'f', i == 0 ? 0 : 2) + ' ' + units[i];
}

} // namespace FileUtils
