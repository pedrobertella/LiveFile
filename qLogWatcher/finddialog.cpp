#include "finddialog.h"
#include "ui_finddialog.h"

FindDialog::FindDialog(QWidget *parent, QPlainTextEdit *t) :
    QDialog(parent),
    ui(new Ui::FindDialog)
{
    ui->setupUi(this);
    edit = t;
}

FindDialog::~FindDialog()
{
    delete ui;
}

void FindDialog::findNext()
{
    QColor color = QColor(Qt::gray).lighter(130);
    if(!ui->lineText->text().isEmpty()){
        if(ui->cboxCase->isChecked()){
            if(edit->find(ui->lineText->text(), QTextDocument::FindCaseSensitively))
            {
                QTextEdit::ExtraSelection extra;
                extra.format.setBackground(color);
                extra.cursor = edit->textCursor();
                extraSelections.append(extra);
            }
        }else{
            if(edit->find(ui->lineText->text()))
            {
                QTextEdit::ExtraSelection extra;
                extra.format.setBackground(color);
                extra.cursor = edit->textCursor();
                extraSelections.append(extra);
            }
        }
    }
    edit->setExtraSelections(extraSelections);
}

void FindDialog::cancel()
{
    this->close();
}

void FindDialog::on_btnFind_clicked()
{
    findNext();
}

void FindDialog::on_btnCancel_clicked()
{
    cancel();
}
