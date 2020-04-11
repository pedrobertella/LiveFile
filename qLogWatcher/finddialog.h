#ifndef FINDDIALOG_H
#define FINDDIALOG_H

#include <QDialog>
#include <QPlainTextEdit>

namespace Ui {
class FindDialog;
}

class FindDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FindDialog(QWidget *parent = nullptr, QPlainTextEdit *t = nullptr);
    ~FindDialog();

private slots:
    void on_btnFind_clicked();

    void on_btnCancel_clicked();

private:
    Ui::FindDialog *ui;
    QPlainTextEdit *edit;
    QList<QTextEdit::ExtraSelection> extraSelections;
    void findNext();
    void cancel();
};

#endif // FINDDIALOG_H
