#include "PBL.h"
#include <iostream>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <limits>
#include <ctime>

using namespace std;

// Khởi tạo biến tĩnh và singleton
int Customer::totalCustomers = 0;
RestaurantManager* RestaurantManager::instance = nullptr;

// Menu đồ ăn và đặt món
static const vector<MenuItem> RESTAURANT_MENU = {
    {1, "Nộm đu đủ bò khô (Món khai vị)" , 50000},
    {2, "Cơm gà xối mỡ", 65000},
    {3, "Lẩu hải sản", 220000},
    {4, "Salad cá ngừ", 75000},
    {5, "Hàu nướng mỡ hành", 80000},
    {6, "Tôm sú nướng", 75000},
    {7, "Lẩu thái", 200000},
    {8, "Trà tắc", 10000},
    {9, "Trà đào", 10000},
    {10, "Đĩa trái cây", 30000}
};

// Xóa phần nhập còn thừa trong bộ đệm, tránh bị trôi getline().
static void clearInputLine() {
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
}

// Lấy ngày hiện tại của máy để ghi vào feedback.
static string getCurrentDate() {
    time_t now = time(nullptr);
    tm* localTime = localtime(&now);
    char buffer[20];
    strftime(buffer, sizeof(buffer), "%d/%m/%Y", localTime);
    return buffer;
}

// Nhập ngày giờ khách sẽ đến và kiểm tra định dạng cơ bản.
static DateTime inputDateTime() {
    DateTime dt;

    while (true) {
        string dateInput;
        cout << "Nhập ngày sẽ tới (dd/mm/yy hoặc dd mm yy): ";
        getline(cin, dateInput);

        for (char& ch : dateInput) {
            if (ch == '/' || ch == '-') ch = ' ';
        }

        stringstream ss(dateInput);
        if (ss >> dt.day >> dt.month >> dt.year &&
            dt.day >= 1 && dt.day <= 31 &&
            dt.month >= 1 && dt.month <= 12 &&
            dt.year >= 2026) {
            break;
        }

        cout << "[Lỗi] Ngày không hợp lệ. Ví dụ đúng: 02/03/2027\n";
    }

    while (true) {
        string timeInput;
        cout << "Nhập giờ sẽ tới (h:m hoặc h m): ";
        getline(cin, timeInput);

        for (char& ch : timeInput) {
            if (ch == ':') ch = ' ';
        }

        stringstream ss(timeInput);
        if (ss >> dt.hour >> dt.minute &&
            dt.hour >= 0 && dt.hour <= 23 &&
            dt.minute >= 0 && dt.minute <= 59) {
            break;
        }

        cout << "[Lỗi] Giờ không hợp lệ. Ví dụ đúng: 19:30\n";
    }

    return dt;
}

// In danh sách món ăn để khách chọn món đặt trước.
static void displayMenuItems() {
    cout << "\n--- MENU MÓN ĂN ---\n";
    cout << left << setw(5) << "ID"
         << setw(25) << "Tên món"
         << setw(12) << "Giá" << endl;
    cout << "------------------------------------------\n";
    for (const auto& item : RESTAURANT_MENU) {
        cout << left << setw(5) << item.id
             << setw(25) << item.name
             << fixed << setprecision(0) << item.price << " VND\n";
    }
}

// Tìm món ăn theo ID trong menu.
static const MenuItem* findMenuItem(int id) {
    for (const auto& item : RESTAURANT_MENU) {
        if (item.id == id) return &item;
    }
    return nullptr;
}

// Tính tổng tiền các món khách đã chọn.
static double calculateOrderTotal(const vector<OrderItem>& orders) {
    double total = 0;
    for (const auto& order : orders) {
        total += order.item.price * order.quantity;
    }
    return total;
}

// Hiển thị danh sách món đã đặt trước của một bàn.
static void displayOrderList(const vector<OrderItem>& orders) {
    if (orders.empty()) {
        cout << "   (Chưa có món đặt trước)\n";
        return;
    }
    cout << "   Danh sách món đã đặt:\n";
    for (const auto& order : orders) {
        cout << "   - " << order.item.name
             << " x" << order.quantity
             << " = " << order.item.price * order.quantity << " VND\n";
    }
    cout << "   Tổng tiền món đặt trước: "
         << fixed << setprecision(0) << calculateOrderTotal(orders) << " VND\n";
}

// Cho người dùng nhập nhiều món và số lượng cho từng món.
static vector<OrderItem> inputOrderList() {
    vector<OrderItem> orders;
    int itemID;

    displayMenuItems();
    cout << "Nhập ID món muốn đặt trước (0 để kết thúc): ";
    while (cin >> itemID) {
        if (itemID == 0) break;
        
        const MenuItem* menuItem = findMenuItem(itemID);
        if (menuItem == nullptr) {
            cout << "[Lỗi] Không có món này (1-10). Nhập lại ID món (0 để kết thúc): ";
            continue;
        }

        int quantity;
        cout << "Nhập số lượng cho \"" << menuItem->name << "\": ";
        while (!(cin >> quantity)) {
            cout << "[Lỗi] Vui lòng nhập một số nguyên. Nhập lại: ";
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
        }
        
        if (quantity <= 0) {
            cout << "[Lỗi] Số lượng phải lớn hơn 0.\n";
        } else {
            bool existed = false;
            for (auto& order : orders) {
                if (order.item.id == menuItem->id) {
                    order.quantity += quantity;
                    existed = true;
                    break;
                }
            }
            if (!existed) {
                orders.push_back({*menuItem, quantity});
            }
            cout << "=> Đã thêm món vào đơn.\n";
        }

        cout << "Nhập ID món tiếp theo (0 để kết thúc): ";
    }

    clearInputLine();
    return orders;
}


// Xóa khoảng trắng ở đầu và cuối chuỗi.
static string trimString(const string& text) {
    size_t start = text.find_first_not_of(" \t\r\n");
    size_t end = text.find_last_not_of(" \t\r\n");
    if (start == string::npos || end == string::npos) return "";
    return text.substr(start, end - start + 1);
}

// Tìm món ăn theo tên, dùng khi đọc lại đơn món từ file.
static const MenuItem* findMenuItemByName(const string& name) {
    for (const auto& item : RESTAURANT_MENU) {
        if (item.name == name) return &item;
    }
    return nullptr;
}

