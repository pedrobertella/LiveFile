#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileSystemWatcher>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr, QApplication *a = nullptr);
    ~MainWindow();

private slots:
    void on_actionAbout_triggered();

    void on_actionCopy_triggered();

    void on_actionHighlighting_triggered();

    void on_actionFont_triggered();

    void on_actionOpen_triggered();

    void on_actionSave_triggered();

    void on_actionClose_all_triggered();

    void on_actionClose_file_triggered();

    void on_actionExit_triggered();

    void on_actionFind_triggered();

    void saveSession();

    void on_actionStop_triggered();

    void on_actionStart_triggered();

private:
    Ui::MainWindow *ui;
    QFileSystemWatcher *watcher;
    QString lastLocation;
    void openFile(QString fileName);
    void fillTextField(QString fileName);
    void restoreSession();
    QString colorText(QString text);
};
#endif // MAINWINDOW_H
