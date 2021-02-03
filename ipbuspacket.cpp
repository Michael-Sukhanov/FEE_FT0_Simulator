#include "ipbuspacket.h"



IPbusPacket::IPbusPacket(PacketHeader header, IPbus_word *data_without_header, quint16 packet_size, Log* output):LittleEndian(header.ByteOrder == 0xf),packet_size(packet_size),packet_header(header){
    if(output) this->output = output;
    this->body = new IPbus_word[this->packet_size];
    this->body[0] = (IPbus_word)this->packet_header;
    for(quint16 words_n_in_packet = 1; words_n_in_packet < packet_size; ++words_n_in_packet)
        this->body[words_n_in_packet] = data_without_header[words_n_in_packet -1];
    if(this->packet_header.ByteOrder != 0xf){
        this->change_endiannes();
        this->packet_header = this->body[0];
    }
}

IPbusPacket::IPbusPacket(IPbus_word header, IPbus_word *data_without_header, quint16 packet_size, Log* output):IPbusPacket(PacketHeader(header), data_without_header, packet_size, output){}


IPbusPacket::IPbusPacket(IPbus_word *data_with_header, quint16 packet_size, Log* output):IPbusPacket(PacketHeader(data_with_header[0]), &data_with_header[1], packet_size, output){}

IPbusPacket::~IPbusPacket(){

}

IPbusPacket &IPbusPacket::handle(FEE &targetMap, QHostAddress &Host_address, quint16 &port_no){
    if(output) this->output->sendMessage("<" + QTime::currentTime().toString("hh:mm:ss") + '>' +
                QString::asprintf(" %7s packet (%3u words %cE) from %s:%u", this->packet_header.PacketType == 0 ? "Control" :
                                                                  (this->packet_header.PacketType == 1 ? "Status" : "ReSend"),
                                                                   this->packet_size,
                                                                   this->LittleEndian ? 'L' : 'B', Host_address.toString().toLocal8Bit().data(), port_no), file);

    switch(this->packet_header.PacketType){
        case control: if(this->output)this->output->sendMessage("Control Packet Received");
                      return PrepareControlResponse(targetMap);
        case status:  if(this->output)this->output->sendMessage("Status Packet Received");
                      return PrepareStatusResponse ();
//        case resend:  return PrepareReSendResponse (); break;
    }
}


bool IPbusPacket::isIPbusPacket(const IPbus_word *pointer_on_header, quint16 packet_size){
        PacketHeader header(*pointer_on_header);
        header = header.ByteOrder == 0xf ? header : PacketHeader(qFromBigEndian(quint32(header)));
        return header.Rsvd == 0              &&
               header.ProtocolVersion == 0x2 &&
               header.PacketType < 3         &&
               header.ByteOrder == 0xf       &&
               header.PacketID == 1 ? (!(*std::max_element(&pointer_on_header[1], &pointer_on_header[15]) > 0x0) && packet_size == 16) : true        &&
               header.PacketID == 2 ? packet_size = 1 : true;
}

void IPbusPacket::change_endiannes(){
    for(quint16 i = 0; i < packet_size; ++i)
        this->body[i] = qFromBigEndian(this->body[i]);
}

void IPbusPacket::show(){
        for(quint16 i = 0; i < packet_size; ++i)
qDebug() << QString::asprintf("0x%08X", this->LittleEndian ?  this->body[i] : qFromBigEndian(this->body[i]));
}

IPbus_word *IPbusPacket::getPacketInIPbusWords(){
    if(!LittleEndian)
        this->change_endiannes();
    return this->body;
}

quint16 IPbusPacket::getPacketSize(){return this->packet_size;}


IPbusPacket& IPbusPacket::PrepareControlResponse(FEE &targetmap){
        quint8 transaction_counter = 0;
        quint16 words_n_in_response = 1, words_n_in_request = 1;
        IPbus_word handled_body_without_header[max_words_per_packet];
        bool containErrors;
        handled_body_without_header[0] = this->packet_header;
        while(words_n_in_request < this->packet_size){
            if(!IPbusTransaction::HeaderisValid(this->body[words_n_in_request])){
                ++words_n_in_request;
                if(this->output)this->output->sendMessage("Invalid transaction header: dissmissed");
                continue;
            }
            IPbusTransaction RequestTransaction(&this->body[words_n_in_request], this->output);
            ++transaction_counter;
            IPbusTransaction &ResponseTransaction = RequestTransaction.getResponse(targetmap, words_n_in_request, words_n_in_response, containErrors);
                for(quint16 k = 0; k < ResponseTransaction.getTransactionSize(); ++k){
                    handled_body_without_header[words_n_in_response] = ResponseTransaction.getTransactionInIPbusWords()[k];
                    ++words_n_in_response;
            }
                //ResponseTransaction.show_transaction();
                if(containErrors) break;//interruping packet handle
        }
        IPbusPacket *response = new IPbusPacket(handled_body_without_header, words_n_in_response, this->output);
        if(!LittleEndian)
            response->change_endiannes();
        if(this->output) this->output->sendMessage(QString::asprintf("Packet with %d transaction%c", transaction_counter, transaction_counter > 1 ? 's' : ' '));
        return *response;
}

IPbusPacket &IPbusPacket::PrepareStatusResponse(){
    IPbus_word response_array_in_IPbus_words[16] =
    {0x200000f1,
     0x000007F8,
     0x00000001,
     0x200002F0,
     0x0F0F0F0F,
     0x0F0F0F0F,
     0x0F0F0F0F,
     0x0F0F0F0F,
     0xF0000020,
     0xF0000020,
     0xF0000020,
     0xF0000020,
     0xF0000020,
     0xF0000020,
     0xF0000020,
     0xF0000020};
    IPbusPacket *response = new IPbusPacket(response_array_in_IPbus_words, 16);
    response->change_endiannes();
    return *response;
}







