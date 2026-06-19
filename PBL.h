#ifndef PBL_H
#define PBL_H

#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>

// ================================ PATH FILE I/O ================================
const std::string EMP_FILE = "employees.txt";
const std::string PAYROLL_FILE = "payroll_report.txt";
const std::string FEEDBACK_FILE = "feedback.txt";
const std::string RESERVATION_FILE = "datban.txt";
const std::string INVOICE_FILE = "hoadon.txt";


// ================================ SƠ ĐỒ NHÀ HÀNG ================================
const int ROWS = 5; // Số hàng của sơ đồ nhà hàng
const int COLS = 5; // Số cột của sơ đồ nhà hàng



// Cấu trúc thời gian hỗ trợ tính năng đặt lịch nâng cao
struct DateTime {
    int hour = 0;
    int minute = 0;
    int day = 1;
    int month = 1;
    int year = 2026;
};

struct MenuItem {
    int id = 0;
    std::string name;
    double price = 0;
};

struct OrderItem {
    MenuItem item;
    int quantity = 0;
};



// ================================ LỚP CƠ SỞ PERSON ================================
class Person {
    protected:
        std::string name;
        std::string phone;
    public:
        Person(std::string n = "", std::string p = "");
        virtual ~Person();
        std::string getName() const;
        std::string getPhone() const;
};



// ================================ LỚP CON CUSTOMER ================================
class Customer : public Person {
    private:
        int customerID;
        static int totalCustomers; // Biến tĩnh đếm tổng số khách hàng
    public:
        Customer(std::string n = "", std::string p = "");
        virtual ~Customer();
        void displayInfo() const;
        int getID() const;
        void updateInfo(std::string n, std::string p);
        static int getTotalCustomers();
};



// ================================ LỚP CON EMPLOYEE ================================
class Employee : public Person {
    private:
        std::string empID;
        std::string role; // "Manager" hoặc "Staff"
        double baseSalary;
        int shifts;
    public:
        Employee(std::string id = "", std::string n = "", std::string p = "", std::string r = "Staff", double salary = 0, int s = 0);
        virtual ~Employee();

        std::string getID() const;
        std::string getRole() const;
        double getBaseSalary() const;
        int getShifts() const;

        void addShift();
        void updateSalary(double newSalary);
        double calculatePay() const;
        void displayEmployee() const;
        std::string toFileString() const;
};



// ================================ LỚP HRMANAGER ================================
class HRManager {
    private:
        std::vector<Employee*> staffList; // Mảng động đa hình quản lý danh sách nhân sự
        std::string trim(const std::string& str); // Hàm chuẩn hóa chuỗi
    public:
        HRManager();
        ~HRManager();
        void noEmployee();
        void loadEmployees();
        void saveEmployees();
        void exportPayroll();
        void displayAllStaff();
        std::string authenticate(std::string empID);

        // Các tính năng mở rộng phân quyền Admin
        void addEmployee();
        void removeEmployee();
        void manageSalaryAndShifts();
        void viewFeedbacks();
};



// ================================ LỚP ĐỐI TƯỢNG TABLE ================================
class Table {
    private:
        int tableID;
        int capacity;
        bool isBooked;
        Customer* bookedBy; // Con trỏ liên kết đến đối tượng khách đặt
        DateTime bookTime;
        std::vector<OrderItem> orderList;
    public:
        Table(int id = 0, int cap = 0);
        Table(const Table& other);
        Table& operator=(const Table& other);
        ~Table();
        int getTableID() const;
        int getCapacity() const;
        bool getStatus() const;
        Customer* getCustomer() const;
        DateTime getBookTime() const;
        const std::vector<OrderItem>& getOrderList() const;
        bool bookTable(Customer* c);
        bool bookTable(Customer* c, DateTime dt); // Nạp chồng hàm đặt bàn kèm thời gian
        void setOrderList(const std::vector<OrderItem>& orders);
        void freeTable();
        void displayTable() const;
};



// ================================ LỚP ĐIỀU PHỐI RESTAURANT MANAGER (SINGLETON) ================================
class RestaurantManager {
    private:
        int totalTables;
        Table* tables;                  // Mảng động chứa danh sách các bàn ăn
        int floorPlan[ROWS][COLS];      // Ma trận sơ đồ vị trí nhà hàng
        static RestaurantManager* instance; // Thực thể tĩnh duy nhất của Singleton

        // Đóng kín Constructor để ngăn chặn việc tạo đối tượng tự do bên ngoài
        RestaurantManager(int numTables);
    public:
        ~RestaurantManager();
        // Phương thức tĩnh duy nhất để lấy con trỏ truy cập thực thể bộ quản lý
        static RestaurantManager* getInstance(int numTables = 13);

        void displayFloorPlan();
        void displayAllTables();
        void addReservation();
        void addOnlineReservation();
        void deleteReservation();
        void checkoutTable();
        void customerDeleteReservation(); // Tính năng bảo mật hủy bàn của khách
        void editReservation();
        void exportReservationToFile(int tableID);
        void saveReservationsToFile(const std::string& filename);
        void loadReservationsFromFile(const std::string& filename);
};



// ================================ CÁC NGUYÊN MẪU HÀM GIAO DIỆN (INTERFACES) ================================
void sendFeedback();
void managerInterface(RestaurantManager* res, HRManager& hr);
void employeeInterface(RestaurantManager* res);
void customerInterface(RestaurantManager* res);
#endif // PBL_H