// Chuyển một dòng món ăn trong file thành OrderItem.
static OrderItem parseOrderLine(const string& line) {
    OrderItem order;
    size_t posX = line.find(" x");
    size_t posEq = line.find(" = ");
    if (posX == string::npos || posEq == string::npos || posEq <= posX) {
        return order;
    }
    string name = trimString(line.substr(2, posX - 2));
    string qtyText = trimString(line.substr(posX + 2, posEq - (posX + 2)));
    int qty = 0;
    try {
        qty = stoi(qtyText);
    } catch (...) {
        return order;
    }
    const MenuItem* menuItem = findMenuItemByName(name);
    if (menuItem && qty > 0) {
        order.item = *menuItem;
        order.quantity = qty;
    }
    return order;
}

// Ghi toàn bộ danh sách bàn đã đặt ra file.
static void writeReservationsToFile(const string& filename, Table* tables, int totalTables) {
    ofstream outFile(filename);
    if (!outFile) return;

    for (int i = 0; i < totalTables; i++) {
        if (tables[i].getStatus() && tables[i].getCustomer() != nullptr) {
            const DateTime& dt = tables[i].getBookTime();
            outFile << "====================================\n";
            outFile << "BÀN SỐ: " << tables[i].getTableID() << "\n";
            outFile << "TÊN KHÁCH: " << tables[i].getCustomer()->getName() << "\n";
            outFile << "SDT: " << tables[i].getCustomer()->getPhone() << "\n";
            outFile << "THỜI GIAN ĐẾN: "
                    << dt.day << "/" << dt.month << "/" << dt.year
                    << " " << setw(2) << setfill('0') << dt.hour
                    << ":" << setw(2) << setfill('0') << dt.minute << setfill(' ') << "\n";
            outFile << "MÓN ĐÃ ĐẶT:\n";
            for (const auto& order : tables[i].getOrderList()) {
                outFile << "- " << order.item.name
                        << " x" << order.quantity
                        << " = " << order.item.price * order.quantity
                        << " VND\n";
            }
            outFile << "====================================\n\n";
        }
    }
    outFile.close();
}


// Lớp Person
// Khởi tạo thông tin chung cho một người.
Person::Person(string n, string p) : name(n), phone(p) {}
// Hàm hủy ảo để các lớp con được hủy đúng cách.
Person::~Person() {}
// Trả về họ tên.
string Person::getName() const { return name; }
// Trả về số điện thoại.
string Person::getPhone() const { return phone; }



// Lớp Customer
// Tạo khách hàng mới và tự tăng mã khách hàng.
Customer::Customer(string n, string p) : Person(n, p) {
    totalCustomers++;
    customerID = totalCustomers;
}
// Hàm hủy của khách hàng.
Customer::~Customer() {}
// In thông tin cơ bản của khách hàng.
void Customer::displayInfo() const {
    cout << left << setw(4) << customerID << " | Tên: " << setw(15) << name << " | SĐT: " << setw(15) << phone;
}
// Trả về mã khách hàng.
int Customer::getID() const { return customerID; }
// Cập nhật tên và số điện thoại của khách hàng.
void Customer::updateInfo(string n, string p) {
    name = n;
    phone = p;
}
// Trả về tổng số khách hàng đã được tạo.
int Customer::getTotalCustomers() { return totalCustomers; }



// Lớp Employee
// Tạo nhân viên với mã, vai trò, lương cơ bản và số ca.
Employee::Employee(string id, string n, string p, string r, double salary, int s) 
    : Person(n, p), empID(id), role(r), baseSalary(salary), shifts(s) {}
// Hàm hủy của nhân viên.
Employee::~Employee() {}
// Trả về mã nhân viên.
string Employee::getID() const { return empID; }
// Trả về vai trò của nhân viên.
string Employee::getRole() const { return role; }
// Trả về lương cơ bản theo ca.
double Employee::getBaseSalary() const { return baseSalary; }
// Trả về số ca đã làm.
int Employee::getShifts() const { return shifts; }
// Tăng số ca làm thêm 1.
void Employee::addShift() { shifts++; }
// Cập nhật lương cơ bản mới.
void Employee::updateSalary(double newSalary) { baseSalary = newSalary; }
// Tính tổng lương dựa trên lương/ca và số ca.
double Employee::calculatePay() const {
    double total = baseSalary * shifts;
    if (role == "Manager") total *= 1.5; // Phụ cấp cho quản lý
    return total;
}
// In thông tin nhân viên ra màn hình.
void Employee::displayEmployee() const {
    cout << left << setw(10) << empID << setw(20) << name 
        << setw(15) << role << setw(15) << fixed << setprecision(0) << baseSalary 
        << setw(10) << shifts << endl;
}
// Chuyển thông tin nhân viên thành một dòng để lưu file.
string Employee::toFileString() const {
    return empID + "|" + name + "|" + phone + "|" + role + "|" + to_string(baseSalary) + "|" + to_string(shifts);
}



