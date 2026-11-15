#pragma once
#include <QObject>

#include "minecraft/auth/AuthStep.h"
#include "net/NetJob.h"
#include "net/Request.h"

class YggdrasilAuthenticateStep : public AuthStep {
    Q_OBJECT

   public:
    explicit YggdrasilAuthenticateStep(AccountData* data, QString username = QString(), QString password = QString());
    virtual ~YggdrasilAuthenticateStep() noexcept = default;

    void perform() override;
    QString describe() override;

   private slots:
    void onRequestDone(QByteArray* response);

   private:
    QString m_username;
    QString m_password;
    bool m_refresh = false;
    Net::Request::Ptr m_request;
    NetJob::Ptr m_task;
};
