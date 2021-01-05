#include "mrand.h"
#include <QDebug>



Mrand::Mrand(Mdistribution type, quint32 mean, double sigma, quint32 min, quint32 max){
    max_minhandler(min,max);
    this->type = type;
    this->mean  = mean;
    this->rms  = sigma;
    this->min  = min;
    this->max  = max;
    this->actual = mean;
    a_unif_border = static_cast<quint32>(this->mean - sqrt(3) * this->rms / 3.0);
    b_unif_border = static_cast<quint32>(this->mean + sqrt(3) * this->rms / 3.0);

}

void Mrand::generate_value(){
    switch (this->type) {
        case Mgaus:{quint32 shift;
                    double gamma = double(QRandomGenerator::global()->generate()) / double(0xffffffff);
                    double P = (erf((double(actual) - double(mean)) / (sqrt(2) * this->rms)) + 1.0) / 2.0;
                    if(max != 0) shift = this->min + QRandomGenerator().global()->generate()%(max - min);
                    else shift = QRandomGenerator().global()->generate()%quint32(this->rms);
                    actual += gamma > P ? shift : -shift;
                    break;}
    case Muniform: {actual = a_unif_border + QRandomGenerator().global()->generate()%(b_unif_border - a_unif_border);
                    break;}
    default:        return;
        }
}

void Mrand::max_minhandler(quint32 &min, quint32 &max){
    quint32 a = min;
    if(max < min){
        min = max;
        max = a;
    }
}