// Lớp HRManager
// Khi tạo HRManager thì nạp danh sách nhân viên từ file.
HRManager::HRManager() {
    loadEmployees(); // Tự động nạp dữ liệu ngay khi đối tượng quản lý được tạo
}
// Giải phóng bộ nhớ các nhân viên được cấp phát bằng new.
HRManager::~HRManager() {
    // Giải phóng bộ nhớ mảng con trỏ động
    for (auto emp : staffList) {
        delete emp;
    }
    staffList.clear();
}
// Xóa khoảng trắng ở đầu và cuối chuỗi khi đọc dữ liệu nhân viên.
string HRManager::trim(const string& str) {
    size_t first = str.find_first_not_of(' ');
    if (string::npos == first) return "";
    size_t last = str.find_last_not_of(' ');
    return str.substr(first, (last - first + 1));
}
// Tạo danh sách nhân viên mẫu khi chưa có file dữ liệu.
void HRManager::noEmployee() {
    // Nap mảng mac dinh theo yeu cau (Ma NV, Ten, SDT, Vai tro, Luong/ca, So ca mac dinh)
    staffList.push_back(new Employee("NV01", "Ngô Nguyên Khang", "123456789", "Manager", 500000, 10));
    staffList.push_back(new Employee("NV02", "Huỳnh Văn Đạt", "123456789", "Staff", 200000, 10));
    staffList.push_back(new Employee("NV03", "Lê Nguyễn Quốc Huy", "123456789", "Staff", 200000, 10));
    staffList.push_back(new Employee("NV04", "Lê Anh Khoa", "123456789", "Staff", 200000, 10));
    staffList.push_back(new Employee("NV05", "Hà Huy AN", "123456789", "Staff", 200000, 10));
}
// Đọc danh sách nhân viên từ file employees.txt.
void HRManager::loadEmployees() {
    ifstream inFile(EMP_FILE);
    
    // TRƯỜNG HỢP 1: File chưa từng tồn tại (Lần đầu chạy ứng dụng)
    if (!inFile) {
        cout << "[He thong] Khoi tao co so du lieu nhan su mac dinh ban dau...\n";
        // Tạo dữ liệu mẫu mặc định
        noEmployee();
        
        // Tu dong tao file va luu lai ngay lap tuc
        saveEmployees();
        return;
    }

    string line;
    while (getline(inFile, line)) {
        if (line.empty()) continue;

        stringstream ss(line);
        string id, name, phone, role, salaryStr, shiftStr;

        // Tách chuỗi theo ký tự phân tách '|'
        getline(ss, id, '|');
        getline(ss, name, '|');
        getline(ss, phone, '|');
        getline(ss, role, '|');
        getline(ss, salaryStr, '|');
        getline(ss, shiftStr, '|');

        double salary = stod(trim(salaryStr));
        int shifts = stoi(trim(shiftStr));

        staffList.push_back(new Employee(trim(id), trim(name), trim(phone), trim(role), salary, shifts));
    }
    inFile.close();

    // TRƯỜNG HỢP 2: File ton tai nhung ai do da xoa het chuoi (File trong rong)
    if (staffList.empty()) {
        cout << "[He thong] File du lieu trong. Tu dong nap lai danh sach goc...\n";
        noEmployee();
        saveEmployees();
    }
}
// Lưu danh sách nhân viên hiện tại vào file.
void HRManager::saveEmployees() {
    ofstream outFile(EMP_FILE);
    
    // Xem file có thực sự mở được không
    if (!outFile) {
        cout << "[ERROR] Khong the mo file " << EMP_FILE << " de ghi dữ liệu! Kiếm tra lại quyền truy cập!\n";
        return;
    }

    for (auto emp : staffList) {
        outFile << emp->toFileString() << endl;
    }

    outFile.close();
}
// Xuất bảng lương của nhân viên ra file báo cáo.
void HRManager::exportPayroll() {
    ofstream outFile(PAYROLL_FILE);
    if (!outFile) {
        cout << "[Loi] KHÔNG THỂ XUẤT FILE BÁO CÁO LƯƠNG!\n";
        return;
    }
    outFile << "========================================================\n";
    outFile << "                 BẢNG LƯƠNG THANH TOÁN                  \n";
    outFile << "========================================================\n";
    outFile << left << setw(10) << "Mã NV" << setw(20) << "Tên Nhân Viên" 
            << setw(10) << "Số ca" << setw(15) << "TỔNG LƯƠNG" << "\n";
    outFile << "--------------------------------------------------------\n";
    
    for (auto emp : staffList) {
        outFile << left << setw(10) << emp->getID() 
                << setw(20) << emp->getName() 
                << setw(10) << emp->getShifts() 
                << fixed << setprecision(0) << emp->calculatePay() << "\n";
    }
    outFile << "========================================================\n";
    outFile.close();
    cout << "=> Đã xuất file " << PAYROLL_FILE << " thành công!\n";
}
// Hiển thị toàn bộ nhân viên trong nhà hàng.
void HRManager::displayAllStaff() {
    cout << "\n-------------------------------------------------------------\n";
    cout << left << setw(10) << "Mã NV" << setw(20) << "Tên Nhân Viên" 
        << setw(15) << "Vai Trò" << setw(15) << "Lương/Ca" << setw(10) << "Số ca\n";
    cout << "-------------------------------------------------------------\n";
    for (auto emp : staffList) emp->displayEmployee();
    cout << "-------------------------------------------------------------\n";
}
// Kiểm tra mã nhân viên để biết người đó là Manager, Staff hay không tồn tại.
string HRManager::authenticate(string empID) {
    for (auto emp : staffList) {
        if (emp->getID() == empID) {
            return emp->getRole();
        }
    }
    return "None";
}
// Thêm nhân viên mới vào danh sách.
void HRManager::addEmployee() {
    string id, name, phone, role;
    double salary;
    cout << "\n--- THÊM NHÂN VIÊN MỚI ---\n";
    cout << "Nhập Mã NV (VD: NV06): "; cin >> id;
    if (authenticate(id) != "None") {
        cout << "[Lỗi] Mã nhân viên đã tồn tại trong nhà hàng!\n"; 
        return;
    }
    cin.ignore();
    cout << "Họ và Tên: "; getline(cin, name);
    cout << "SDT: "; getline(cin, phone);
    cout << "Vai trò (Manager/Staff): "; cin >> role;
    if (role != "Manager" && role != "Staff") {
        cout << "[Lỗi] Vai trò không hợp lệ ! Vui lòng chọn 'Manager' hoặc 'Staff'.\n"; 
        return;
    }
    cout << "Lương cơ bản mỗi ca: "; cin >> salary;

    staffList.push_back(new Employee(id, name, phone, role, salary, 0));
    cout << "=> THÊM NHÂN VIÊN THÀNH CÔNG!\n";
}
// Xóa nhân viên khỏi danh sách theo mã nhân viên.
void HRManager::removeEmployee() {
    string id;
    cout << "\n--- SA THẢI NHÂN VIÊN ---\n";
    cout << "Nhập mã NV bạn muốn sa thải "; cin >> id;
    for (auto it = staffList.begin(); it != staffList.end(); ++it) {
        if ((*it)->getID() == id) {
            delete *it; // Xóa vùng nhớ động trước
            staffList.erase(it); // Xóa khỏi danh sách Vector
            cout << "=> ĐÃ XÓA NHÂN VIÊN NÀY KHỎI HỆ THỐNG!\n";
            return;
        }
    }
    cout << "[Lỗi] KHÔNG TÌM THẤY NHÂN VIÊN HỢP LỆ!\n";
}
// Điểm danh thêm ca làm hoặc cập nhật lương nhân viên.
void HRManager::manageSalaryAndShifts() {
    string id;
    int opt;
    cout << "\n--- ĐIỀU CHỈNH LƯƠNG/CÔNG NHÂN VIÊN ---\n";
    cout << "Nhập Mã NV: "; cin >> id;
    for (auto emp : staffList) {
        if (emp->getID() == id) {
            cout << "Tên nhân viên: "; cout << emp->getName() << endl;
            cout << "1. ĐIỂM DANH TĂNG 1 CA LÀM (Check-in)\n2. CẬP NHẬP MỨC LƯƠNG CỨNG\nChon: "; 
            cin >> opt;
            if (opt == 1) {
                emp->addShift();
                cout << "=> Ghi nhân ca làm việc thành công!\n";
            } else if (opt == 2) {
                double newSal;
                cout << "Nhập mức lương mới: "; cin >> newSal;
                emp->updateSalary(newSal);
                cout << "=> Cập nhập mức lương mới cho nhân viên thành công!\n";
            }
            
            exportPayroll();

            return;
        }
    }
    cout << "[Lỗi] Không tìm thấy mã nhân viên hợp lệ!\n";
}
// Xem các feedback khách hàng đã gửi.
void HRManager::viewFeedbacks() {
    ifstream inFile(FEEDBACK_FILE);
    cout << "\n=== DANH SÁCH PHẢN HỒI CỦA KHÁCH HÀNG ===\n";
    if (!inFile) {
        cout << "(CHƯA CÓ PHẢN HỒI NÀO TRONG HỆ THỐNG)\n"; return;
    }
    string line;
    while (getline(inFile, line)) {
        cout << line << endl;
    }
    inFile.close();
}



