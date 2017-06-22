/** @file
    @brief Some text about server.h
*/

#ifndef SERVER_H
#define SERVER_H

#include <QByteArray>
#include <QTime>
#include <QJsonDocument>
#include <QString>

/**
    @class Server
    @brief about Server
*/
class Server
{
public:
    Server();
    ~Server();

    /**
        @brief Start
    */
    void Start();

protected:
    /**
        @brief getResponse
        @param request - about request
        @param currentTime - about getResponse
    */
    QByteArray getResponse(QByteArray request, QTime currentTime);
    QJsonDocument getFavorites();
    QJsonDocument getFavorites(QString key, QString value);
    QJsonDocument getFavorite(int id);
    QJsonDocument getFile();
    QJsonDocument getFileData();
};

#endif // SERVER_H
