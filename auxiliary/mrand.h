#ifndef MRAND_H
#define MRAND_H

#include <QtGlobal>
#include <QtCore>
#include <QObject>
#include <QTimer>
#include <math.h>
#include <QRandomGenerator>

enum Mdistribution {Muniform, Mgaus};

class Mrand : public QObject{
    Q_OBJECT
public:
    Mrand(Mdistribution type = Muniform, quint32 mean = 0, double sigma = 1,
          quint32 min = 0, quint32 max = 0);
    operator quint32(){
        generate_value();
        return actual;}


private:
    Mdistribution type;
    double rms;
    quint32 mean, min, max, actual, b_unif_border, a_unif_border;


private:
    void generate_value();
    void max_minhandler(quint32&, quint32&);

};

#endif // MRAND_H
