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
                    if(detectorRegMap.readWord(baseTransactionAddress + (header.TypeID == read ? i : 0), dataToSend[1 + i], this->output) && max_words_per_packet - itterator_response - 1 != 0){
                        amount_of_words ++;
                        header.InfoCode = 0;
                    }else{
                        header.InfoCode = 0x4;
                        header.Words = amount_of_words - 1;
                        break;}
                    }
                    if(this->output)this->output->sendMessage(QString::asprintf("reading %3u words from 0x%08X (%s)", header.Words, baseTransactionAddress, header.TypeID == read ? "sequental" : "non-incrementing"), file);
                    break;
        }
        case nonIncrementingWrite:
        case write: {
            amount_of_words = 1;
            for(quint16 i = 0; i < header.Words; ++i){
                if(!detectorRegMap.writeWord(this->body[2 + i], baseTransactionAddress + (header.TypeID == write ? i : 0), this->output, TransactionType(header.TypeID))){
                    header.InfoCode = 0x5;
                    header.Words = i;
                    break;
                }else header.InfoCode = 0;
            }
            break;
        }
        case RMWbits:
        case RMWsum:{
            amount_of_words = 2;
            if(!detectorRegMap.getValue(baseTransactionAddress, dataToSend[1],this->output, TransactionType(header.TypeID)) ||
                    !detectorRegMap.writeWord((header.TypeID == RMWbits) ? ((dataToSend[1] & this->body[2]) | this->body[3]) :
                                                               (dataToSend[1] + this->body[2]), baseTransactionAddress, this->output, TransactionType(header.TypeID))){
                amount_of_words = 1;
                header.InfoCode = 0x5;
                header.Words = 0;
                break;
            }else header.InfoCode = 0;
            break;
        }
//            case nonIncrementingRead:
//            case read:  {bool interupped = false;
//                         header.InfoCode = (header.Words + 1 + itterator_response > max_words_per_packet) ? 4 : 0;
//                         amount_of_words = (header.InfoCode ? max_words_per_packet - itterator_response - 1 : header.Words) + 1;
//                         //shift of prohibited address relatively to baseAddress
//                         prohibitedAddress = whereIsProhibited(detectorRegMap, baseTransactionAddress, header.Words);
//                         if((header.TypeID == read && prohibitedAddress != header.Words) ||
//                            (header.TypeID == nonIncrementingRead && prohibitedAddress == 0)){
//                             amount_of_words = 1 + prohibitedAddress;
//                             header.InfoCode = 4;
//                             header.Words = prohibitedAddress;
//                             interupped = true;
//                         }
//                         data_to_send = new quint32[amount_of_words];
//                         for(quint16 i = 0; i < amount_of_words - 1; ++i){
//                             if(detectorRegMap.isFIFO(baseTransactionAddress + i) && header.TypeID != nonIncrementingRead)
//                                 if(this->output)this->output->sendMessage("Warning FIFO reading!!");
//                          data_to_send[i + 1] = detectorRegMap.read_word(baseTransactionAddress + (header.TypeID == read ? i : 0));
//                         }
//                         if(this->output)this->output->sendMessage(QString::asprintf("reading %3u words from 0x%08X (%s)%s",
//                                                                                     amount_of_words - 1, baseTransactionAddress,
//                                                                                     header.TypeID == read ? "sequental" : "non-incrementing", interupped ? " ->were interupped" : ""), file);
//                         break;}
//            case write:  {/*if(this->output)this->output->sendMessage("Write transaction detected");*/
//                          data_to_send = new IPbus_word;
//                          amount_of_words = 1;
//                          header.InfoCode = 0x0;
//                          for(quint16 i = 0; i < header.Words; ++i){
//                              if(detectorRegMap.isFIFO(baseTransactionAddress + i) || whereIsProhibited(detectorRegMap, baseTransactionAddress + i, 1) == 0){
//                                  if(this->output) this->output->sendMessage(QString::asprintf("writing 0x%08X to %s 0x%08X: forbidden",
//                                                                                               this->body[2 + i], detectorRegMap.isFIFO(baseTransactionAddress + i) ? "FIFO" : "non-existant" , baseTransactionAddress + i), file);
//                                  header.InfoCode = 0x5;
//                                  header.Words = i;
//                                  break;
//                              }
//                              detectorRegMap.write_word(this->body[2+i], baseTransactionAddress + i);
//                              if(this->output) this->output->sendMessage(QString::asprintf("writing 0x%08X to 0x%08X",this->body[2 + i], baseTransactionAddress + i), file);
//                          }
//                          break;}
//            case RMWsum:
//            case RMWbits: {/*if(this->output)this->output->sendMessage("RMWbits transaction detected");*/
//                          amount_of_words = RMWbits_response + itterator_response > max_words_per_packet ? 1 : 2;
//                          header.InfoCode = header.Words != 1 ? 0x1 : 0;
//                          if(whereIsProhibited(detectorRegMap, baseTransactionAddress, 1) == 0){
//                              header.Words = 0;
//                              header.InfoCode = 0x4;
//                              data_to_send = new quint32;
//                              amount_of_words = 1;
//                              break;
//                          }
//                          data_to_send = new quint32[RMWbits_response];
//                          IPbus_word result_bits = this->header.TypeID == RMWbits ? (detectorRegMap.getValue(baseTransactionAddress) & this->body[2]) | this->body[3]:
//                                                                                     detectorRegMap.getValue(baseTransactionAddress) + this->body[2];
//                          data_to_send[1] = detectorRegMap.getValue(baseTransactionAddress);
//                          if(detectorRegMap.isFIFO(baseTransactionAddress)){
//                              if(this->output) this->output->sendMessage(QString::asprintf("writing 0x%08X to FIFO 0x%08X: forbidden",
//                                                                                           result_bits, baseTransactionAddress), file);
//                              header.Words = 0;
//                              header.InfoCode = 0x5;
//                              amount_of_words = 1;
//                          }else{
//                              detectorRegMap.write_word(result_bits, baseTransactionAddress);
//                              if(this->output) this->output->sendMessage(QString::asprintf("writing 0x%08X to 0x%08X (RMWoperation)",
//                                                                                           result_bits, baseTransactionAddress), file);
//                          }
//                          break;}
//            case nonIncrementingWrite:{if(this->output)this->output->sendMessage("non Incrementing Write transaction detected");
//                          data_to_send = new IPbus_word;
//                          amount_of_words = 1;
//                          header.InfoCode = whereIsProhibited(detectorRegMap, baseTransactionAddress, 1) == 0 ? 0x5 : 0x0;
//                          if(header.InfoCode == 0x5) break;
//                          for(quint16 i = 0; i < header.Words; ++i){
//                              detectorRegMap.write_word(this->body[2 + i], baseTransactionAddress);
//                              if(this->output) this->output->sendMessage(QString::asprintf("writing 0x%08X to 0x%08X (non-incrementing)",this->body[2 + i], baseTransactionAddress), file);
//                          }
//                          break;}

//            default: data_to_send = new IPbus_word;
//                     header.InfoCode = 1;
//                     amount_of_words = 1;
//                     break;
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





