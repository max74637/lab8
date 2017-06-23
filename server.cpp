/** @file
    @brief Server class implementation
*/

#include <stdio.h>
#include <QUrl>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QJsonObject>
#include <QJsonArray>

#include <progbase/net.h>

#include "server.h"

#define PORT 3000

#define BUFFER_LEN 1024

#define SERVER_TIME_FORMAT   "hh:mm:ss AP"

#define RESPONSE_OK          "HTTP/1.0 200 OK\r\n"                \
                             "Content-Type: application/json\r\n" \
                             "\r\n"

#define RESPONSE_BAD_REQUEST "HTTP/1.0 400 Bad Request\r\n"       \
                             "\r\n"

#define RESPONSE_NOT_FOUND   "HTTP/1.0 404 Not Found\r\n"         \
                             "\r\n"

#define SERVER_NAME          "lab8"

#define DEVELOPER_NAME       "Maxim Sysoev"

#define SERVER_INFO          "{ "                                 \
                                 "\"title\": \"%s\", "            \
                                 "\"developer\": \"%s\", "        \
                                 "\"time\": \"%s\" "              \
                             "}"

#define FAVORITES_FILE       "../favorites.json"

#define DATA_FILE            "../data/data.txt"

Server::Server()
{
}

Server::~Server()
{
}

void Server::Start()
{
    char buffer[BUFFER_LEN];

    TcpListener tcpListener;
    TcpListener_init(&tcpListener);

    IpAddress ipAddress;
    IpAddress_initAny(&ipAddress, PORT);

    if(!TcpListener_bind(&tcpListener, &ipAddress)) {
        printf("ERROR: tcp bind\n");
        return;
    }

    if (!TcpListener_start(&tcpListener)) {
        printf("ERROR: tcp server start\n");
        return;
    }

    printf("TCP Server is listening on port %d\n",
        IpAddress_port(TcpListener_address(&tcpListener)));

    NetMessage netMessage;
    NetMessage_init(&netMessage, buffer,  BUFFER_LEN);

    TcpClient tcpClient;
    tcpClient.ssl = NULL;
    while (1) {
        TcpListener_accept(&tcpListener, &tcpClient);
        QTime currentTime = QTime::currentTime();
        QByteArray request;
        while (1)
        {
            if(!TcpClient_receive(&tcpClient, &netMessage)) {
                printf("ERROR: receive\n");
                return;
            }
            request.append(netMessage.buffer, netMessage.dataLength);
            if ((size_t)netMessage.dataLength < netMessage.bufferLength)
                break;
        }
        IpAddress* clientIpAddress = TcpClient_address(&tcpClient);
        printf("%s request from %s:%d (%d bytes):\n%s\n",
            currentTime.toString(SERVER_TIME_FORMAT).toLocal8Bit().data(),
            IpAddress_address(clientIpAddress),
            IpAddress_port(clientIpAddress),
            request.size(),
            request.data());

        QByteArray response = getResponse(request, currentTime);
        printf("Response:\n%s\n\n", response.data());

        NetMessage responseMessage;
        responseMessage.buffer = response.data();
        responseMessage.bufferLength = response.size();
        responseMessage.dataLength = response.size();
        responseMessage.sentDataLength = 0;
        if(!TcpClient_send(&tcpClient, &responseMessage)) {
            printf("ERROR: send\n");
            break;
        }
        TcpClient_close(&tcpClient);
    }

    TcpListener_close(&tcpListener);
}

