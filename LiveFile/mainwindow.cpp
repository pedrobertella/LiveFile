#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QPlainTextEdit>
#include <QTextStream>
#include <QFileDialog>
#include <QStandardPaths>
#include <QFontDialog>
#include <QSettings>
#include <QAbstractSlider>
#include <QStyleFactory>
#include <QStyle>
#include "finddialog.h"
#include <QMessageBox>
#include "highlighting.h"
#include "about.h"

MainWindow::MainWindow(QWidget *parent, int argc, char *argv[])
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setTheme();
    watcher=new QFileSystemWatcher(this);
    connect(qApp, SIGNAL(aboutToQuit()), this, SLOT(saveSession()));
    restoreSession();
    openArguments(argc, argv);
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
    QSettings set("Pedro Bertella", "LiveFile");
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
    QStringList filePaths = QFileDialog::getOpenFileNames(this, "Open Files...", lastLocation);
    if(!filePaths.isEmpty()){
        QFileInfo fi(filePaths.at(0));
        lastLocation = fi.absolutePath();
    }

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
    QSettings settings("Pedro Bertella", "LiveFile");
    settings.beginGroup("FontSettings");
    QFont fontSet = settings.value("font").value<QFont>();
    QPlainTextEdit *t = new QPlainTextEdit(this);
    t->setFont(fontSet);
    t->setReadOnly(true);
    t->setHidden(true);
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
    t->setHidden(false);
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
    QSettings s("Pedro Bertella", "LiveFile");
    s.beginGroup("session");
    s.setValue("openFiles", files);
    s.setValue("lastLoc", lastLocation);
    s.endGroup();
}

void MainWindow::restoreSession()
{
    QSettings s("Pedro Bertella", "LiveFile");
    s.beginGroup("session");
    QStringList files = s.value("openFiles").toStringList();
    if(s.value("lastLoc").isNull()){
        lastLocation = QStandardPaths::standardLocations(QStandardPaths::DesktopLocation).at(0);
    }else{
        lastLocation = s.value("lastLoc").toString();
    }
    s.endGroup();
    foreach(QString path, files){
        QFile file(path);
        openFile(file.fileName());
    }
}

void MainWindow::openArguments(int argc, char *argv[])
{
    for(int i = 1; i<argc; i++){
        openFile(argv[i]);
    }
}

QString MainWindow::colorText(QString text)
{
    QSettings s("Pedro Bertella", "LiveFile");
    s.beginGroup("Highlighting");
    QStringList h = s.childKeys();
    QString res = "<p style=\"background-color: "+textBkgColor+"\">"+text+"</p>";
    foreach(QString st, h){
        if(text.contains(st)){
            res = "<p style=\"background-color: "+s.value(st).toString()+";\">"+text+"</p>";
        }
    }
    s.endGroup();
    return res;
}

void MainWindow::setTheme()
{
    QSettings s("Pedro Bertella", "LiveFile");
    int theme = s.value("theme").toInt();
    if(theme==0){
        qApp->setStyle(QStyleFactory::create("windowsvista"));
        qApp->setPalette(QStyleFactory::create("windowsvista")->standardPalette());
    }else if(theme==1){
        qApp->setStyle(QStyleFactory::create("fusion"));
        qApp->setPalette(QStyleFactory::create("fusion")->standardPalette());
    }else if(theme==2){
        qApp->setStyle(QStyleFactory::create("fusion"));
        QPalette p = qApp->palette();
        p.setColor(QPalette::Window,QColor(53,53,53));
        p.setColor(QPalette::WindowText,Qt::white);
        p.setColor(QPalette::Disabled,QPalette::WindowText,QColor(127,127,127));
        p.setColor(QPalette::Base,QColor(42,42,42));
        p.setColor(QPalette::AlternateBase,QColor(66,66,66));
        p.setColor(QPalette::ToolTipBase,Qt::white);
        p.setColor(QPalette::ToolTipText,Qt::white);
        p.setColor(QPalette::Text,Qt::white);
        p.setColor(QPalette::Disabled,QPalette::Text,QColor(127,127,127));
        p.setColor(QPalette::Dark,QColor(35,35,35));
        p.setColor(QPalette::Shadow,QColor(20,20,20));
        p.setColor(QPalette::Button,QColor(53,53,53));
        p.setColor(QPalette::ButtonText,Qt::white);
        p.setColor(QPalette::Disabled,QPalette::ButtonText,QColor(127,127,127));
        p.setColor(QPalette::BrightText,Qt::red);
        p.setColor(QPalette::Link,QColor(42,130,218));
        p.setColor(QPalette::Highlight,QColor(42,130,218));
        p.setColor(QPalette::Disabled,QPalette::Highlight,QColor(80,80,80));
        p.setColor(QPalette::HighlightedText,Qt::white);
        p.setColor(QPalette::Disabled,QPalette::HighlightedText,QColor(127,127,127));
        qApp->setPalette(p);
        textBkgColor = "black";
        updateTextColor();
    }else if(theme==3){
        qApp->setStyle(QStyleFactory::create("windows"));
        qApp->setPalette(QStyleFactory::create("windows")->standardPalette());
        textBkgColor = "white";
        updateTextColor();
    }
}

void MainWindow::updateTextColor()
{
    for (int i=0;i<ui->tabWidget->count();i++) {
        fillTextField(ui->tabWidget->tabToolTip(i));
    }
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

void MainWindow::on_actionChange_Theme_triggered()
{
    QSettings s("Pedro Bertella", "LiveFile");
    int theme = s.value("theme").toInt();
    theme++;
    if(theme==4){
        theme=0;
    }
    s.setValue("theme", theme);
    setTheme();
}
