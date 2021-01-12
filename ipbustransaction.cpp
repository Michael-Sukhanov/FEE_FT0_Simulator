#include "ipbustransaction.h"


//Попробовать указатель на ссылку или ссылку на указатель?
IPbusTransaction::IPbusTransaction(IPbus_word * const data, Log* output):header(TransactionHeader(data[0])){
    if(output) this->output = output;
    if(initTransactionSize()){
        this->body = new IPbus_word[transaction_size];
        for(quint16 i = 0; i < transaction_size; ++i)
            this->body[i] = data[i];
    }else{
        header.InfoCode = 0x1;
        this->body = new IPbus_word;
        this->body[0] = header;
    }
}

IPbusTransaction::IPbusTransaction( IPbus_word * const body, quint16 size_of_data, Log* output):header(TransactionHeader(body[0])){ //for cutted transactions;
    if(output) this->output = output;
    this->transaction_size = size_of_data;
    this->body = new IPbus_word[transaction_size];
    for(quint16 i = 0; i < size_of_data; ++i)
        this->body[i] = body[i];
}

void IPbusTransaction::show_transaction(){
    for(quint16 i = 0; i < transaction_size; ++i){
        qDebug()<< QString::asprintf("0x%08X",this->body[i]);
    }
}

//contain Errors if header infoCode > 0 in response;
IPbusTransaction &IPbusTransaction::getResponse(FEE &detectorRegMap, quint16& itterator_request, quint16 itterator_response, bool& containErrors){
    static quint32 dataToSend[256];
    quint16 amount_of_words = 1;
    quint32 baseTransactionAddress = this->body[1];
    if(header.InfoCode == 0xf){
        switch (header.TypeID) {
        case nonIncrementingRead:
        case read: {for(quint16 i = 0; i < header.Words; ++i){
                    if(max_words_per_packet - itterator_response - 1 != 0){
                        header.InfoCode = detectorRegMap.readWord(baseTransactionAddress + (header.TypeID == read ? i : 0), dataToSend[i + 1], this->output);
                        if(header.InfoCode != 0x0){
                            header.Words = amount_of_words;
                            break;}
                        amount_of_words ++;
                    }else{
                        header.InfoCode = 0x4;
                        header.Words = amount_of_words - 1;
                        break;}
                    }
                    if(this->output && !header.InfoCode)this->output->sendMessage(QString::asprintf("reading %3u words from 0x%08X (%s)", header.Words, baseTransactionAddress, header.TypeID == read ? "sequental" : "non-incrementing"), file);
                    break;
        }
        case nonIncrementingWrite:
        case write: {
            amount_of_words = 1;
            for(quint16 i = 0; i < header.Words; ++i){
                header.InfoCode = detectorRegMap.writeWord(baseTransactionAddress + (header.TypeID == write ? i : 0), this->body[2 + i], this->output, TransactionType(header.TypeID));
                if(header.InfoCode != 0){
                    header.Words = i;
                    break;}
            }
            break;
        }
        case RMWbits:
        case RMWsum:{
            amount_of_words = 2;
            detectorRegMap.readWord(baseTransactionAddress, dataToSend[1]);
            header.InfoCode = detectorRegMap.writeWord(baseTransactionAddress, header.TypeID == RMWbits ? (dataToSend[1] & this->body[2]) | this->body[3]: dataToSend[1] + this->body[2], this->output, TransactionType(header.TypeID));
            break;
        }

        }
        itterator_request += this->transaction_size;
    }
    else{
        header.InfoCode = 1;
        amount_of_words = 1;
    }
    containErrors = header.InfoCode > 0;
    dataToSend[0] = header;
    return *new IPbusTransaction(dataToSend, amount_of_words);
}


bool IPbusTransaction::initTransactionSize(){
    if(this->header.InfoCode == 0xf)
        switch (this->header.TypeID) {
            case write                :
            case nonIncrementingWrite :  transaction_size = header.Words + 2;                   break;
            case read                 :
            case nonIncrementingRead  :  transaction_size = read_request    ;                   break;
            case RMWbits              :  transaction_size = RMWbits_request + header.Words - 1; break;
            case RMWsum               :  transaction_size = RMWsum_request  + header.Words - 1; break;
            default                   :                                                         break;
        }
    else if(this->header.InfoCode == 0x0)
        switch (this->header.TypeID) {
            case write                :
            case nonIncrementingWrite :  transaction_size = write_response  ;                   break;
            case read                 :
            case nonIncrementingRead  :  transaction_size = header.Words + 1;                   break;
            case RMWbits              :  transaction_size = RMWbits_response;                   break;
            case RMWsum               :  transaction_size = RMWsum_response ;                   break;
            default                   :                                                         break;
        }
    else       transaction_size = 1;
    return HeaderisValid(this->header) ;
}





