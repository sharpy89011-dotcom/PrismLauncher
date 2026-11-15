#include "YggdrasilLoginDialog.h"

#include <QDialogButtonBox>
#include <QFormLayout>
#include <QUrl>

YggdrasilLoginDialog::YggdrasilLoginDialog(QWidget* parent) : QDialog(parent)
{
    setWindowTitle(tr("Add Yggdrasil account"));

    m_provider = new QComboBox(this);
    m_provider->addItem(tr("Ely.by"), "https://authserver.ely.by/auth");
    m_provider->addItem(tr("Custom server"), QString());

    m_server = new QLineEdit(this);
    m_server->setPlaceholderText("https://example.com/authserver");

    m_username = new QLineEdit(this);
    m_password = new QLineEdit(this);
    m_password->setEchoMode(QLineEdit::Password);

    auto buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    m_ok = buttons->button(QDialogButtonBox::Ok);

    auto form = new QFormLayout(this);
    form->addRow(tr("Provider:"), m_provider);
    form->addRow(tr("Server URL:"), m_server);
    form->addRow(tr("Username or email:"), m_username);
    form->addRow(tr("Password:"), m_password);
    form->addRow(buttons);

    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(m_provider, &QComboBox::currentIndexChanged, this, &YggdrasilLoginDialog::onProviderChanged);
    connect(m_server, &QLineEdit::textChanged, this, &YggdrasilLoginDialog::updateAcceptAllowed);
    connect(m_username, &QLineEdit::textChanged, this, &YggdrasilLoginDialog::updateAcceptAllowed);
    connect(m_password, &QLineEdit::textChanged, this, &YggdrasilLoginDialog::updateAcceptAllowed);

    onProviderChanged(0);
}

void YggdrasilLoginDialog::onProviderChanged(int index)
{
    auto preset = m_provider->itemData(index).toString();
    m_server->setText(preset);
    m_server->setEnabled(preset.isEmpty());
    updateAcceptAllowed();
}

void YggdrasilLoginDialog::updateAcceptAllowed()
{
    QUrl url(m_server->text().trimmed());
    bool urlOk = url.isValid() && url.scheme() == "https" && !url.host().isEmpty();
    m_ok->setEnabled(urlOk && !m_username->text().trimmed().isEmpty() && !m_password->text().isEmpty());
}

QString YggdrasilLoginDialog::serverUrl() const
{
    return m_server->text().trimmed();
}

QString YggdrasilLoginDialog::providerName() const
{
    return m_provider->currentIndex() == 0 ? m_provider->currentText() : QUrl(serverUrl()).host();
}

QString YggdrasilLoginDialog::username() const
{
    return m_username->text().trimmed();
}

QString YggdrasilLoginDialog::takePassword()
{
    auto pw = m_password->text();
    m_password->clear();
    return pw;
}
