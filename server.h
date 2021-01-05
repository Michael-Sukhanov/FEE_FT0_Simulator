#ifndef SERVER_H
#define SERVER_H

#include <QObject>
#include <QUdpSocket>
#include "ipbuspacket.h"

class Server: public QObject
{
    Q_OBJECT
public:
    Server(FEE *targetmap, Log* output = nullptr);
    bool start();
    void stop();
    bool isStarted(){return started;};

signals:
    void packet_recieved(QHostAddress , quint16);
    void response_ready();

private:
    bool started = false;
    const quint16 port_to_listen = 50001;
    QUdpSocket* socket;
    quint16 senderPort, response_packet_size, request_packet_size;
    QHostAddress senderAdress;
    IPbus_word request_space[max_words_per_packet], response_space[max_words_per_packet];
    char* request_char_pointer = reinterpret_cast<char*>(request_space), *response_char_pointer = reinterpret_cast<char*>(response_space);

    Log* output;
    IPbusPacket* buffer;
    FEE* targetmap;



private slots:
    void Service();
    void getDatagram();
    void sendDatagram();

};

#endif // SERVER_H
