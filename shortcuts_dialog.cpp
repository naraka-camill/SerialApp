#include "shortcuts_dialog.h"
#include "ui_shortcuts_dialog.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QSettings>

ShortcutsDialog::ShortcutsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ShortcutsDialog),
    m_currentIndex(-1)
{
    ui->setupUi(this);
    clearForm();
}

ShortcutsDialog::~ShortcutsDialog()
{
    delete ui;
}

QList<ShortcutCommand> ShortcutsDialog::getShortcuts() const
{
    return m_shortcuts;
}

void ShortcutsDialog::setShortcuts(const QList<ShortcutCommand> &shortcuts)
{
    m_shortcuts = shortcuts;
    updateListWidget();
}

void ShortcutsDialog::on_pushButton_add_clicked()
{
    // 确保添加模式下m_currentIndex为-1
    m_currentIndex = -1;
    if (!validateInput()) {
        return;
    }

    ShortcutCommand cmd;
    cmd.name = ui->lineEdit_name->text();
    cmd.type = ui->comboBox_type->currentText();
    cmd.data = ui->lineEdit_data->text();

    m_shortcuts.append(cmd);
    updateListWidget();
    clearForm();
}

void ShortcutsDialog::on_pushButton_edit_clicked()
{
    if (m_currentIndex < 0 || m_currentIndex >= m_shortcuts.size()) {
        QMessageBox::warning(this, "编辑失败", "请先选择要编辑的指令");
        return;
    }

    if (!validateInput()) {
        return;
    }

    ShortcutCommand &cmd = m_shortcuts[m_currentIndex];
    cmd.name = ui->lineEdit_name->text();
    cmd.type = ui->comboBox_type->currentText();
    cmd.data = ui->lineEdit_data->text();

    updateListWidget();
    clearForm();
}

void ShortcutsDialog::on_pushButton_delete_clicked()
{
    if (m_currentIndex < 0 || m_currentIndex >= m_shortcuts.size()) {
        QMessageBox::warning(this, "删除失败", "请先选择要删除的指令");
        return;
    }

    m_shortcuts.removeAt(m_currentIndex);
    updateListWidget();
    clearForm();
}

void ShortcutsDialog::on_pushButton_import_clicked()
{
    QString filePath = QFileDialog::getOpenFileName(this, "导入快捷指令", "", "INI文件 (*.ini)");
    if (filePath.isEmpty()) {
        return;
    }

    importFromIni(filePath);
    updateListWidget();
}

void ShortcutsDialog::on_pushButton_export_clicked()
{
    QString filePath = QFileDialog::getSaveFileName(this, "导出快捷指令", "shortcuts.ini", "INI文件 (*.ini)");
    if (filePath.isEmpty()) {
        return;
    }

    exportToIni(filePath);
}

void ShortcutsDialog::on_pushButton_ok_clicked()
{
    accept();
}

void ShortcutsDialog::on_pushButton_cancel_clicked()
{
    reject();
}

void ShortcutsDialog::on_listWidget_shortcuts_itemClicked(QListWidgetItem *item)
{
    m_currentIndex = ui->listWidget_shortcuts->row(item);
    if (m_currentIndex < 0 || m_currentIndex >= m_shortcuts.size()) {
        return;
    }

    const ShortcutCommand &cmd = m_shortcuts[m_currentIndex];
    ui->lineEdit_name->setText(cmd.name);
    ui->comboBox_type->setCurrentText(cmd.type);
    ui->lineEdit_data->setText(cmd.data);
}

void ShortcutsDialog::updateListWidget()
{
    ui->listWidget_shortcuts->clear();
    for (const ShortcutCommand &cmd : m_shortcuts) {
        QListWidgetItem *item = new QListWidgetItem(cmd.name);
        item->setToolTip(QString("类型: %1\n数据: %2").arg(cmd.type).arg(cmd.data));
        ui->listWidget_shortcuts->addItem(item);
    }
}

void ShortcutsDialog::clearForm()
{
    ui->lineEdit_name->clear();
    ui->comboBox_type->setCurrentIndex(0);
    ui->lineEdit_data->clear();
    m_currentIndex = -1;
}

bool ShortcutsDialog::validateInput()
{
    QString name = ui->lineEdit_name->text();
    if (name.isEmpty()) {
        QMessageBox::warning(this, "输入错误", "请输入指令名称");
        return false;
    }

    // 检查名称是否已存在
    for (int i = 0; i < m_shortcuts.size(); ++i) {
        if (i == m_currentIndex) { // 编辑模式下，跳过当前正在编辑的项
            continue;
        }
        if (m_shortcuts[i].name == name) {
            QMessageBox::warning(this, "输入错误", "指令名称已存在，请使用其他名称");
            return false;
        }
    }

    if (ui->lineEdit_data->text().isEmpty()) {
        QMessageBox::warning(this, "输入错误", "请输入指令数据");
        return false;
    }

    if (ui->comboBox_type->currentText() == "HEX") {
        QString data = ui->lineEdit_data->text();
        QStringList hexList = data.split(" ");
        for (const QString &hex : hexList) {
            if (hex.isEmpty()) {
                continue;
            }
            bool ok;
            hex.toUInt(&ok, 16);
            if (!ok) {
                QMessageBox::warning(this, "输入错误", "HEX格式数据无效");
                return false;
            }
        }
    }

    return true;
}

void ShortcutsDialog::importFromIni(const QString &filePath)
{
    QSettings settings(filePath, QSettings::IniFormat);
    settings.beginGroup("Shortcuts");
    QStringList keys = settings.childKeys();

    m_shortcuts.clear();
    for (const QString &key : keys) {
        QString value = settings.value(key).toString();
        QStringList parts = value.split("|");
        if (parts.size() >= 2) {
            ShortcutCommand cmd;
            cmd.name = key;
            cmd.type = parts[0];
            cmd.data = parts.size() > 1 ? parts[1] : "";
            m_shortcuts.append(cmd);
        }
    }
    settings.endGroup();
}

void ShortcutsDialog::exportToIni(const QString &filePath)
{
    QSettings settings(filePath, QSettings::IniFormat);
    settings.beginGroup("Shortcuts");
    settings.remove(""); // 清空现有内容

    for (const ShortcutCommand &cmd : m_shortcuts) {
        QString value = QString("%1|%2").arg(cmd.type).arg(cmd.data);
        settings.setValue(cmd.name, value);
    }

    settings.endGroup();
}