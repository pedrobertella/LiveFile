#ifndef HIGHLIGHTING_H
#define HIGHLIGHTING_H

#include <QDialog>

namespace Ui {
class Highlighting;
}

class Highlighting : public QDialog
{
    Q_OBJECT

public:
    explicit Highlighting(QWidget *parent = nullptr);
    ~Highlighting();

private slots:
    void on_btnRemove_clicked();

    void on_btnExit_clicked();

    void on_btnColor_clicked();

    void on_btnSave_clicked();

    void on_listWidget_currentRowChanged();

    void on_lineColor_textChanged(const QString &arg1);

private:
    Ui::Highlighting *ui;
    void initList();
};

#endif // HIGHLIGHTING_H
