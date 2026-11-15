#pragma once

#include <QString>
#include <memory>

#include "LaunchMode.h"

class MinecraftAccount;

struct AuthSession {
    bool MakeOffline(QString offline_playername);
    void MakeDemo(QString name, QString uuid);

    QString serializeUserProperties();

    // combined session ID
    QString session;
    // volatile auth token
    QString access_token;
    // profile name
    QString player_name;
    // profile ID
    QString uuid;
    // 'msa' or 'offline', depending on account type
    QString user_type;
    // non-empty for Yggdrasil accounts: server URL passed to authlib-injector
    QString authlibInjectorApiRoot;
    // the actual launch mode for this session
    LaunchMode launchMode;
};

using AuthSessionPtr = std::shared_ptr<AuthSession>;
