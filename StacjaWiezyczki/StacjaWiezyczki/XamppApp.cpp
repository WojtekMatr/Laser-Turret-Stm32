#include "XamppApp.h"
#include <iostream>


XamppApp baza;
XamppApp::XamppApp() : conn(nullptr), id_aktualnego_polaczenia(0), polaczono(false) {}



bool XamppApp::Polacz() {
    conn = mysql_init(NULL);
    if (mysql_real_connect(conn, "127.0.0.1", "root", "", "bazawiezyczki", 3306, NULL, 0)) {        
        polaczono = true;
        return true;
    }
    std::cout << "Problem z polaczeniem\n";
    return false;
}

void XamppApp::Start() {
    if (!polaczono) return;
    std::string query = "INSERT INTO Polaczenie (godzina_polaczenia) VALUES (NOW())";
    if (mysql_query(conn, query.c_str()) == 0) {
        id_aktualnego_polaczenia = mysql_insert_id(conn);
        std::cout << "Nowa Sesja: " << id_aktualnego_polaczenia << "\n";
    }
}

void XamppApp::ZapiszRuch(int trybWiezy, int impulsX, int impulsY) {
    if (!polaczono || id_aktualnego_polaczenia == 0) return;

    char query[256];
    sprintf_s(query, "INSERT INTO Ruch (id_polaczenia, id_typu, data_ruchu, impuls_os_x, impuls_os_y) VALUES (%d, 1, NOW(), %d, %d)",
        id_aktualnego_polaczenia, impulsX, impulsY);

    mysql_query(conn, query);
}

void XamppApp::ZapiszStrzal() {
    if (!polaczono || id_aktualnego_polaczenia == 0) return;

    char query[256];
    sprintf_s(query, "INSERT INTO Strzal (id_polaczenia, data_strzalu) VALUES (%d, NOW())", id_aktualnego_polaczenia);

    mysql_query(conn, query);
}

void XamppApp::Rozlacz() {
    if (polaczono && id_aktualnego_polaczenia != 0) {
        char query[256];
        sprintf_s(query, "UPDATE Polaczenie SET godzina_zerwania = NOW() WHERE id_polaczenia = %d", id_aktualnego_polaczenia);
        mysql_query(conn, query);
    }
    if (conn) {
        mysql_close(conn);
    }
    polaczono = false;
}
void XamppApp::Aktualizacja() {



}
