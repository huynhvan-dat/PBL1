#include "PBL.h"

#include <fstream>
#include <iostream>

int main() {
    std::ofstream fileOut("output.txt");

    if (!fileOut.is_open()) {
        std::cout << "Khong the mo file de ghi!\n";
        return 1;
    }

    fileOut << "Kiem tra ghi file thanh cong.\n";
    fileOut << "File nhan vien dang dung: " << EMP_FILE << '\n';
    fileOut << "File dat ban dang dung: " << DATA_FILE << '\n';

    std::cout << "Xuat file thanh cong!\n";
    return 0;
}
