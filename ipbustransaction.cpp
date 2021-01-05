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

IPbusTransaction &IPbusTransaction::getResponse(FEE &detectorRegMap, quint16& itterator_request, quint16 itterator_response){
    quint32 *data_to_send;
    quint16 baseTransactionAddress = this->body[1];
    quint16 amount_of_words;
    if(header.InfoCode == 0xf){
        switch (header.TypeID) {
            case nonIncrementingRead:
            case read:  {header.InfoCode = (header.Words + 1 + itterator_response > max_words_per_packet) ? 4 : 0;
                         amount_of_words = (header.InfoCode ? max_words_per_packet - itterator_response - 1 : header.Words) + 1;
                         data_to_send = new quint32[amount_of_words];
                         for(quint16 i = 0; i < amount_of_words - 1; ++i){
                             if(detectorRegMap.isFIFO(baseTransactionAddress + i) && header.TypeID != nonIncrementingRead)
                                 if(this->output)this->output->sendMessage("Warning FIFO reading!!");
                          data_to_send[i + 1] = detectorRegMap.read_word(baseTransactionAddress + (header.TypeID == read ? i : 0));
                         }
                         if(this->output)this->output->sendMessage(QString::asprintf("reading %3u words from 0x%08X (%s)",
                                                                                     amount_of_words - 1, baseTransactionAddress,
                                                                                     header.TypeID == read ? "sequental" : "non-incrementing"), file);
                         break;}
            case write:  {/*if(this->output)this->output->sendMessage("Write transaction detected");*/
                          data_to_send = new IPbus_word;
                          amount_of_words = 1;
                          header.InfoCode = 0x0;
                          for(quint16 i = 0; i < header.Words; ++i){
                              if(detectorRegMap.isFIFO(baseTransactionAddress + i)){
                                  if(this->output) this->output->sendMessage(QString::asprintf("writing 0x%08X to FIFO 0x%08X: forbidden",
                                                                                               this->body[2 + i], baseTransactionAddress + i), file);
                                  header.InfoCode = 0x3;
                                  break;
                              }
                              detectorRegMap.write_word(this->body[2+i], baseTransactionAddress + i);
                              if(this->output) this->output->sendMessage(QString::asprintf("writing 0x%08X to 0x%08X",this->body[2 + i], baseTransactionAddress + i), file);
                          }
                          break;}
            case RMWsum:
            case RMWbits: {/*if(this->output)this->output->sendMessage("RMWbits transaction detected");*/
                          amount_of_words = RMWbits_response + itterator_response > max_words_per_packet ? 1 : 2;
                          header.InfoCode = header.Words != 1 ? 0x1 : 0;
                          IPbus_word result_bits = this->header.TypeID == RMWbits ? (detectorRegMap.getValue(baseTransactionAddress) & this->body[2]) | this->body[3]:
                                                                                     detectorRegMap.getValue(baseTransactionAddress) + this->body[2];
                          data_to_send = new quint32[RMWbits_response];
                          data_to_send[1] = detectorRegMap.getValue(baseTransactionAddress);
                          if(detectorRegMap.isFIFO(baseTransactionAddress)){
                              if(this->output) this->output->sendMessage(QString::asprintf("writing 0x%08X to FIFO 0x%08X: forbidden",
                                                                                           result_bits, baseTransactionAddress), file);
                              header.InfoCode = 0x3;
                              amount_of_words = 1;
                          }else{
                              detectorRegMap.write_word(result_bits, baseTransactionAddress);
                              if(this->output) this->output->sendMessage(QString::asprintf("writing 0x%08X to 0x%08X (RMWoperation)",
                                                                                           result_bits, baseTransactionAddress), file);
                          }
                          break;}
            case nonIncrementingWrite:{if(this->output)this->output->sendMessage("non Incrementing Write transaction detected");
                                        data_to_send = new IPbus_word;
                                        amount_of_words = 1;
                                        header.InfoCode = 0x0;
                                        for(quint16 i = 0; i < header.Words; ++i){
                                            detectorRegMap.write_word(this->body[2+i], baseTransactionAddress);
                                            if(this->output) this->output->sendMessage(QString::asprintf("writing 0x%08X to 0x%08X (non-incrementing)",this->body[2 + i], baseTransactionAddress), file);
                                        }
                                        break;}

            default: data_to_send = new IPbus_word;
                     header.InfoCode = 1;
                     amount_of_words = 1;
                     break;
        }
        itterator_request += this->transaction_size;
    }
    else{
        data_to_send = new IPbus_word;
        header.InfoCode = 1;
        amount_of_words = 1;
    }
    data_to_send[0] = header;
    return *new IPbusTransaction(data_to_send,amount_of_words);
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