// Lớp Table
// Tạo bàn ăn với mã bàn và sức chứa ban đầu.
Table::Table(int id, int cap) : tableID(id), capacity(cap), isBooked(false), bookedBy(nullptr) {}
// Sao chép bàn, bao gồm cả thông tin khách nếu bàn đã được đặt.
Table::Table(const Table& other)
    : tableID(other.tableID),
      capacity(other.capacity),
      isBooked(other.isBooked),
      bookedBy(other.bookedBy != nullptr ? new Customer(*other.bookedBy) : nullptr),
      bookTime(other.bookTime),
      orderList(other.orderList) {}
// Gán dữ liệu từ bàn khác sang bàn hiện tại.
Table& Table::operator=(const Table& other) {
    if (this == &other) {
        return *this;
    }

    Customer* copiedCustomer = other.bookedBy != nullptr ? new Customer(*other.bookedBy) : nullptr;
    delete bookedBy;

    tableID = other.tableID;
    capacity = other.capacity;
    isBooked = other.isBooked;
    bookedBy = copiedCustomer;
    bookTime = other.bookTime;
    orderList = other.orderList;

    return *this;
}
// Xóa khách đang gắn với bàn để tránh rò rỉ bộ nhớ.
Table::~Table() {
    delete bookedBy;
}
// Trả về số bàn.
int Table::getTableID() const { return tableID; }
// Trả về số chỗ ngồi của bàn.
int Table::getCapacity() const { return capacity; }
// Kiểm tra bàn đã được đặt hay chưa.
bool Table::getStatus() const { return isBooked; }
// Trả về khách đang đặt bàn này.
Customer* Table::getCustomer() const { return bookedBy; }
// Trả về thời gian khách hẹn đến.
DateTime Table::getBookTime() const { return bookTime; }
// Trả về danh sách món đặt trước.
const vector<OrderItem>& Table::getOrderList() const { return orderList; }
// Đặt bàn không kèm thời gian, dùng cho trường hợp đơn giản.
bool Table::bookTable(Customer* c) {
    if (isBooked){
        return false;
    }

    bookedBy = new Customer(*c);
    isBooked = true;
    orderList.clear();
    return true;
}
// Đặt bàn có kèm ngày giờ khách sẽ đến.
bool Table::bookTable(Customer* c, DateTime dt) {
    if (isBooked) {
        return false;
    }
    bookedBy = new Customer(*c);
    isBooked = true;
    bookTime = dt;
    orderList.clear();
    return true;
}
// Lưu danh sách món đặt trước cho bàn.
void Table::setOrderList(const vector<OrderItem>& orders) {
    orderList = orders;
}
// Giải phóng bàn, xóa khách và món đã đặt.
void Table::freeTable() {
    if (isBooked) {
        delete bookedBy;
        bookedBy = nullptr;
        isBooked = false;
        orderList.clear();
        }
}
// Hiển thị trạng thái chi tiết của một bàn.
void Table::displayTable() const {
    cout << "Bàn số: " << setw(3) << tableID << " | Chỗ ngồi: " << setw(3) << capacity 
                << " | Trạng thái: " << (isBooked ? "Đã Đặt" : "Trống") << endl;
    if (isBooked && bookedBy != nullptr) {
        cout << "   -> Thông tin: ";
        bookedBy->displayInfo();
        cout << endl;
    }
}



