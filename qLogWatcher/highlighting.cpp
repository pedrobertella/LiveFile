#include "highlighting.h"
#include "ui_highlighting.h"
#include <QSettings>
#include <QColorDialog>

Highlighting::Highlighting(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Highlighting)
{
    ui->setupUi(this);
    initList();
    setWindowFlags(Qt::Window | Qt::WindowTitleHint | Qt::CustomizeWindowHint);
}

Highlighting::~Highlighting()
{
    delete ui;
}

void Highlighting::on_btnRemove_clicked()
{
    if(!ui->listWidget->selectedItems().isEmpty()){
        delete ui->listWidget->selectedItems().at(0);
    }
}

void Highlighting::on_btnExit_clicked()
{
    QSettings s("Pedro Bertella", "qLogWatcher");
    s.beginGroup("Highlighting");
    s.remove("");
    s.endGroup();
    s.beginGroup("Highlighting");
    for(int i = 0;i<ui->listWidget->count();i++){
        s.setValue(ui->listWidget->item(i)->text(), ui->listWidget->item(i)->data(Qt::UserRole));
    }
    s.endGroup();
    this->accept();
}

void Highlighting::on_btnColor_clicked()
{
    QColor color = QColorDialog::getColor(Qt::white, this, "Choose color...");
    ui->lineColor->setText(color.name());
}

void Highlighting::on_btnSave_clicked()
{
    int index = -1;
    for(int i = 0;i<ui->listWidget->count();i++){
        if(ui->listWidget->item(i)->text()==ui->lineText->text()){
            index = i;
        }
    }
    if(index==-1){
        QListWidgetItem *item = new QListWidgetItem(ui->listWidget);
        item->setText(ui->lineText->text());
        item->setData(Qt::UserRole, ui->lineColor->text());
    }else{
        QListWidgetItem *item = ui->listWidget->item(index);
        item->setText(ui->lineText->text());
        item->setData(Qt::UserRole, ui->lineColor->text());
    }
    ui->lineText->clear();
    ui->lineColor->clear();
}

void Highlighting::on_listWidget_currentRowChanged()
{
    if(ui->listWidget->currentItem()!=nullptr){
        ui->lineText->setText(ui->listWidget->currentItem()->text());
        ui->lineColor->setText(ui->listWidget->currentItem()->data(Qt::UserRole).toString());
    }

}

void Highlighting::initList()
{
    QSettings s("Pedro Bertella", "qLogWatcher");
    s.beginGroup("Highlighting");
    QStringList list = s.childKeys();
    foreach(QString t, list){
        QListWidgetItem *item = new QListWidgetItem(ui->listWidget);
        item->setText(t);
        item->setData(Qt::UserRole, s.value(t).toString());
        ui->listWidget->addItem(item);
    }
}

void Highlighting::on_lineColor_textChanged(const QString &arg1)
{
    QPixmap pixmap(100,100);
    pixmap.fill(QColor(arg1));
    ui->labelColor->setPixmap(pixmap);
    ui->linePreview->setStyleSheet("background-color: "+arg1+";");
}
