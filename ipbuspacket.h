#ifndef IPBUSPACKET_H
#define IPBUSPACKET_H

#include <ipbustransaction.h>

const quint8 Word_size = sizeof (quint32);


class IPbusPacket
{
public:
    IPbusPacket(PacketHeader header, IPbus_word *data_without_header, quint16 packet_size, Log* = nullptr);
    IPbusPacket(IPbus_word header, IPbus_word *data_without_header, quint16 packet_size, Log* = nullptr);
    IPbusPacket(IPbus_word *data_with_header, quint16 packet_size, Log* = nullptr);
    ~IPbusPacket();

    IPbusPacket& handle(FEE &targetmap, QHostAddress &Host_address, quint16 &port_no);
    static bool isIPbusPacket(const IPbus_word *pointer_on_header, quint16 packet_size);
    void show();

    IPbus_word* getPacketInIPbusWords();
    quint16 getPacketSize();


private:
    const bool LittleEndian;
    const quint16 packet_size; // in 4 bytes words;
    PacketHeader packet_header;
    IPbus_word *body;
    Log *output;


    void change_endiannes();

    IPbusPacket& PrepareControlResponse(FEE &targetmap);
    IPbusPacket& PrepareStatusResponse ();
    IPbusPacket& PrepareReSendResponse ();


};

#endif // IPBUSPACKET_H
