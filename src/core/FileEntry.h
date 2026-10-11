#pragma once
#include <QString>
#include <QByteArray>

struct FileEntry {
    QString  path;
    qint64   size = 0;
    QByteArray miniHash;    // first 4KiB hash
    QByteArray fullHash;    // full hash (SHA-256)
};
