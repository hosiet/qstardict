/*****************************************************************************
 * anki.cpp - QStarDict, a StarDict clone written with using Qt              *
 * Copyright (C) 2018 Alexander Rodin                                        *
 *                                                                           *
 * This program is free software; you can redistribute it and/or modify      *
 * it under the terms of the GNU General Public License as published by      *
 * the Free Software Foundation; either version 2 of the License, or         *
 * (at your option) any later version.                                       *
 *                                                                           *
 * This program is distributed in the hope that it will be useful,           *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
 * GNU General Public License for more details.                              *
 *                                                                           *
 * You should have received a copy of the GNU General Public License along   *
 * with this program; if not, write to the Free Software Foundation, Inc.,   *
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.               *
 *****************************************************************************/

#include "anki.h"
#include "settingsdialog.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QSettings>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonValue>
#include <QJsonObject>
#include <QMessageBox>

#if QT_VERSION < QT_VERSION_CHECK(5,0,0)
#include "anki-meta.h"
#endif

#if QT_VERSION < QT_VERSION_CHECK(5,0,0)
QStarDict::PluginMetadata Anki::metadata() const
{
    QStarDict::PluginMetadata md;
    md.id = PLUGIN_ID;
    md.name = QString::fromUtf8(PLUGIN_NAME);
    md.version = PLUGIN_VERSION;
    md.description = PLUGIN_DESCRIPTION;
    md.authors = QString::fromUtf8(PLUGIN_AUTHORS).split(';', QString::SkipEmptyParts);
    md.features = QString::fromLatin1(PLUGIN_FEATURES).split(';', QString::SkipEmptyParts);
    md.icon = QIcon(":/icons/anki.png");
    return md;
}
#else
QIcon Anki::pluginIcon() const
{
    return QIcon(":/icons/anki.png");
}
#endif

Anki::Anki(QObject *parent)
    : QObject(parent)
{
    m_networkAccessManager = new QNetworkAccessManager(this);
    connect(m_networkAccessManager, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(onNetworkRequestFinished(QNetworkReply*)));

    QSettings settings("qstardict", "qstardict");
    m_connectUrl = settings.value("Anki/connectUrl", "http://127.0.0.1:8765").toString();
    m_deckName = settings.value("Anki/deckName", "Default").toString();
    m_modelName = settings.value("Anki/modelName", "Basic").toString();
    m_allowDuplicates = settings.value("Anki/allowDuplicates", false).toBool();
}

Anki::~Anki()
{
    QSettings settings("qstardict", "qstardict");
    settings.setValue("Anki/connectUrl", m_connectUrl);
    settings.setValue("Anki/deckName", m_deckName);
    settings.setValue("Anki/modelName", m_modelName);
    settings.setValue("Anki/allowDuplicates", m_allowDuplicates);
}

QIcon Anki::toolbarIcon() const {
    return QIcon(":/icons/anki.png");
}

QString Anki::toolbarText() const {
    return tr("Add word to Anki");
}

void Anki::execute(const QString &word, const QString &translation) {
    QJsonObject requestObject;
    requestObject.insert("action", QString("addNote"));
    requestObject.insert("version", 6);
    QJsonObject paramsObject;
    QJsonObject noteObject;
    noteObject.insert("deckName", m_deckName);
    noteObject.insert("modelName", m_modelName);
    QJsonObject fieldsObject;
    fieldsObject.insert("Front", word);
    fieldsObject.insert("Back", translation);
    noteObject.insert("fields", fieldsObject);
    QJsonObject optionsObject;
    optionsObject.insert("allowDuplicate", m_allowDuplicates);
    noteObject.insert("options", optionsObject);
    QJsonArray tagsArray;
    noteObject.insert("tags", tagsArray);
    paramsObject.insert("note", noteObject);
    requestObject.insert("params", paramsObject);

    QJsonDocument requestDocument;
    requestDocument.setObject(requestObject);

    QNetworkRequest request(m_connectUrl);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    m_networkAccessManager->post(request, requestDocument.toJson());
}

void Anki::onNetworkRequestFinished(QNetworkReply *reply) {
    if (reply->error() != QNetworkReply::NoError) {
        QMessageBox::critical(nullptr, tr("Anki error"),
                tr("Unable to add the word to Anki: network error. " \
                   "Check if Anki is running and " \
                   "<a href=\"https://ankiweb.net/shared/info/2055492159\">AnkiConnect</a> " \
                   "add-on is installed to Anki."));
    }
}

int Anki::execSettingsDialog(QWidget *parent)
{
    ::SettingsDialog dialog(this, parent);
    return dialog.exec();
}

#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(anki, Anki)
#endif

// vim: tabstop=4 softtabstop=4 shiftwidth=4 expandtab cindent textwidth=120 formatoptions=tc