QByteArray Server::getResponse(QByteArray request, QTime currentTime)
{
    QByteArray response;

    QString header = QString(request).split("\r\n").at(0);
    QString method = header.split(" ").at(0);
    QString path   = header.split(" ").at(1);

    if (method != "GET") {
        response.append(RESPONSE_BAD_REQUEST);
        return response;
    }

    if (path == "/") {
        QString s;
        s.sprintf(SERVER_INFO,
                  SERVER_NAME,
                  DEVELOPER_NAME,
                  currentTime.toString(SERVER_TIME_FORMAT).toLocal8Bit().data());
        response.append(RESPONSE_OK);
        response.append(s.toLocal8Bit());
        return response;
    }

    if (path == "/favorites") {
        QJsonDocument doc = getFavorites();
        if (doc.isNull()) {
            response.append(RESPONSE_NOT_FOUND);
            return response;
        }
        response.append(RESPONSE_OK);
        response.append(doc.toJson());
        return response;
    }

    if (path.startsWith("/favorites?")) {
        QStringList keys = QString("id author name album year").split(" ");
        QStringList pair = path.right(path.size() - strlen("/favorites?")).split("=");
        if (pair.size() != 2 || !keys.contains(pair[0])) {
            response.append(RESPONSE_BAD_REQUEST);
            return response;
        }
        QJsonDocument doc = getFavorites(pair[0], pair[1]);
        if (doc.isNull()) {
            response.append(RESPONSE_NOT_FOUND);
            return response;
        }
        response.append(RESPONSE_OK);
        response.append(doc.toJson());
        return response;
    }

    if (path.startsWith("/favorites/")) {
        bool ok;
        int id = path.right(path.size() - strlen("/favorites/")).toInt(&ok);
        if (!ok) {
            response.append(RESPONSE_BAD_REQUEST);
            return response;
        }
        QJsonDocument doc = getFavorite(id);
        if (doc.isNull()) {
            response.append(RESPONSE_NOT_FOUND);
            return response;
        }
        response.append(RESPONSE_OK);
        response.append(doc.toJson());
        return response;
    }

    if (path == "/file") {
        QJsonDocument doc = getFile();
        if (doc.isNull()) {
            response.append(RESPONSE_NOT_FOUND);
            return response;
        }
        response.append(RESPONSE_OK);
        response.append(doc.toJson());
        return response;
    }

    if (path == "/file/data") {
        QJsonDocument doc = getFileData();
        if (doc.isNull()) {
            response.append(RESPONSE_NOT_FOUND);
            return response;
        }
        response.append(RESPONSE_OK);
        response.append(doc.toJson());
        return response;
    }

    response.append(RESPONSE_NOT_FOUND);
    return response;
}

QJsonDocument Server::getFavorites() {
    QFile file(FAVORITES_FILE);
    if (!file.open(QFile::ReadOnly))
        return QJsonDocument::fromJson("");
    else
        return QJsonDocument::fromJson(file.readAll());
}

QJsonDocument Server::getFavorites(QString key, QString value)
{
    QJsonDocument doc = getFavorites();
    if (doc.isNull())
        return doc;

    value = QUrl::fromPercentEncoding(value.toLocal8Bit());

    bool valueIsInt = false;
    int intValue;
    if (key == "id" || key == "year") {
        intValue = value.toInt(&valueIsInt);
        if (!valueIsInt)
           return QJsonDocument::fromJson("");
    }

    QJsonArray selectedFavorites;
    QJsonArray favorites = doc.object()["favorites"].toArray();
    for (int n = 0; n < favorites.count(); n++) {
        QJsonObject favorite = favorites[n].toObject();
        if (valueIsInt && favorite[key].toInt() == intValue)
            selectedFavorites.append(favorite);
        else if (!valueIsInt && favorite[key].toString() == value)
            selectedFavorites.append(favorite);
    }

    QJsonObject obj;
    obj.insert("favorites", selectedFavorites);
    QJsonDocument result;
    result.setObject(obj);
    return result;
}

QJsonDocument Server::getFavorite(int id)
{
    QJsonDocument doc = getFavorites();
    if (doc.isNull())
        return doc;

    QJsonArray favorites = doc.object()["favorites"].toArray();
    for (int n = 0; n < favorites.count(); n++) {
        QJsonObject favorite = favorites[n].toObject();
        if (favorite["id"].toInt() == id) {
            QJsonDocument result;
            result.setObject(favorite);
            return result;
        }
    }

    return QJsonDocument::fromJson("");
}

QJsonDocument Server::getFile()
{
    QFile file(DATA_FILE);
    if (!file.open(QFile::ReadOnly))
        return QJsonDocument::fromJson("");
    QJsonDocument doc;
    QJsonObject obj;
    obj.insert("file_name",    QFileInfo(file).fileName());
    obj.insert("file_size",    file.size());
    obj.insert("file_content", QString(file.readAll()));
    doc.setObject(obj);
    return doc;
}

QJsonDocument Server::getFileData()
{
    QFile file(DATA_FILE);
    if (!file.open(QFile::ReadOnly))
        return QJsonDocument::fromJson("");
    bool first_line = true;
    int number_count = 0;
    int minimal_number = 0;
    QTextStream textStream(&file);
    while (!textStream.atEnd()) {
        QString line = textStream.readLine();
        bool ok;
        int n = line.toInt(&ok);
        if (!ok)
            continue;
        number_count++;
        if (first_line) {
            minimal_number = n;
            first_line = false;
        }
        if (n < minimal_number)
            minimal_number = n;
    }

    QJsonDocument doc;
    QJsonObject obj;
    obj.insert("file_name",      QFileInfo(file).fileName());
    obj.insert("number_count",   number_count);
    obj.insert("minimal_number", minimal_number);
    doc.setObject(obj);
    return doc;
}