// Lớp RestaurantManager
// Lấy đối tượng quản lý nhà hàng duy nhất theo mẫu Singleton.
RestaurantManager* RestaurantManager::getInstance(int numTables) {
    if (instance == nullptr) {
        instance = new RestaurantManager(numTables);
    }
    return instance;
}
// Khởi tạo danh sách bàn và sơ đồ vị trí bàn trong nhà hàng.
RestaurantManager::RestaurantManager(int numTables) {
    totalTables = numTables;
    tables = new Table[totalTables];
    for (int i = 0; i < totalTables; i++) {
        tables[i] = Table(i + 1, (i % 2 == 0) ? 4 : 6);
    }
    int currentTable = 0;
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            if (currentTable < totalTables && (i + j) % 2 == 0) {
                floorPlan[i][j] = tables[currentTable].getTableID();
                currentTable++;
            } else {
                floorPlan[i][j] = 0;
            }
        }
    }
    loadReservationsFromFile(RESERVATION_FILE);
}
// Giải phóng mảng bàn khi tắt chương trình.
RestaurantManager::~RestaurantManager() {
    delete[] tables;
}
// In sơ đồ bàn: bàn trống, bàn đã đặt và lối đi.
void RestaurantManager::displayFloorPlan() {
    cout << "\n=== SƠ ĐỒ NHÀ HÀNG ===\n";
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            if (floorPlan[i][j] == 0) cout << "[   ] ";
            else {
                int id = floorPlan[i][j] - 1;
                if (tables[id].getStatus()) cout << "[ X ] ";
                else cout << "[ " << floorPlan[i][j] << " ] ";
            }
        }
        cout << endl;
    }
}
// In thông tin trạng thái của tất cả bàn.
void RestaurantManager::displayAllTables() {
    cout << "\n=== DANH SÁCH TẤT CẢ CÁC BÀN ===\n";
    for (int i = 0; i < totalTables; i++) tables[i].displayTable();
}
// Nhân viên hoặc quản lý đặt bàn trực tiếp cho khách.
void RestaurantManager::addReservation() {
    int tableID;
    string name, phone;
    cout << "\n--- ĐẶT BÀN ---" << endl;
    displayFloorPlan();
    cout << "\nChú thích:\n"
        << "- [ ID ] = Bàn trống\n"
        << "- [ X  ] = Bàn đã đặt\n"
        << "- [    ] = Lối đi\n";

    cout << "Nhập ID bàn: "; cin >> tableID;
    if (tableID < 1 || tableID > totalTables || tables[tableID - 1].getStatus()) {
        cout << "Không hợp lệ hoặc bàn đã bị đặt!\n"; return;
    }
    cin.ignore();
    cout << "Tên: "; getline(cin, name);
    cout << "SDT: "; getline(cin, phone);

    DateTime dt = inputDateTime();

    Customer newCust(name, phone);
    if (!tables[tableID - 1].bookTable(&newCust, dt)) {
        cout << "[Lỗi] Đặt bàn thất bại vì bàn đã bị đặt từ lúc vừa kiểm tra.\n";
        return;
    }

    cout << "\nBạn có muốn đặt món trước cho khách hàng không? (y/n): ";
    char chooseDish = 'n';
    cin >> chooseDish;
    if (chooseDish == 'y' || chooseDish == 'Y') {
        vector<OrderItem> orders = inputOrderList();
        if (!orders.empty()) {
            tables[tableID - 1].setOrderList(orders);
            cout << "=> Đã lưu danh sách món cho bàn " << tableID << "!\n";
        } else {
            cout << "=> Không có món nào được chọn. Bỏ qua phần đặt món.\n";
        }
    }

    exportReservationToFile(tableID);
    cout << "=> ĐẶT BÀN THÀNH CÔNG! Thông tin đã được lưu vào " << RESERVATION_FILE << "\n";
}
// Khách tự đặt bàn online, có thể chọn món trước.
void RestaurantManager::addOnlineReservation() {
    int tableID;
    string name, phone;

    cout << "\n--- ĐẶT BÀN ONLINE ---" << endl;
    displayFloorPlan();
    cout << "\nChú thích:\n"
        << "- [ id ] = Bàn trống\n"
        << "- [ X  ] = Bàn đã đặt\n"
        << "- [    ] = Lối đi\n";

    cout << "Nhập ID bàn muốn đặt: ";
    cin >> tableID;
    if (tableID < 1 || tableID > totalTables || tables[tableID - 1].getStatus()) {
        cout << "Không hợp lệ hoặc bàn đã bị đặt!\n";
        return;
    }

    clearInputLine();
    cout << "TÊN KHÁCH HÀNG : ";
    getline(cin, name);
    cout << "SỐ ĐIỆN THOẠI : ";
    getline(cin, phone);

    vector<OrderItem> orders = inputOrderList();
    DateTime dt = inputDateTime();

    Customer newCust(name, phone);
    if (tables[tableID - 1].bookTable(&newCust, dt)) {
        tables[tableID - 1].setOrderList(orders);
        exportReservationToFile(tableID);
        cout << "=> ĐẶT BÀN THÀNH CÔNG ! THÔNG TIN CỦA BẠN ĐÃ ĐƯỢC LƯU VÀO ! "
             << RESERVATION_FILE << "\n";
    }
}

// Lưu lại dữ liệu đặt bàn sau khi có thay đổi.
void RestaurantManager::exportReservationToFile(int tableID) {
    (void)tableID;
    saveReservationsToFile(RESERVATION_FILE);
}

// Gọi hàm ghi file cho danh sách đặt bàn.
void RestaurantManager::saveReservationsToFile(const string& filename) {
    writeReservationsToFile(filename, tables, totalTables);
}

