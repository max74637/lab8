/** @file
    @brief Server class header
*/

#ifndef SERVER_H
#define SERVER_H

#include <QByteArray>
#include <QTime>
#include <QJsonDocument>
#include <QString>

/**
    @class Server
    @brief implements lab8 tasks
*/
class Server
{
public:
    /**
        @brief Server constructor
    */
    Server();
    /**
        @brief Server destructor
    */
    ~Server();

    /**
        @brief Start Server listener
    */
    void Start();

protected:
    /**
        @brief Get Server response
        @param request - HTTP request from client
        @param currentTime - current server time
    */
    QByteArray getResponse(QByteArray request, QTime currentTime);
    /**
        @brief Get all favorites
    */
    QJsonDocument getFavorites();
    /**
        @brief Get favorites by params
        @param key - favorite key
        @param value - favorite value of key
    */
    QJsonDocument getFavorites(QString key, QString value);
    /**
        @brief Get favorite by id
        @param id - favorite id
    */
    QJsonDocument getFavorite(int id);
    /**
        @brief Get file information
    */
    QJsonDocument getFile();
    /**
        @brief Get file number count and minimal bumber
    */
    QJsonDocument getFileData();
};

#endif // SERVER_H
