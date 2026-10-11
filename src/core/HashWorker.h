#pragma once
#include "FileEntry.h"
#include "CancellationToken.h"
#include <QRunnable>
#include <QObject>
#include <atomic>

class HashWorker : public QRunnable {
public:
    using Callback = std::function<void(FileEntry, bool /*fullHash*/)>;

    HashWorker(FileEntry entry,
               bool fullHash,
               const CancellationToken* token,
               Callback onDone);

    void run() override;

private:
    FileEntry entry_;
    bool fullHash_;
    const CancellationToken* token_;
    Callback onDone_;
};
