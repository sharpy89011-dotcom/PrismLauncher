#include "YggdrasilAuthenticateStep.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QUuid>

#include "Application.h"
#include "net/NetUtils.h"
#include "net/RawHeaderProxy.h"

YggdrasilAuthenticateStep::YggdrasilAuthenticateStep(AccountData* data, QString username, QString password)
    : AuthStep(data), m_username(std::move(username)), m_password(std::move(password))
{
    m_refresh = m_password.isEmpty();
}

QString YggdrasilAuthenticateStep::describe()
{
    return m_refresh ? tr("Refreshing the session with %1.").arg(m_data->yggdrasilProviderName)
                     : tr("Logging in with %1.").arg(m_data->yggdrasilProviderName);
}

void YggdrasilAuthenticateStep::perform()
{
    QString base = m_data->yggdrasilServerUrl;
    if (!base.startsWith("https://")) {
        emit finished(AccountTaskState::STATE_FAILED_HARD, tr("Authentication server must use HTTPS."));
        return;
    }
    while (base.endsWith('/'))
        base.chop(1);

    if (m_data->yggdrasilClientToken.isEmpty())
        m_data->yggdrasilClientToken = QUuid::createUuid().toString(QUuid::Id128);

    QJsonObject body;
    body["clientToken"] = m_data->yggdrasilClientToken;
    if (m_refresh) {
        body["accessToken"] = m_data->yggdrasilToken.token;
    } else {
        QJsonObject agent;
        agent["name"] = "Minecraft";
        agent["version"] = 1;
        body["agent"] = agent;
        body["username"] = m_username;
        body["password"] = m_password;
    }
    body["requestUser"] = true;

    QUrl url(base + (m_refresh ? "/refresh" : "/authenticate"));
    auto headers = QList<Net::HeaderPair>{ { "Content-Type", "application/json" }, { "Accept", "application/json" } };
    auto payload = QJsonDocument(body).toJson(QJsonDocument::Compact);

    // Drop the password from memory as soon as the body is built.
    m_password.fill(QChar('\0'));
    m_password.clear();

    auto [request, response] = Net::Request::makeByteArray(url, payload);
    m_request = request;
    m_request->addHeaderProxy(std::make_unique<Net::RawHeaderProxy>(headers));

    m_task.reset(new NetJob("YggdrasilAuthenticateStep", APPLICATION->network()));
    m_task->setAskRetry(false);
    m_task->addNetAction(m_request);
    connect(m_task.get(), &Task::finished, this, [this, response] { onRequestDone(response); });
    m_task->start();
}

void YggdrasilAuthenticateStep::onRequestDone(QByteArray* response)
{
    // NOTE: never log the response body; it contains the access token.
    if (m_request->error() != QNetworkReply::NoError) {
        qWarning() << "Yggdrasil request failed. HTTP" << m_request->replyStatusCode() << m_request->errorString();
        if (Net::isApplicationError(m_request->error()) && !Net::isServerError(m_request->error())) {
            emit finished(m_refresh ? AccountTaskState::STATE_FAILED_HARD : AccountTaskState::STATE_FAILED_SOFT,
                          tr("Authentication failed: %1").arg(m_request->errorString()));
        } else {
            m_data->networkError = m_request->error();
            emit finished(AccountTaskState::STATE_OFFLINE, tr("Could not reach the authentication server: %1").arg(m_request->errorString()));
        }
        return;
    }

    QJsonParseError err;
    auto doc = QJsonDocument::fromJson(*response, &err);
    if (err.error != QJsonParseError::NoError || !doc.isObject()) {
        emit finished(AccountTaskState::STATE_FAILED_SOFT, tr("The authentication server sent an invalid response."));
        return;
    }
    auto obj = doc.object();
    auto accessToken = obj.value("accessToken").toString();
    auto profile = obj.value("selectedProfile").toObject();
    if (accessToken.isEmpty()) {
        emit finished(AccountTaskState::STATE_FAILED_SOFT, tr("The authentication server returned no access token."));
        return;
    }

    m_data->yggdrasilToken.token = accessToken;
    m_data->yggdrasilToken.issueInstant = QDateTime::currentDateTimeUtc();
    m_data->yggdrasilToken.validity = Validity::Certain;
    if (obj.contains("clientToken"))
        m_data->yggdrasilClientToken = obj.value("clientToken").toString();

    if (!profile.isEmpty()) {
        m_data->minecraftProfile.id = profile.value("id").toString();
        m_data->minecraftProfile.name = profile.value("name").toString();
        m_data->minecraftProfile.validity = Validity::Certain;
    } else if (m_data->minecraftProfile.id.isEmpty()) {
        emit finished(AccountTaskState::STATE_FAILED_SOFT, tr("This account has no profile selected."));
        return;
    }
    m_data->minecraftEntitlement.ownsMinecraft = true;
    m_data->minecraftEntitlement.canPlayMinecraft = true;
    m_data->minecraftEntitlement.validity = Validity::Certain;

    emit finished(AccountTaskState::STATE_WORKING, tr("Authenticated"));
}