// Đọc lại các bàn đã đặt từ file khi mở chương trình.
void RestaurantManager::loadReservationsFromFile(const string& filename) {
    ifstream inFile(filename);
    if (!inFile) return;

    string line;
    int currentTableID = 0;
    string currentName;
    string currentPhone;
    DateTime currentDT;
    vector<OrderItem> currentOrders;
    bool inOrderSection = false;

    auto commitReservation = [&]() {
        if (currentTableID > 0 && currentTableID <= totalTables && !currentName.empty()) {
            Customer newCust(currentName, currentPhone);
            if (tables[currentTableID - 1].bookTable(&newCust, currentDT)) {
                tables[currentTableID - 1].setOrderList(currentOrders);
            }
        }
        currentTableID = 0;
        currentName.clear();
        currentPhone.clear();
        currentOrders.clear();
        currentDT = DateTime();
        inOrderSection = false;
    };

    while (getline(inFile, line)) {
        line = trimString(line);
        if (line.empty()) continue;

        if (line.find("====================================") == 0) {
            if (currentTableID != 0) {
                commitReservation();
            }
            continue;
        }

        try {
            if (line.rfind("BÀN SỐ:", 0) == 0) {
                currentTableID = stoi(trimString(line.substr(7)));
                continue;
            }

            if (line.rfind("TÊN KHÁCH:", 0) == 0) {
                currentName = trimString(line.substr(10));
                continue;
            }

            if (line.rfind("SDT:", 0) == 0) {
                currentPhone = trimString(line.substr(4));
                continue;
            }

            if (line.rfind("THỜI GIAN ĐẾN:", 0) == 0) {
                string timeText = trimString(line.substr(14));
                stringstream ss(timeText);
                char sep;
                ss >> currentDT.day >> sep >> currentDT.month >> sep >> currentDT.year >> currentDT.hour >> sep >> currentDT.minute;
                continue;
            }

            if (line == "MÓN ĐÃ ĐẶT:") {
                inOrderSection = true;
                continue;
            }

            if (inOrderSection && line.rfind("- ", 0) == 0) {
                OrderItem order = parseOrderLine(line);
                if (order.quantity > 0) {
                    currentOrders.push_back(order);
                }
                continue;
            }
        } catch (const exception& e) {
            // Nếu có lỗi parse, bỏ qua dòng đó và tiếp tục
            continue;
        }
    }

    if (currentTableID != 0) {
        commitReservation();
    }

    inFile.close();
}
// Hủy một đặt bàn theo ID bàn.
void RestaurantManager::deleteReservation() {
    int tableID;
    cout << "\n--- HỦY ĐẶT BÀN ---" << endl;
    displayFloorPlan();
    cout << "Nhập ID bàn cần hủy: "; cin >> tableID;
    if (tableID < 1 || tableID > totalTables || !tables[tableID - 1].getStatus()) {
        cout << "ID không hợp lệ hoặc bàn đang trống!\n"; return;
    }
    tables[tableID - 1].freeTable();
    saveReservationsToFile(RESERVATION_FILE);
    cout << "=> HỦY ĐẶT BÀN THÀNH CÔNG!\n";
}
// Xuất hóa đơn, lưu vào file hoadon.txt rồi giải phóng bàn.
void RestaurantManager::checkoutTable() {
    int tableID;
    cout << "\n--- THANH TOÁN / XUẤT HÓA ĐƠN ---" << endl;
    displayFloorPlan();
    cout << "Nhập ID bàn cần thanh toán: ";
    cin >> tableID;

    if (tableID < 1 || tableID > totalTables || !tables[tableID - 1].getStatus()) {
        cout << "ID không hợp lệ hoặc bàn đang trống!\n";
        return;
    }

    Table& table = tables[tableID - 1];
    Customer* customer = table.getCustomer();
    const vector<OrderItem>& orders = table.getOrderList();
    double total = calculateOrderTotal(orders);

    cout << "\n========== HÓA ĐƠN THANH TOÁN ==========\n";
    cout << "Bàn số: " << table.getTableID() << "\n";
    if (customer != nullptr) {
        cout << "Tên khách: " << customer->getName() << "\n";
        cout << "SĐT: " << customer->getPhone() << "\n";
    }
    cout << "----------------------------------------\n";
    if (orders.empty()) {
        cout << "(Khách chưa đặt món trước)\n";
    } else {
        cout << "Danh sách món:\n";
        for (const auto& order : orders) {
            cout << "- " << order.item.name
                 << " x" << order.quantity
                 << " = " << fixed << setprecision(0)
                 << order.item.price * order.quantity << " VND\n";
        }
    }
    cout << "----------------------------------------\n";
    cout << "Tổng tiền: " << fixed << setprecision(0) << total << " VND\n";
    cout << "========================================\n";

    ofstream outFile(INVOICE_FILE, ios::app);
    if (outFile) {
        outFile << "========== HÓA ĐƠN THANH TOÁN ==========\n";
        outFile << "Bàn số: " << table.getTableID() << "\n";
        if (customer != nullptr) {
            outFile << "Tên khách: " << customer->getName() << "\n";
            outFile << "SĐT: " << customer->getPhone() << "\n";
        }
        outFile << "Danh sách món:\n";
        if (orders.empty()) {
            outFile << "(Khách chưa đặt món trước)\n";
        } else {
            for (const auto& order : orders) {
                outFile << "- " << order.item.name
                        << " x" << order.quantity
                        << " = " << fixed << setprecision(0)
                        << order.item.price * order.quantity << " VND\n";
            }
        }
        outFile << "Tổng tiền: " << fixed << setprecision(0) << total << " VND\n";
        outFile << "========================================\n\n";
    }

    table.freeTable();
    saveReservationsToFile(RESERVATION_FILE);
    cout << "=> Đã lưu hóa đơn vào " << INVOICE_FILE << " và giải phóng bàn thành công!\n";
}
// Khách tự hủy đặt bàn bằng cách xác thực số điện thoại.
void RestaurantManager::customerDeleteReservation() {
    string inputPhone;
    displayFloorPlan();
    cout << "\n--- KHÁCH HÀNG TỰ HỦY ĐẶT BÀN (YÊU CẦU XÁC THỰC THEO SĐT) ---\n";
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    cout << "Nhập số điện thoại đã dùng để đặt bàn: ";
    getline(cin, inputPhone);
    inputPhone = trimString(inputPhone);

    vector<int> matches;
    for (int i = 0; i < totalTables; i++) {
        if (tables[i].getStatus() && tables[i].getCustomer() != nullptr &&
            tables[i].getCustomer()->getPhone() == inputPhone) {
            matches.push_back(i);
        }
    }

    if (matches.empty()) {
        cout << "[Lỗi] Không tìm thấy đặt bàn nào với SĐT này.\n";
        return;
    }

    int selectedIndex = matches[0];
    if (matches.size() > 1) {
        cout << "Tìm thấy " << matches.size() << " đặt bàn với số điện thoại này:\n";
        for (int idx : matches) {
            cout << "- Bàn " << tables[idx].getTableID()
                 << " | Khách: " << tables[idx].getCustomer()->getName()
                 << " | Thời gian: " << tables[idx].getBookTime().day << "/"
                 << tables[idx].getBookTime().month << "/"
                 << tables[idx].getBookTime().year << " "
                 << setw(2) << setfill('0') << tables[idx].getBookTime().hour << ":"
                 << setw(2) << setfill('0') << tables[idx].getBookTime().minute << setfill(' ') << "\n";
        }
        cout << "Nhập ID bàn bạn muốn hủy: ";
        int tableID;
        if (!(cin >> tableID)) {
            cout << "[Lỗi] Dữ liệu nhập không hợp lệ!\n";
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            return;
        }
        bool found = false;
        for (int idx : matches) {
            if (tables[idx].getTableID() == tableID) {
                selectedIndex = idx;
                found = true;
                break;
            }
        }
        if (!found) {
            cout << "[Lỗi] ID bàn không khớp với SĐT đã nhập.\n";
            return;
        }
    }

    cout << "Xác nhận hủy đặt bàn Bàn " << tables[selectedIndex].getTableID()
         << " cho khách " << tables[selectedIndex].getCustomer()->getName()
         << "? (y/n): ";
    char confirm;
    cin >> confirm;

    if (confirm == 'y' || confirm == 'Y') {
        tables[selectedIndex].freeTable();
        saveReservationsToFile(RESERVATION_FILE);
        cout << "=> HỦY ĐẶT BÀN THÀNH CÔNG!\n";
    } else {
        cout << "=> HỦY ĐẶT BÀN ĐÃ BỊ HỦY.\n";
    }
}
// Sửa thông tin khách hoặc đổi danh sách món đã đặt.
void RestaurantManager::editReservation() {
    int tableID;
    cout << "\n--- SỬA THÔNG TIN ĐẶT BÀN ---" << endl;
    displayFloorPlan();
    cout << "Nhập ID bàn: "; cin >> tableID;
    if (tableID < 1 || tableID > totalTables || !tables[tableID - 1].getStatus()) {
        cout << "ID không hợp lệ hoặc bàn đang trống!\n"; return;
    }
    string newName, newPhone;
    cin.ignore();
    cout << "Tên:"; getline(cin, newName);
    cout << "SDT: "; getline(cin, newPhone);
    tables[tableID - 1].getCustomer()->updateInfo(newName, newPhone);

    const vector<OrderItem>& currentOrders = tables[tableID - 1].getOrderList();
    if (!currentOrders.empty()) {
        cout << "\nThông tin món hiện tại của khách:\n";
        displayOrderList(currentOrders);
    } else {
        cout << "\nKhách chưa đặt món trước.\n";
    }

    cout << "\nBạn có muốn đổi món cho khách không? (y/n): ";
    char chooseChange = 'n';
    cin >> chooseChange;
    if (chooseChange == 'y' || chooseChange == 'Y') {
        vector<OrderItem> orders = inputOrderList();
        if (!orders.empty()) {
            tables[tableID - 1].setOrderList(orders);
            cout << "=> Đã cập nhật lại đơn món của khách.\n";
        } else {
            tables[tableID - 1].setOrderList({});
            cout << "=> Đã xóa toàn bộ đơn món hiện tại.\n";
        }
    }

    saveReservationsToFile(RESERVATION_FILE);
    cout << "=> CẬP NHẬT THÔNG TIN THÀNH CÔNG!\n";
}
// Cho khách gửi đánh giá gồm tên, ngày, số sao và nhận xét.
void sendFeedback() {
    string customerName, msg;
    int rating;

    clearInputLine();
    cout << "\n--- GỬI ĐÁNH GIÁ DỊCH VỤ ---\n";

    do {
        cout << "Tên người đánh giá: ";
        getline(cin, customerName);
        customerName = trimString(customerName);
        if (customerName.empty()) {
            cout << "[Lỗi] Tên không được để trống.\n";
        }
    } while (customerName.empty());

    while (true) {
        cout << "Đánh giá dịch vụ (1-5 sao): ";
        if (cin >> rating && rating >= 1 && rating <= 5) {
            break;
        }
        cout << "[Lỗi] Vui lòng nhập số sao từ 1 đến 5.\n";
        cin.clear();
        clearInputLine();
    }

    clearInputLine();
    do {
        cout << "Nhập ý kiến đóng góp của bạn về nhà hàng: ";
        getline(cin, msg);
        msg = trimString(msg);
        if (msg.empty()) {
            cout << "[Lỗi] Nội dung đánh giá không được để trống.\n";
        }
    } while (msg.empty());

    ofstream outFile(FEEDBACK_FILE, ios::app); // Mở ở chế độ Append ghi nối đuôi
    if (outFile) {
        outFile << "====================================\n";
        outFile << "Tên người đánh giá: " << customerName << "\n";
        outFile << "Ngày đánh giá: " << getCurrentDate() << "\n";
        outFile << "Số sao dịch vụ: " << rating << "/5\n";
        outFile << "Nhận xét: " << msg << "\n";
        outFile << "====================================\n\n";
        outFile.close();
        cout << "=> GỬI PHẢN HỒI THÀNH CÔNG. CẢM ƠN BẠN ĐÃ ĐÓNG GÓP Ý KIẾN CHO NHÀ HÀNG.\n";
    } else {
        cout << "[Lỗi] Không thể mở file " << FEEDBACK_FILE << " để lưu phản hồi.\n";
    }
}



