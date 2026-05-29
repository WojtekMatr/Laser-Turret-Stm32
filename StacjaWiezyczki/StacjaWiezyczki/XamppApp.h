#ifndef XAMPPAPP_H
#define XAMPPAPP_H

#include <mysql.h>
#include <string>
#include <vector>
struct DBPolaczenie { std::string id, start, stop; };
struct DBRuch { std::string id, id_pol, id_typu, data, x, y; };
struct DBStrzal { std::string id, id_pol, id_ruch, id_obj, data; };
struct DBObiekt { std::string data, kategoria, szerokosc, wysokosc; };

class XamppApp {
private:
    MYSQL* conn;
    int id_aktualnego_polaczenia;
    bool polaczono;

public:
    XamppApp();
    std::vector<DBPolaczenie>   lista_polaczen;
    std::vector<DBRuch>         lista_ruchow;
    std::vector<DBStrzal>       lista_strzalow;
    std::vector<DBObiekt>       lista_obiektow;
    bool Polacz();
    void Start();
    void ZapiszRuch(int trybWiezy, int impulsX, int impulsY);
    void ZapiszStrzal();
    void Rozlacz();
    void Aktualizacja();
    void PobierzDaneDoZakladek();
    bool getPolaczono();
};

extern XamppApp baza;

#endif