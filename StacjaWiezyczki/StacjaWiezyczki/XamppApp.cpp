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
    lista_polaczen.clear();
    if (mysql_query(conn, "SELECT id_polaczenia, godzina_polaczenia, godzina_zerwania FROM Polaczenie ORDER BY id_polaczenia DESC LIMIT 3000") == 0) {
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
    lista_ruchow.clear();
    if (mysql_query(conn, "SELECT id_ruchu, id_polaczenia, id_typu, data_ruchu, impuls_os_x, impuls_os_y FROM Ruch ORDER BY id_ruchu DESC LIMIT 3000") == 0) {
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
    lista_strzalow.clear();
    if (mysql_query(conn, "SELECT * FROM Strzal ORDER BY 1 DESC LIMIT 3000") == 0) {
        MYSQL_RES* res = mysql_store_result(conn);
        if (res) {
            MYSQL_ROW row;
            int num_fields = mysql_num_fields(res);
            while ((row = mysql_fetch_row(res))) {
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
    lista_lokalizacji.clear();
    if (mysql_query(conn, "SELECT id, data, nazwa_miejsca FROM zmapowana_lokalizacja ORDER BY id DESC LIMIT 3000") == 0) {
        MYSQL_RES* res = mysql_store_result(conn);
        if (res) {
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(res))) {
                lista_lokalizacji.push_back({
                    row[0] ? row[0] : "-",
                    row[1] ? row[1] : "-",
                    row[2] ? row[2] : "-"
                    });
            }
            mysql_free_result(res);
        }
    }

    lista_punktow.clear();
    if (mysql_query(conn, "SELECT id, wspolrzedna_x, wspolrzedna_y, odleglosc, id_zmapowanej_lokalizacji FROM skan_miejsca ORDER BY id DESC LIMIT 5000") == 0) {
        MYSQL_RES* res = mysql_store_result(conn);
        if (res) {
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(res))) {
                lista_punktow.push_back({
                    row[0] ? row[0] : "-",
                    row[1] ? row[1] : "-",
                    row[2] ? row[2] : "-",
                    row[3] ? row[3] : "-",
                    row[4] ? row[4] : "-"
                    });
            }
            mysql_free_result(res);
        }
    }

    lista_obiektow.clear();
    if (mysql_query(conn, "SELECT * FROM Obiekt ORDER BY 1 DESC LIMIT 3000") == 0) { 
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
int aktualneIdLokalizacji = -1;

int XamppApp::StworzNowaLokalizacje(const std::string& nazwaMiejsca) {
    if (!polaczono) return -1;

    char query[512];
    // Wstawiamy rekord i od razu przypisujemy datê przez NOW()
    sprintf_s(query, "INSERT INTO zmapowana_lokalizacja (data, nazwa_miejsca) VALUES (NOW(), '%s')", nazwaMiejsca.c_str());

    if (mysql_query(conn, query) == 0) {
        // mysql_insert_id zwraca ID ostatnio dodanego wiersza w tym konkretnym po³¹czeniu
        int nowe_id = (int)mysql_insert_id(conn);
        std::cout << "Utworzono nowa mape (Lokalizacja ID: " << nowe_id << ")\n";
        return nowe_id;
    }

    std::cout << "Blad tworzenia lokalizacji: " << mysql_error(conn) << "\n";
    return -1;
}

void XamppApp::ZapiszPunktSkanu(int x, int y, unsigned int dystans, int idLokalizacji) {
    if (!polaczono || idLokalizacji == -1) return;

    char query[256];
    sprintf_s(query, "INSERT INTO skan_miejsca (wspolrzedna_x, wspolrzedna_y, odleglosc, id_zmapowanej_lokalizacji) VALUES (%d, %d, %u, %d)",
        x, y, dystans, idLokalizacji);

    if (mysql_query(conn, query) != 0) {
        std::cout << "Blad SQL przy dodawaniu punktu: " << mysql_error(conn) << "\n";
    }
}
void XamppApp::ZapiszObiektSkanera(int x, int y, unsigned int dystans) {
    if (!polaczono) return;

    char query[256];
     sprintf_s(query, "INSERT INTO obiekt_wykryty (data, kategoria, szerokosc, wysokosc) VALUES (NOW(), 'Punkt_Lidar', '%d', '%u')", x, dystans);

    mysql_query(conn, query);
}