// Menu phân quyền
// Menu dành cho quản lý, có đủ quyền về bàn và nhân sự.
void managerInterface(RestaurantManager* res, HRManager& hr) {
    int choice;
    do {
        cout << "\n=========================================\n";
        cout << "        MENU ĐIỀU HÀNH - QUẢN LÝ (MANAGER) \n";
        cout << "=========================================\n";
        cout << "1. Xem sơ đồ nhà hàng hiện tại\n";
        cout << "2. Xem danh sách trạng thái đặt bàn\n";
        cout << "3. Đặt bàn trực tiếp hộ khách\n";
        cout << "4. Hủy bàn bất kỳ (Quyền ADMIN)\n";
        cout << "5. Sửa đổi thông tin đặt lịch\n";
        cout << "6. Xem danh sách hồ sơ nhân viên\n";
        cout << "7. Thêm nhân viên mới vào biên chế\n";
        cout << "8. Sa thải / Xóa nhân viên khỏi hệ thống\n";
        cout << "9. Quản lý ca làm (Check-in) / Sửa lương\n";
        cout << "10. Xem hòm thư góp ý (Feedback khách)\n";
        cout << "11. Xuất file báo cáo lương tổng hợp (.txt)\n";
        cout << "12. Thanh toán / Xuất hóa đơn cho bàn\n";
        cout << "0. Đăng xuất tài khoản Quản lý\n";
        cout << "=========================================\n";
        cout << "Nhập lựa chọn của bạn: "; cin >> choice;

        switch(choice) {
            case 1: res->displayFloorPlan(); break;
            case 2: res->displayAllTables(); break;
            case 3: res->addReservation(); break;
            case 4: res->deleteReservation(); break;
            case 5: res->editReservation(); break;
            case 6: hr.displayAllStaff(); break;
            case 7: hr.addEmployee(); break;
            case 8: hr.removeEmployee(); break;
            case 9: hr.manageSalaryAndShifts(); break;
            case 10: hr.viewFeedbacks(); break;
            case 11: hr.exportPayroll(); break;
            case 12: res->checkoutTable(); break;
            case 0: cout << "=> Đang thoát tài khoản quản lý và đồng bộ file nhân sự...\n"; break;
            default: cout << "Lựa chọn không hợp lệ!\n";
        }
        if(choice != 0) cout << "\n#######################################################\n";
    } while (choice != 0);
}
// Menu dành cho nhân viên, chủ yếu xử lý đặt bàn và thanh toán.
void employeeInterface(RestaurantManager* res) {
    int choice;
    do {
        cout << "\n=========================================\n";
        cout << "        MENU NGHIỆP VỤ - NHÂN VIÊN (STAFF) \n";
        cout << "=========================================\n";
        cout << "1. Xem sơ đồ nhà hàng\n";
        cout << "2. Xem trạng thái tất cả các bàn\n";
        cout << "3. Hỗ trợ khách đặt bàn mới\n";
        cout << "4. Sửa đổi thông tin khách hàng đặt lịch\n";
        cout << "5. Thanh toán / Xuất hóa đơn cho bàn\n";
        cout << "6. Hủy đặt bàn theo yêu cầu khách\n";
        cout << "0. Đăng xuất tài khoản nhân viên\n";
        cout << "=========================================\n";
        cout << "Nhập lựa chọn của bạn: "; cin >> choice;

        switch(choice) {
            case 1: res->displayFloorPlan(); break;
            case 2: res->displayAllTables(); break;
            case 3: res->addReservation(); break;
            case 4: res->editReservation(); break;
            case 5: res->checkoutTable(); break;
            case 6: res->deleteReservation(); break;
            case 0: cout << "=> Đang đăng xuất tài khoản nhân viên...\n"; break;
            default: cout << "Lựa chọn không hợp lệ!\n";
        }
        if(choice != 0) cout << "\n#######################################################\n";
    } while (choice != 0);
}
// Menu dành cho khách hàng tự xem bàn, đặt bàn và gửi feedback.
void customerInterface(RestaurantManager* res) {
    int choice;
    do {
        cout << "\n=========================================\n";
        cout << "         CỔNG THÔNG TIN KHÁCH HÀNG       \n";
        cout << "=========================================\n";
        cout << "1. Xem sơ đồ vị trí bàn trống\n";
        cout << "2. Quét dasnh sách sức chứa các bàn\n";
        cout << "3. Thực hiện Đặt bàn trực tuyến, chọn món trước và ngày giờ tới\n";
        cout << "4. Yêu cầu hủy đặt bàn (Bảo mật SĐT)\n";
        cout << "5. Gửi đánh giá dịch vụ & Feedback đóng góp\n";
        cout << "0. Quay lại màn hình chính Gateway\n";
        cout << "=========================================\n";
        cout << "Nhập lựa chọn của bạn: "; cin >> choice;

        switch(choice) {
            case 1: res->displayFloorPlan(); break;
            case 2: res->displayAllTables(); break;
            case 3: res->addOnlineReservation(); break;
            case 4: res->customerDeleteReservation(); break;
            case 5: sendFeedback(); break;
            case 0: cout << "=> Đang chuyển hướng về cổng Gateway nhà hàng...\n"; break;
            default: cout << "Lựa chọn không hợp lệ!\n";
        }
        if(choice != 0) cout << "\n#######################################################\n";
    } while (choice != 0);
}



