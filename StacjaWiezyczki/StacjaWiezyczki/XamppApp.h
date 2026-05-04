#ifndef XAMPPAPP_H
#define XAMPPAPP_H

#include <mysql.h>
#include <string>

class XamppApp {
private:
    MYSQL* conn;
    int id_aktualnego_polaczenia;
    bool polaczono;

public:
    XamppApp();

    bool Polacz();
    void Start();
    void ZapiszRuch(int trybWiezy, int impulsX, int impulsY);
    void ZapiszStrzal();
    void Rozlacz();
    void Aktualizacja();
};

extern XamppApp baza;

#endif