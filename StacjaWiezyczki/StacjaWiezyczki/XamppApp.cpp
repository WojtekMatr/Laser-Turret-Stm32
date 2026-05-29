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
bool XamppApp::getPolaczono() {
    return this->polaczono;
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
void XamppApp::PobierzDaneDoZakladek() {
    if (!polaczono) return;

    // 1. Pobieranie tabeli Polaczenie
    lista_polaczen.clear();
    if (mysql_query(conn, "SELECT id_polaczenia, godzina_polaczenia, godzina_zerwania FROM Polaczenie ORDER BY id_polaczenia DESC LIMIT 50") == 0) {
        MYSQL_RES* res = mysql_store_result(conn);
        if (res) {
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(res))) {
                lista_polaczen.push_back({
                    row[0] ? row[0] : "-",
                    row[1] ? row[1] : "-",
                    row[2] ? row[2] : "W toku..."
                    });
            }
            mysql_free_result(res);
        }
    }

    // 2. Pobieranie tabeli Ruch
    lista_ruchow.clear();
    if (mysql_query(conn, "SELECT id_ruchu, id_polaczenia, id_typu, data_ruchu, impuls_os_x, impuls_os_y FROM Ruch ORDER BY id_ruchu DESC LIMIT 50") == 0) {
        MYSQL_RES* res = mysql_store_result(conn);
        if (res) {
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(res))) {
                lista_ruchow.push_back({
                    row[0] ? row[0] : "-", row[1] ? row[1] : "-", row[2] ? row[2] : "-",
                    row[3] ? row[3] : "-", row[4] ? row[4] : "-", row[5] ? row[5] : "-"
                    });
            }
            mysql_free_result(res);
        }
    }

    // 3. Pobieranie tabeli Strzal
    lista_strzalow.clear();
    if (mysql_query(conn, "SELECT * FROM Strzal ORDER BY 1 DESC LIMIT 50") == 0) {
        MYSQL_RES* res = mysql_store_result(conn);
        if (res) {
            MYSQL_ROW row;
            int num_fields = mysql_num_fields(res);
            while ((row = mysql_fetch_row(res))) {
                // Bezpieczne mapowanie kolumn niezale¿nie od dok³adnej struktury tabeli Strzal
                lista_strzalow.push_back({
                    row[0] ? row[0] : "-",
                    (num_fields > 1 && row[1]) ? row[1] : "-",
                    (num_fields > 2 && row[2]) ? row[2] : "-",
                    (num_fields > 3 && row[3]) ? row[3] : "-",
                    (num_fields > 4 && row[4]) ? row[4] : ((num_fields > 1 && row[1]) ? row[1] : "-")
                    });
            }
            mysql_free_result(res);
        }
    }

    // 4. Pobieranie tabeli Obiekt wykryty
    lista_obiektow.clear();
    if (mysql_query(conn, "SELECT * FROM Obiekt ORDER BY 1 DESC LIMIT 50") == 0) { // upewnij siê czy tabela nazywa siê 'Obiekt'
        MYSQL_RES* res = mysql_store_result(conn);
        if (res) {
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(res))) {
                lista_obiektow.push_back({
                    row[0] ? row[0] : "-", row[1] ? row[1] : "-",
                    row[2] ? row[2] : "-", row[3] ? row[3] : "-"
                    });
            }
            mysql_free_result(res);
        }
    }
}