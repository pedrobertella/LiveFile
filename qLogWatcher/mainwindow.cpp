#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QPlainTextEdit>
#include <QTextStream>
#include <QFileDialog>
#include <QStandardPaths>
#include <QFontDialog>
#include <QSettings>
#include <QAbstractSlider>
#include "finddialog.h"
#include <QMessageBox>
#include "highlighting.h"
#include "about.h"

MainWindow::MainWindow(QWidget *parent, QApplication *a)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    watcher=new QFileSystemWatcher(this);
    connect(a, SIGNAL(aboutToQuit()), this, SLOT(saveSession()));
    restoreSession();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_actionAbout_triggered()
{
    About *a = new About(this);
    a->exec();
}

void MainWindow::on_actionCopy_triggered()
{
    if(ui->tabWidget->count()>0){
        QPlainTextEdit *edit = qobject_cast<QPlainTextEdit*>(ui->tabWidget->widget(ui->tabWidget->currentIndex()));
        edit->copy();
    }
}

void MainWindow::on_actionHighlighting_triggered()
{
    Highlighting *h = new Highlighting(this);
    h->exec();
}

void MainWindow::on_actionFont_triggered()
{
    QSettings set("Pedro Bertella", "qLogWatcher");
    set.beginGroup("FontSettings");
    QFont fontSet = set.value("font").value<QFont>();
    bool ok;
    QFont font = QFontDialog::getFont(&ok, fontSet, this, "Choose Font...");
    if(ok){
        set.setValue("font", font);
        for(int i = 0;i<ui->tabWidget->count();i++){
            QPlainTextEdit *edit = qobject_cast<QPlainTextEdit*>(ui->tabWidget->widget(i));
            edit->setFont(font);
        }
    }
}

void MainWindow::on_actionOpen_triggered()
{
    QStringList filePaths = QFileDialog::getOpenFileNames(this, "Open Files...");
    foreach(QString path, filePaths){
        QFile file(path);
        openFile(file.fileName());
    }
}

void MainWindow::on_actionSave_triggered()
{
    QString fileName = QFileDialog::getSaveFileName(this, "Save File...", QStandardPaths::standardLocations(QStandardPaths::DesktopLocation).at(0));
    QFile file(fileName);
    QPlainTextEdit *edit = qobject_cast<QPlainTextEdit*>(ui->tabWidget->widget(ui->tabWidget->currentIndex()));
    if(file.open(QIODevice::WriteOnly)){
        QTextStream stream(&file);
        stream << edit->toPlainText();
    }else{
        QMessageBox::critical(this, "Save failed ", "Could not save the file, check your permissions on the path selected!");
    }
}

void MainWindow::on_actionClose_all_triggered()
{
    for(int i=ui->tabWidget->count()-1;i>-1;i--){
        watcher->removePath(ui->tabWidget->tabToolTip(i));
        ui->tabWidget->removeTab(i);
    }
}

void MainWindow::on_actionClose_file_triggered()
{
    if(ui->tabWidget->count()>0){
        watcher->removePath(ui->tabWidget->tabToolTip(ui->tabWidget->currentIndex()));
        ui->tabWidget->removeTab(ui->tabWidget->currentIndex());
    }
}

void MainWindow::on_actionExit_triggered()
{
    QApplication::quit();
}

void MainWindow::on_actionFind_triggered()
{
    if(ui->tabWidget->count()>0){
        QPlainTextEdit *edit = qobject_cast<QPlainTextEdit*>(ui->tabWidget->widget(ui->tabWidget->currentIndex()));
        edit->moveCursor(QTextCursor::Start);
        FindDialog *fd = new FindDialog(nullptr, edit);
        fd->show();
    }
}

void MainWindow::openFile(QString fileName)
{
    for(int i=0;i<ui->tabWidget->count();i++){
        if(ui->tabWidget->tabToolTip(i)==fileName){
            ui->tabWidget->setCurrentIndex(i);
            return;
        }
    }
    QSettings settings("Pedro Bertella", "qLogWatcher");
    settings.beginGroup("FontSettings");
    QFont fontSet = settings.value("font").value<QFont>();
    QPlainTextEdit *t = new QPlainTextEdit(this);
    t->setFont(fontSet);
    t->setReadOnly(true);
    connect(t, &QPlainTextEdit::selectionChanged, this, [=](){
        QList<QTextEdit::ExtraSelection> a;
        t->setExtraSelections(a);
    });
    QFileInfo fileInfo(fileName);
    QString fileShortName(fileInfo.fileName());
    ui->tabWidget->insertTab(ui->tabWidget->count(), t, QIcon(":/assets/file.png"), fileShortName);
    ui->tabWidget->setTabToolTip(ui->tabWidget->count()-1, fileName);
    watcher->addPath(fileName);
    fillTextField(fileName);
    connect(watcher, &QFileSystemWatcher::fileChanged, this, &MainWindow::fillTextField);
    ui->tabWidget->setCurrentIndex(ui->tabWidget->count()-1);
}

void MainWindow::fillTextField(QString fileName)
{
    QFile file(fileName);
    if(!file.open(QIODevice::ReadOnly))
    {
        return;
    }
    QTextStream stream(&file);
    int index = -1;
    for(int i=0;i<ui->tabWidget->count(); i++){
        if(ui->tabWidget->tabToolTip(i)==fileName){
            index = i;
        }
    }
    QPlainTextEdit *edit = qobject_cast<QPlainTextEdit*>(ui->tabWidget->widget(index));
    edit->clear();
    while(!stream.atEnd()){
        QString line = stream.readLine();
        edit->appendHtml(colorText(line));
    }
}

void MainWindow::saveSession()
{
    QStringList files;
    for(int i = 0; i<ui->tabWidget->count(); i++){
        files << ui->tabWidget->tabToolTip(i);
    }
    QSettings s("Pedro Bertella", "qLogWatcher");
    s.beginGroup("session");
    s.setValue("openFiles", files);
    s.endGroup();
}

void MainWindow::restoreSession()
{
    QSettings s("Pedro Bertella", "qLogWatcher");
    s.beginGroup("session");
    QStringList files = s.value("openFiles").toStringList();
    s.endGroup();
    foreach(QString path, files){
        QFile file(path);
        openFile(file.fileName());
    }
}

QString MainWindow::colorText(QString text)
{
    QSettings s("Pedro Bertella", "qLogWatcher");
    s.beginGroup("Highlighting");
    QStringList h = s.childKeys();
    QString res = "<p style=\"background-color: white\">"+text+"</p>";
    foreach(QString st, h){
        if(text.contains(st)){
            res = "<p style=\"background-color: "+s.value(st).toString()+";\">"+text+"</p>";
        }
    }
    s.endGroup();
    return res;
}

void MainWindow::on_actionStop_triggered()
{
    delete watcher;
    ui->actionStart->setEnabled(true);
    ui->actionStop->setEnabled(false);
}

void MainWindow::on_actionStart_triggered()
{
    watcher = new QFileSystemWatcher(this);
    for(int i = 0; i<ui->tabWidget->count(); i++){
        watcher->addPath(ui->tabWidget->tabToolTip(i));
    }
    connect(watcher, &QFileSystemWatcher::fileChanged, this, &MainWindow::fillTextField);
    ui->actionStart->setEnabled(false);
    ui->actionStop->setEnabled(true);
}
