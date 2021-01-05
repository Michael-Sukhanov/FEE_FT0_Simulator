#include "server.h"

Server::Server(FEE* targetmap, Log* output){
    this->targetmap = targetmap;
    if(output) this->output = output;
    socket = new QUdpSocket(this);
    connect(socket, &QUdpSocket::readyRead, this, &Server::Service);
}



bool Server::start(){
    started = socket->bind(QHostAddress::AnyIPv4, port_to_listen);
    if(this->output)this->output->sendMessage(started ? "IPbus target server started" : "Binding failed: check port 50001");
    return started;
}
void Server::stop(){
    if(this->output)this->output->sendMessage("IPbus target server stoped");
    socket->close();}

void Server::Service(){
    if(socket->hasPendingDatagrams()){
        getDatagram();
        if(IPbusPacket::isIPbusPacket(request_space, request_packet_size / Word_size)){
            IPbusPacket Request(request_space, request_packet_size / Word_size, this->output);
            IPbusPacket &Response = Request.handle(*targetmap, senderAdress, senderPort);
            emit response_ready();
            response_packet_size = Response.getPacketSize() * Word_size;
            for(quint16 i = 0; i < Response.getPacketSize(); ++i)
                response_space[i] = Response.getPacketInIPbusWords()[i];
            if(this->output) this->output->sendMessage("Response ready");
            sendDatagram();
        }else
            if(this->output) this->output->sendMessage(QString::asprintf("Packet is not IPbus type from %s:%u", senderAdress.toString().toLocal8Bit().data(), senderPort));
    }
}

void Server::getDatagram(){
    request_packet_size = socket->pendingDatagramSize();
    socket->readDatagram(request_char_pointer, max_words_per_packet * Word_size, &senderAdress,&senderPort);
    emit packet_recieved(senderAdress, senderPort);}

void Server::sendDatagram(){socket->writeDatagram(response_char_pointer, response_packet_size, senderAdress, senderPort);}
