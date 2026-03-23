#include <iostream>
#include <windows.h>
#include <conio.h> // _getch , _kbhit
#include <string>

using namespace std;

int main() {
    // COM3 jest przypisany do esp
    string portName = "\\\\.\\COM3";

    cout << "Laczenie " << portName << "..." << endl;

    HANDLE hSerial = CreateFileA(portName.c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    if (hSerial == INVALID_HANDLE_VALUE) {
        system("pause");
        return 1;
    }

    DCB dcbSerialParams = { 0 };
    dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
    if (!GetCommState(hSerial, &dcbSerialParams)) {
        CloseHandle(hSerial);
        return 1;
    }

    dcbSerialParams.BaudRate = CBR_115200; //115200 bitow na sekunde
    dcbSerialParams.ByteSize = 8; //rozmiar danych
    dcbSerialParams.StopBits = ONESTOPBIT;//
    dcbSerialParams.Parity = NOPARITY;//brak bitow kontrolnych 


    if (!SetCommState(hSerial, &dcbSerialParams)) {
        CloseHandle(hSerial);
        return 1;
    }
    

    cout << "###########################   Wieza STM32    ###########################" << endl;
    cout << "##### WSAD sterowanie , Q wyjscie, 1 zatrzymanie maszyny, 2 tryby  #####" << endl;

    char key;
    DWORD bytes_written;

    while (true) {
        if (_kbhit()) { //metoda ktora sprawdza czy jest klawisz
            key = _getch(); // _getch() bez enter

            if (key == 'q' || key == 'Q') {
                cout << "\nZamykam" << endl;
                break;
            }
            if (key == '1') {
                cout << "Wybrano menu zmiany trybu, jaki chcesz tryb?" << endl;
            }
            else if (key == '2') {

            }

            else if (key == 'w' || key == 'W' || key == 's' || key == 'S' ||
                key == 'a' || key == 'A' || key == 'd' || key == 'D' || key == ' ') {

                // wyslany znak do esp32
                WriteFile(hSerial, &key, 1, &bytes_written, NULL);
    
                cout << "Komenda: " << key << endl;
            }
        }
        // bez sleep niebezpiecznie
        Sleep(10);
    }
    //zamkniecie portu
    CloseHandle(hSerial);
    return 0;
}