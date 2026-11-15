#pragma once

#include <QComboBox>
#include <QDialog>
#include <QLineEdit>
#include <QPushButton>

// Collects provider, server URL, username and password for a Yggdrasil-compatible login.
// The password is read once via takePassword() and cleared from the widget.
class YggdrasilLoginDialog final : public QDialog {
    Q_OBJECT

   public:
    explicit YggdrasilLoginDialog(QWidget* parent = nullptr);

    QString serverUrl() const;
    QString providerName() const;
    QString username() const;
    QString takePassword();

   private slots:
    void onProviderChanged(int index);
    void updateAcceptAllowed();

   private:
    QComboBox* m_provider;
    QLineEdit* m_server;
    QLineEdit* m_username;
    QLineEdit* m_password;
    QPushButton* m_ok;
};
