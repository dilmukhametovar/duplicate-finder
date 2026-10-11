#pragma once
#include <QMainWindow>
#include "core/FileEntry.h"

QT_BEGIN_NAMESPACE
class QLineEdit;
class QPushButton;
class QPushButton;
class QPushButton;
class QProgressBar;
class QLabel;
class QTreeView;
QT_END_NAMESPACE

class DuplicateScanner;
class DuplicateGroupsModel;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void chooseFolder();
    void startScan();
    void onProgress(int scanned, int total, qint64 bytes);
    void onGroupFound(const QVector<FileEntry>& group);
    void onFinished(int groups, qint64 wasted);

private:
    QLineEdit*   pathEdit_   = nullptr;
    QPushButton* browseBtn_  = nullptr;
    QPushButton* scanBtn_    = nullptr;
    QPushButton* cancelBtn_  = nullptr;
    QProgressBar* progressBar_ = nullptr;
    QLabel*      statusLabel_  = nullptr;
    QTreeView*   treeView_   = nullptr;

    DuplicateScanner*     scanner_ = nullptr;
    DuplicateGroupsModel* model_   = nullptr;
};
