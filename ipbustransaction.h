#ifndef IPBUSTRANSACTION_H
#define IPBUSTRANSACTION_H

#include<IPbusHeaders.h>
#include<RegisterMap.h>
#include"log.h"
#include"fee.h"

using IPbus_word = quint32;

enum TransactionSize{ write_response   = 1,
                      read_request     = 2,
                      NIwrite_response = 2,
                      NIread_request   = 2,
                      RMWbits_request  = 4,
                      RMWbits_response = 2,
                      RMWsum_request   = 3,
                      RMWsum_response  = 2};

const quint16 max_words_per_packet = 368;

class IPbusTransaction
{
public:
    IPbusTransaction(IPbus_word * const, Log* output = nullptr);
    IPbusTransaction(IPbus_word * const, quint16 size_of_data_to_fill = 0, Log* = nullptr);
    ~IPbusTransaction(){delete[] this->body;}

    void show_transaction();

    IPbusTransaction& getResponse(FEE &detector_regMap, quint16& itterator_request, quint16 itterator_response, bool& containErrors);
    static bool HeaderisValid(TransactionHeader header){
        return header.TypeID < 6              &&
               header.ProtocolVersion == 0x2  &&
               header.Words != 0              ;
    }

    IPbus_word* getTransactionInIPbusWords(){return this->body;}
    quint16     getTransactionSize(){return transaction_size;}


private:
    TransactionHeader header;
    quint16 transaction_size;
    IPbus_word * body;
    Log* output;


    bool initTransactionSize();
};


#endif // IPBUSTRANSACTION_H
