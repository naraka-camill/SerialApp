#ifndef SHORTCUTS_DIALOG_H
#define SHORTCUTS_DIALOG_H

#include <QDialog>
#include <QListWidgetItem>

namespace Ui {
class ShortcutsDialog;
}

struct ShortcutCommand {
    QString name;
    QString type; // "ASCII" or "HEX"
    QString data;
};

class ShortcutsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ShortcutsDialog(QWidget *parent = nullptr);
    ~ShortcutsDialog();

    QList<ShortcutCommand> getShortcuts() const;
    void setShortcuts(const QList<ShortcutCommand> &shortcuts);

private slots:
    void on_pushButton_add_clicked();
    void on_pushButton_edit_clicked();
    void on_pushButton_delete_clicked();
    void on_pushButton_import_clicked();
    void on_pushButton_export_clicked();
    void on_pushButton_ok_clicked();
    void on_pushButton_cancel_clicked();
    void on_listWidget_shortcuts_itemClicked(QListWidgetItem *item);

private:
    Ui::ShortcutsDialog *ui;
    QList<ShortcutCommand> m_shortcuts;
    int m_currentIndex;

    void updateListWidget();
    void clearForm();
    bool validateInput();
    void importFromIni(const QString &filePath);
    void exportToIni(const QString &filePath);
};

#endif // SHORTCUTS_DIALOG_H