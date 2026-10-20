#pragma once
#include "FileEntry.h"
#include "CancellationToken.h"
#include <QElapsedTimer>
#include <QHash>
#include <QMutex>
#include <QObject>
#include <QString>
#include <QVector>

class DuplicateScanner : public QObject {
    Q_OBJECT
public:
    explicit DuplicateScanner(QObject* parent = nullptr);

    void start(const QString& rootDir, int threadCount = 0);
    void cancel();

signals:
    void progress(int filesScanned, int filesTotal, qint64 bytesRead);
    void groupFound(const QVector<FileEntry>& group);
    void finished(int totalGroups, qint64 totalSize);
    void failed(const QString& error);

private:
    void onHashed(FileEntry entry, bool fullHash);

    CancellationToken token_;
    QMutex mutex_;

    QHash<QByteArray, QVector<FileEntry>> byMiniHash_;
    QHash<QByteArray, QVector<FileEntry>> byFullHash_;

    int  pending_       = 0;
    enum Stage : int {
        IDLE = 0,
        MINI = 1,
        FULL = 2,
    } stage_;
    int  totalFiles_    = 0;
    int  scannedFiles_  = 0;
};
