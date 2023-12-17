#include <systemc.h>
#include <iostream>
#include <cstring>
#include <iomanip>
#include <cstdlib>
#include <ctime>


using namespace sc_core;
using namespace std;



// lưu giá trị của các sensors
class CarSensor {
private:
    char phim;
    char func;
    int money;
    bool ID;

public:
    CarSensor() : phim('\0'), money(0), ID(false) {}

    CarSensor(char phim, int money, bool ID, char func) {
        this->phim = phim;
        this->func = func;
        this->money = money;
        this->ID = ID;
    }

    void set_phim(char phim) {
        this->phim = phim;
    }
    void set_func(char func) {
        this->func = func;
    }

    void set_money(int money) {
        this->money = money;
    }

    void set_ID(bool ID) {
        this->ID = ID;
    }
	bool generateID() {
        // Tạo số nguyên ngẫu nhiên trong khoảng [0, RAND_MAX]
        int randomInt = std::rand();

        // Ngưỡng ngẫu nhiên để quyết định giá trị bool
        return (randomInt > RAND_MAX / 2);
    }

    int generateMoney() {
        // Tính toán giá trị ngẫu nhiên trong khoảng [0, 10,000,000]
        int randomInt = (std::rand() % 10) *100000;

        return randomInt;
    }

    char generatePhim() {
        // Tạo số nguyên ngẫu nhiên trong khoảng [0, 255]
        int randomInt;
        do {
            randomInt = std::rand() % 256;
        } while (!(randomInt >= 48 && randomInt <= 57));

        // Ép kiểu số nguyên thành kiểu char
        return static_cast<char>(randomInt);
    }
    char generatefunc() {
        // Tạo số nguyên ngẫu nhiên trong khoảng [0, 255]
        int randomInt;
        do {
            randomInt = std::rand() % 256;
        } while (randomInt != 35 && randomInt != 42);

        // Ép kiểu số nguyên thành kiểu char
        return static_cast<char>(randomInt);
    }
    char get_phim() {
        this->phim = generatePhim();
        return this->phim;
    }
    char get_func() {
        this->func = generatefunc();
        return this->func;
    }
    int get_money() {
        this->money = generateMoney();
        return this->money;
    }

    bool get_ID() {
        this->ID = generateID();
        return this->ID;
    }
};

SC_MODULE (ESC_System) {
// Các khai báo các thành phần của module
  sc_in<char> phim;
  sc_in<char> func;
  sc_in<int> money;
  sc_in<bool> ID;

  CarSensor c;
// Phương thức xử lý sự kiện
  void control_logic() {
    int S1 = 0, S2 = 0, S3 = 0, d;
    bool id;
    int MONEY;
    char p1,p2,p3;
    for(int i = 0 ; i <= 20 ; i++ ) {
        id = c.get_ID();
        MONEY = c.get_money();
        //cout <<"T: " << sc_time_stamp() << " ID: " << c.get_ID() << " M: " << c.get_money() << " phim: " << c.get_phim() << endl;
        if(id)
        {
            S1 = 1;
            cout << "Da quet the: " << id << endl;
        }
        else{
            cout << "Da quet the: " << id << endl;
            cout << "Trang thai cho" << endl;
            continue;
        }
        p1 = c.get_phim();
        if(p1 >= '1' && p1 <= '9' && S1)
        {
            S2 = 1;
            d = static_cast<int>(p1 - '0');
            S1 = 0;
            cout << "Da chon diem den: " << p1 << endl;
        }
        else
        {
            cout << "Da chon diem den: " << p1 << endl;
            cout << "Trang thai cho" << endl;
            continue;
        }
        p2 = c.get_func();
        if(S2 && p2 == '#')
        {
            S3 = 1;
            cout << "Dong y thanh toan: " << p2 << endl;
            S2 = 0;
        }
        else
        {
            cout << "Dong y thanh toan: " << p2 << endl;
            cout << "Trang thai cho" << endl;
            continue;
        }
        p3 = c.get_func();
        if(S3 && MONEY >= d*100000 && p3 == '#')
        {
            cout << "Thanh Toan Thanh Cong: " << MONEY << endl;
            cout << "Len Tau" << endl;
            S3 = 0;
        }        
        else if(S3 && p3 == '*')
        {
            cout << "Huy Thanh Toan: " << p3 << endl;
            cout << "Trang thai cho" << endl;
            continue;
        }
        else if(S3 && MONEY < d*100000 && p3 == '#')
        {
            S3 = 1;
            cout << "Thuc hien lai: " << MONEY << endl;
            cout << "Trang thai cho" << endl;
        }
        cout << endl << endl;
  }
  }

// Constructor của module
  SC_CTOR (ESC_System) {
    // Đăng ký các quá trình hoặc các phương thức xử lý sự kiện
    SC_THREAD (control_logic);
    sensitive << ID << phim << money;
  }
};

int sc_main(int argc, char* argv[]) {
  sc_signal<char> phim;
    sc_signal<char> func;
  sc_signal<int> money;
  sc_signal<bool> ID;
  // Tạo một instance của module
  ESC_System esc_system("ESC_System");
  esc_system.phim(phim);
    esc_system.func(func);
  esc_system.money(money);
  esc_system.ID(ID);
  
  cout << "\n \t HE THONG ESC O TO \n\n"
         << endl;
  
 // Chạy mô phỏng SystemC
  sc_start();

  return 0;

}