// ================================ HÀM ĐIỀU HƯỚNG MAIN (GATEWAY TRUNG TÂM) ================================
// Hàm chính điều hướng người dùng theo vai trò đăng nhập.
int main() {
    RestaurantManager* bkRestaurant = RestaurantManager::getInstance(13);
    
    HRManager hrSystem; 
    
    int accessRole;
    do {
        cout << "\n=======================================================\n";
        cout << "   XIN CHÀO! VUI LÒNG CHỌN VAI TRÒ ĐỂ TRUY CẬP HỆ THỐNG\n";
        cout << "=======================================================\n";
        cout << "1. Quản lý (Manager)\n";
        cout << "2. Nhân viên (Employee/Staff)\n";
        cout << "3. Khách hàng (Customer)\n";
        cout << "0. Tắt hệ thống dữ liệu nhà hàng\n";
        cout << "=======================================================\n";
        cout << "Nhập vai trò của bạn: "; cin >> accessRole;

        if (accessRole == 1) {
            string token;
            cout << "Yêu cầu nhập mã tài khoản quản lý (VD: NV01): "; cin >> token;
            string role = hrSystem.authenticate(token);
            if (role == "Manager") {
                cout << "\n[Xác thực thành công] Xin chào quản lý cấp cao!\n";
                managerInterface(bkRestaurant, hrSystem);
            } else {
                cout << "\n[Truy cập bị từ chối] Mã định danh không đúng hoặc bạn không có quyền hạn Quản lý!\n";
            }
        } 
        else if (accessRole == 2) {
            string token;
            cout << "Yêu cầu nhập mã tài khoản nhân viên (VD: NV02): "; cin >> token;
            string role = hrSystem.authenticate(token);
            if (role == "Staff") {
                cout << "\n[Xác thực thành công] Đăng nhập tài khoản Nhân viên làm việc!\n";
                employeeInterface(bkRestaurant);
            } else {
                cout << "\n[Truy cập bị từ chối] Mã nhân viên không tồn tại trên hệ thống dữ liệu ca trực!\n";
            }
        } 
        else if (accessRole == 3) {
            customerInterface(bkRestaurant);
        }
        
        cout << "\n#######################################################\n";
    } while (accessRole != 0);

    hrSystem.saveEmployees();
    
    delete bkRestaurant;
    
    cout << "=> HỆ THỐNG ĐÃ ĐỒNG BỘ DỮ LIỆU VÀ ĐÓNG CỬA AN TOÀN!\n";
    return 0;
}
