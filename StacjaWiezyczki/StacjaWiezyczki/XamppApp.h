#ifndef XAMPPAPP_H
#define XAMPPAPP_H

#include <mysql.h>
#include <string>
#include <vector>
struct ZmapowanaLokalizacja {
    std::string id;
    std::string data;
    std::string nazwa;
};
struct PunktSkanu {
    std::string id;
    std::string x;
    std::string y;
    std::string dystans;
    std::string id_lokalizacji;
};
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
    std::vector<ZmapowanaLokalizacja> lista_lokalizacji;
    std::vector<PunktSkanu>     lista_punktow;
    bool Polacz();
    void Start();
    void ZapiszRuch(int trybWiezy, int impulsX, int impulsY);
    void ZapiszStrzal();
    void Rozlacz();
    void Aktualizacja();
    void PobierzDaneDoZakladek();
    int StworzNowaLokalizacje(const std::string& nazwaMiejsca);
    void ZapiszPunktSkanu(int x, int y, unsigned int dystans, int idLokalizacji);
    void ZapiszObiektSkanera(int x, int y, unsigned int dystans);
    bool getPolaczono();
};

extern XamppApp baza;

#endif