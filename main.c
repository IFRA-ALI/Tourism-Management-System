 #include <windows.h>
#include <stdio.h>
#include <stdlib.h> 
#include <string.h>
#include <ctype.h>  

#define MAX_PACKAGES 500
#define LINE_BUF 256

// Control IDs
enum {
    ID_MAIN_USER = 100,
    ID_MAIN_ADMIN,
    ID_MAIN_EXIT,

    // Admin Login
    ID_LOGIN_ID,
    ID_LOGIN_PASS,
    ID_LOGIN_BTN,

    // Admin
    ID_ADMIN_LISTBOX,
    ID_ADMIN_ADD,
    ID_ADMIN_UPDATE,
    ID_ADMIN_DELETE,
    ID_ADMIN_SAVE,
    ID_ADMIN_VIEW_BOOKINGS,
    ID_ADMIN_VIEW_PACKAGES, 


    ID_ADMIN_DEST,
    ID_ADMIN_HOTEL,
    ID_ADMIN_VEH,
    ID_ADMIN_DAYS,
    ID_ADMIN_PRICE,

    // User
    ID_USER_LISTBOX,
    ID_USER_CUSTOMIZE,
    ID_USER_BOOK,
    ID_USER_REFRESH
};

typedef struct {
    char destination[30];
    char hotel_type[30];
    char vehicle_type[30];
    int days;
    float price;
} Package;

typedef struct {
    char name[64];
    char cnic[64];
    int persons;
    char date[64];
    int package_number;
    float bill;
} Booking;

static HWND hMainWnd = NULL;
static HWND hAdminWnd = NULL;
static HWND hUserWnd = NULL;

static Package packages[MAX_PACKAGES];
static int package_count = 0;
static int admin_viewing_bookings = 0; 

//Function Prototypes
LRESULT CALLBACK AdminWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK UserWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK BookingWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK CustomizeWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);


void trim_newline(char *s) {
    size_t n = strlen(s);
    while (n > 0 && (s[n-1] == '\n' || s[n-1] == '\r' || s[n-1] == ' ' || s[n-1] == '\t')) {
        s[n-1] = '\0';
        n--;
    }
}

int load_packages_from_file(const char *fname) {
    FILE *fp = fopen(fname, "r");
    if(!fp) return 0;
    char line[LINE_BUF];
    int cnt = 0;
    while(fgets(line, sizeof(line), fp) && cnt < MAX_PACKAGES) {
        trim_newline(line);
        if(sscanf(line, "%[^|]|%[^|]|%[^|]|%d|%f", packages[cnt].destination, packages[cnt].hotel_type, packages[cnt].vehicle_type, &packages[cnt].days, &packages[cnt].price) == 5) {
             cnt++;
        }
    }
    fclose(fp);
    package_count = cnt;
    return cnt;
}


int save_packages_to_file(const char *fname) {
    FILE *fp = fopen(fname, "w");
    if(!fp) return 0;
    for(int i=0;i<package_count;i++) {
        fprintf(fp, "%s|%s|%s|%d|%f\n", packages[i].destination, packages[i].hotel_type, packages[i].vehicle_type, packages[i].days, packages[i].price);
    }
    fclose(fp);
    return 1;
}


void EnablePackageControls(HWND hwnd, BOOL enable) {
    EnableWindow(GetDlgItem(hwnd, ID_ADMIN_ADD), enable);
    EnableWindow(GetDlgItem(hwnd, ID_ADMIN_UPDATE), enable);
    EnableWindow(GetDlgItem(hwnd, ID_ADMIN_DELETE), enable);
    EnableWindow(GetDlgItem(hwnd, ID_ADMIN_SAVE), enable);
    EnableWindow(GetDlgItem(hwnd, ID_ADMIN_DEST), enable);
    EnableWindow(GetDlgItem(hwnd, ID_ADMIN_HOTEL), enable);
    EnableWindow(GetDlgItem(hwnd, ID_ADMIN_VEH), enable);
    EnableWindow(GetDlgItem(hwnd, ID_ADMIN_DAYS), enable);
    EnableWindow(GetDlgItem(hwnd, ID_ADMIN_PRICE), enable);


    if (!enable) {
        SetWindowTextA(GetDlgItem(hwnd, ID_ADMIN_DEST), "");
        SetWindowTextA(GetDlgItem(hwnd, ID_ADMIN_HOTEL), "");
        SetWindowTextA(GetDlgItem(hwnd, ID_ADMIN_VEH), "");
        SetWindowTextA(GetDlgItem(hwnd, ID_ADMIN_DAYS), "");
        SetWindowTextA(GetDlgItem(hwnd, ID_ADMIN_PRICE), "");
    }
}



void populate_listbox_with_packages(HWND hList, HWND hParent) {
    SendMessage(hList, LB_RESETCONTENT, 0, 0);
    char buf[200];
    for(int i=0;i<package_count;i++) {
        snprintf(buf, sizeof(buf), " %d. Destination: %s |  Hotel: %s |  Transport:%s |  Days:%d |  Price:%.2f Rs", i+1, packages[i].destination, packages[i].hotel_type, packages[i].vehicle_type, packages[i].days, packages[i].price);
        SendMessage(hList, LB_ADDSTRING, 0, (LPARAM)buf);
    }
    admin_viewing_bookings = 0;
    SetWindowTextA(hParent, "Admin - Tourist System");
    EnablePackageControls(hParent, TRUE);
}


void populate_listbox_with_bookings(HWND hList, HWND hParent) {
    SendMessage(hList, LB_RESETCONTENT, 0, 0);
    FILE *fp = fopen("booking.txt", "r");
    if(!fp) {
        SendMessage(hList, LB_ADDSTRING, 0, (LPARAM)"Error: booking.txt not found or inaccessible.");
        admin_viewing_bookings = 1; // Still mark as viewing bookings attempt
        SetWindowTextA(hParent, "Admin - Tourist System (Bookings)");
        EnablePackageControls(hParent, FALSE);
        return;
    }
    
    char line[LINE_BUF];
    Booking b;
    int cnt = 0;
    char buf[256];

    while(fgets(line, sizeof(line), fp)) {
        trim_newline(line);
        if(sscanf(line, "%[^|]|%[^|]|%d|%[^|]|%d|%f", b.name, b.cnic, &b.persons, b.date, &b.package_number, &b.bill) == 6) {
             snprintf(buf, sizeof(buf), "%d. %s |  CNIC:%s |  Pkg#%d |  Date: %s |  Persons: %d |  Total:%.2f", cnt+1, b.name, b.cnic, b.package_number, b.date, b.persons, b.bill);
             SendMessage(hList, LB_ADDSTRING, 0, (LPARAM)buf);
             cnt++;
        }
    }
    fclose(fp);
    admin_viewing_bookings = 1;
    SetWindowTextA(hParent, "Admin - Tourist System (Bookings)");
    EnablePackageControls(hParent, FALSE);
}

HWND CreateLabel(HWND parent, const char *text, int x, int y, int w, int h, int id) {
    return CreateWindowA("STATIC", text, WS_CHILD | WS_VISIBLE, x, y, w, h, parent, (HMENU)id, GetModuleHandle(NULL), NULL);
}

HWND CreateEditBox(HWND parent, int x, int y, int w, int h, int id) {
    return CreateWindowA("EDIT", "", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL, x, y, w, h, parent, (HMENU)id, GetModuleHandle(NULL), NULL);
}

// --- LOGIN WINDOW
LRESULT CALLBACK LoginWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    static HWND hID, hPass;
    switch(msg) {
        case WM_CREATE:
            CreateLabel(hwnd, "Admin ID:", 10, 10, 70, 20, 0);
            hID = CreateEditBox(hwnd, 85, 10, 180, 22, ID_LOGIN_ID);
            
            CreateLabel(hwnd, "Password:", 10, 40, 70, 20, 0);

            hPass = CreateWindowA("EDIT", "", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_PASSWORD | ES_AUTOHSCROLL, 
                                  85, 40, 180, 22, hwnd, (HMENU)ID_LOGIN_PASS, GetModuleHandle(NULL), NULL);

            CreateWindowA("BUTTON", "Login", WS_CHILD | WS_VISIBLE, 100, 80, 80, 25, hwnd, (HMENU)ID_LOGIN_BTN, GetModuleHandle(NULL), NULL);
            break;

        case WM_COMMAND:
            if(LOWORD(wParam) == ID_LOGIN_BTN) {
                char entered_id[64], entered_pass[64];
                char id[64], pass[64];
                GetWindowTextA(hID, entered_id, sizeof(entered_id));
                GetWindowTextA(hPass, entered_pass, sizeof(entered_pass));
				
				FILE *fp=fopen("admin.txt", "r");
				if (!fp) {
    					MessageBoxA(hwnd, "admin.txt not found!", "Error", MB_OK | MB_ICONERROR);
    					break;
				}
    			fgets(id, sizeof(id), fp);
				fgets(pass, sizeof(pass), fp);
    			fclose(fp);
    			
    			id[strcspn(id, "\r\n")] = 0;
				pass[strcspn(pass, "\r\n")] = 0;
				for(int i=0; i<strlen(pass);i++){
					pass[i]=pass[i]^5;
				}

                if (strcmp(entered_id, id) == 0 && strcmp(entered_pass, pass)==0) {

                    DestroyWindow(hwnd);
                    
                    // Create the Admin Window
                    WNDCLASSA wc = {0};
                    wc.lpfnWndProc = AdminWndProc;
                    wc.hInstance = GetModuleHandle(NULL);
                    wc.lpszClassName = "AdminWindowClass";
                    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
                    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
                    RegisterClassA(&wc);
                    hAdminWnd = CreateWindowA("AdminWindowClass", "Admin - Tourist System (Packages)", WS_OVERLAPPEDWINDOW,
                                             80, 80, 640, 420, NULL, NULL, GetModuleHandle(NULL), NULL);
                    ShowWindow(hAdminWnd, SW_SHOW);
                } else {
                    MessageBoxA(hwnd, "Invalid ID or Password.", "Login Failed", MB_OK | MB_ICONERROR);
                }
            }
            break;

        case WM_CLOSE:
            DestroyWindow(hwnd);
            break;
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}


// Admin window 
LRESULT CALLBACK AdminWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    static HWND hList, hDest, hHotel, hVeh, hDays, hPrice;
    switch(msg) {
        case WM_CREATE:
            // Listbox
            hList = CreateWindowA("LISTBOX", NULL, WS_CHILD | WS_VISIBLE | WS_VSCROLL | LBS_NOTIFY | WS_BORDER,
                                     10, 10, 800, 200, hwnd, (HMENU)ID_ADMIN_LISTBOX, GetModuleHandle(NULL), NULL);
            // Labels and edits for Add/Update
            CreateLabel(hwnd, "Destination:", 10, 220, 80, 20, 0);
            hDest = CreateEditBox(hwnd, 95, 220, 200, 22, ID_ADMIN_DEST);
            CreateLabel(hwnd, "Hotel Type:", 10, 250, 80, 20, 0);
            hHotel = CreateEditBox(hwnd, 95, 250, 200, 22, ID_ADMIN_HOTEL);
            CreateLabel(hwnd, "Vehicle Type:", 10, 280, 80, 20, 0);
            hVeh = CreateEditBox(hwnd, 95, 280, 200, 22, ID_ADMIN_VEH);
            CreateLabel(hwnd, "Days:", 320, 220, 40, 20, 0);
            hDays = CreateEditBox(hwnd, 370, 220, 60, 22, ID_ADMIN_DAYS);
            CreateLabel(hwnd, "Price:", 320, 250, 40, 20, 0);
            hPrice = CreateEditBox(hwnd, 370, 250, 100, 22, ID_ADMIN_PRICE);

            // Buttons
            CreateWindowA("BUTTON", "Add Package", WS_CHILD | WS_VISIBLE, 480, 220, 120, 25, hwnd, (HMENU)ID_ADMIN_ADD, GetModuleHandle(NULL), NULL);
            CreateWindowA("BUTTON", "Update Selected", WS_CHILD | WS_VISIBLE, 480, 255, 120, 25, hwnd, (HMENU)ID_ADMIN_UPDATE, GetModuleHandle(NULL), NULL);
            CreateWindowA("BUTTON", "Delete Selected", WS_CHILD | WS_VISIBLE, 480, 290, 120, 25, hwnd, (HMENU)ID_ADMIN_DELETE, GetModuleHandle(NULL), NULL);
            CreateWindowA("BUTTON", "Save Packages", WS_CHILD | WS_VISIBLE, 480, 325, 120, 25, hwnd, (HMENU)ID_ADMIN_SAVE, GetModuleHandle(NULL), NULL);
            CreateWindowA("BUTTON", "View Bookings", WS_CHILD | WS_VISIBLE, 10, 360, 120, 25, hwnd, (HMENU)ID_ADMIN_VIEW_BOOKINGS, GetModuleHandle(NULL), NULL);
            CreateWindowA("BUTTON", "View Packages", WS_CHILD | WS_VISIBLE, 10, 395, 120, 25, hwnd, (HMENU)ID_ADMIN_VIEW_PACKAGES, GetModuleHandle(NULL), NULL);


            load_packages_from_file("packages.txt");
            populate_listbox_with_packages(hList, hwnd);
            break;

        case WM_COMMAND:
            if(LOWORD(wParam) == ID_ADMIN_VIEW_PACKAGES) {
                load_packages_from_file("packages.txt");
                populate_listbox_with_packages(hList, hwnd);
            } else if(LOWORD(wParam) == ID_ADMIN_VIEW_BOOKINGS) {
                populate_listbox_with_bookings(hList, hwnd);
            } else if(LOWORD(wParam) == ID_ADMIN_ADD) {
                if(admin_viewing_bookings) {
                    MessageBoxA(hwnd, "Switch back to 'Packages' view to add packages.", "Info", MB_OK);
                    break;
                }
                char dest[64], hotel[64], veh[64], days_s[16], price_s[32];
                GetWindowTextA(hDest, dest, sizeof(dest));
                GetWindowTextA(hHotel, hotel, sizeof(hotel));
                GetWindowTextA(hVeh, veh, sizeof(veh));
                GetWindowTextA(hDays, days_s, sizeof(days_s));
                GetWindowTextA(hPrice, price_s, sizeof(price_s));

                if(strlen(dest)==0 || strlen(days_s)==0 || strlen(price_s)==0) {
                    MessageBoxA(hwnd, "Destination, Days, and Price required", "Error", MB_OK | MB_ICONERROR);
                    break;
                }
                if(package_count < MAX_PACKAGES) {
                    strncpy(packages[package_count].destination, dest, sizeof(packages[0].destination)-1);
                    strncpy(packages[package_count].hotel_type, hotel, sizeof(packages[0].hotel_type)-1);
                    strncpy(packages[package_count].vehicle_type, veh, sizeof(packages[0].vehicle_type)-1);
                    packages[package_count].days = atoi(days_s);
                    packages[package_count].price = (float) atof(price_s);
                    package_count++;
                    populate_listbox_with_packages(hList, hwnd);
                    MessageBoxA(hwnd, "Package added (Click Save Packages to persist).", "Info", MB_OK);
                } else {
                    MessageBoxA(hwnd, "Package limit reached", "Error", MB_OK | MB_ICONERROR);
                }
            } else if(LOWORD(wParam) == ID_ADMIN_UPDATE) {
                if(admin_viewing_bookings) {
                    MessageBoxA(hwnd, "Update is only for packages. Switch back to 'Packages' view.", "Info", MB_OK);
                    break;
                }
                int sel = (int) SendMessage(hList, LB_GETCURSEL, 0, 0);
                if(sel == LB_ERR) {
                    MessageBoxA(hwnd, "Select a package to update", "Error", MB_OK | MB_ICONERROR);
                    break;
                }
                char dest[64], hotel[64], veh[64], days_s[16], price_s[32];
                GetWindowTextA(hDest, dest, sizeof(dest));
                GetWindowTextA(hHotel, hotel, sizeof(hotel));
                GetWindowTextA(hVeh, veh, sizeof(veh));
                GetWindowTextA(hDays, days_s, sizeof(days_s));
                GetWindowTextA(hPrice, price_s, sizeof(price_s));

                if(strlen(dest)==0 || strlen(days_s)==0 || strlen(price_s)==0) {
                    MessageBoxA(hwnd, "Destination, Days, and Price required", "Error", MB_OK | MB_ICONERROR);
                    break;
                }

                strncpy(packages[sel].destination, dest, sizeof(packages[0].destination)-1);
                strncpy(packages[sel].hotel_type, hotel, sizeof(packages[0].hotel_type)-1);
                strncpy(packages[sel].vehicle_type, veh, sizeof(packages[0].vehicle_type)-1);
                packages[sel].days = atoi(days_s);
                packages[sel].price = (float) atof(price_s);
                populate_listbox_with_packages(hList, hwnd);
                MessageBoxA(hwnd, "Package updated (Click Save Packages to persist).", "Info", MB_OK);
            } else if(LOWORD(wParam) == ID_ADMIN_DELETE) {
                if(admin_viewing_bookings) {
                    MessageBoxA(hwnd, "Delete is only for packages. Switch back to 'Packages' view.", "Info", MB_OK);
                    break;
                }
                int sel = (int) SendMessage(hList, LB_GETCURSEL, 0, 0);
                if(sel == LB_ERR) {
                    MessageBoxA(hwnd, "Select a package to delete", "Error", MB_OK | MB_ICONERROR);
                    break;
                }

                for(int i=sel;i<package_count-1;i++) packages[i]=packages[i+1];
                package_count--;
                populate_listbox_with_packages(hList, hwnd);
                MessageBoxA(hwnd, "Package removed.", "Info", MB_OK);
            } else if(LOWORD(wParam) == ID_ADMIN_SAVE) {
                if(admin_viewing_bookings) {
                    MessageBoxA(hwnd, "Cannot save while viewing bookings.", "Error", MB_OK | MB_ICONERROR);
                    break;
                }
                if(save_packages_to_file("packages.txt")) {
                    MessageBoxA(hwnd, "Packages saved to packages.txt", "Info", MB_OK);
                } else {
                    MessageBoxA(hwnd, "Failed to save packages.txt", "Error", MB_OK | MB_ICONERROR);
                }
            } else if(LOWORD(wParam) == ID_ADMIN_LISTBOX && HIWORD(wParam) == LBN_SELCHANGE) {
                if(admin_viewing_bookings) {
                    break;
                }
                int sel = (int) SendMessage(hList, LB_GETCURSEL, 0, 0);
                if(sel != LB_ERR) {
   
                    SetWindowTextA(hDest, packages[sel].destination);
                    SetWindowTextA(hHotel, packages[sel].hotel_type);
                    SetWindowTextA(hVeh, packages[sel].vehicle_type);
                    char buf[32];
                    snprintf(buf, sizeof(buf), "%d", packages[sel].days);
                    SetWindowTextA(hDays, buf);
                    snprintf(buf, sizeof(buf), "%.2f", packages[sel].price);
                    SetWindowTextA(hPrice, buf);
                }
            }
            break;

        case WM_CLOSE:
            DestroyWindow(hwnd);
            hAdminWnd = NULL;
            break;
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}


void save_booking_to_file(const char *name, const char *cnic, int persons, const char *date, int package_number, float bill) {
    FILE *fp = fopen("booking.txt", "a");
    if(!fp) return;
    fprintf(fp, "%s|%s|%d|%s|%d|%f\n", name, cnic, persons, date, package_number, bill);
    fclose(fp);
}

// Booking
LRESULT CALLBACK BookingWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    static int pkg_index;
    static HWND heName, heCnic, hePersons, heDate, hbtnConfirm;
    switch(msg) {
        case WM_CREATE:
        {
            pkg_index = (int)(intptr_t)GetWindowLongPtr(hwnd, GWLP_USERDATA);
            CreateLabel(hwnd, "Name:", 10, 10, 60, 20, 0);
            heName = CreateEditBox(hwnd, 80, 10, 220, 22, 0);
            CreateLabel(hwnd, "CNIC:", 10, 40, 60, 20, 0);
            heCnic = CreateEditBox(hwnd, 80, 40, 220, 22, 0);
            CreateLabel(hwnd, "Persons:", 10, 70, 60, 20, 0);
            hePersons = CreateEditBox(hwnd, 80, 70, 80, 22, 0);
            CreateLabel(hwnd, "Date:", 10, 100, 110, 20, 0);
            heDate = CreateEditBox(hwnd, 130, 100, 170, 22, 0);
            hbtnConfirm = CreateWindowA("BUTTON", "Confirm Booking", WS_CHILD | WS_VISIBLE, 80, 140, 150, 30, hwnd, (HMENU)1, GetModuleHandle(NULL), NULL);
            break;
        }
        case WM_COMMAND:
            if(LOWORD(wParam) == 1) {
                char name[64], cnic[64], persons_s[16], date[64];
                GetWindowTextA(heName, name, sizeof(name));
                GetWindowTextA(heCnic, cnic, sizeof(cnic));
                GetWindowTextA(hePersons, persons_s, sizeof(persons_s));
                GetWindowTextA(heDate, date, sizeof(date));
                if(strlen(name)==0 || strlen(cnic)==0 || strlen(persons_s)==0 || strlen(date)==0) {
                    MessageBoxA(hwnd, "All fields required", "Error", MB_OK | MB_ICONERROR);
                    break;
                }
                if (strlen(cnic) != 13) {
    				MessageBoxA(hwnd, "CNIC must be exactly 13 digits", "Error", MB_OK | MB_ICONERROR);
   					break;
				}
				for (int i = 0; i < 13; i++) {
    				if (!isdigit((unsigned char)cnic[i])) {
        				MessageBoxA(hwnd, "CNIC must contain only digits", "Error", MB_OK | MB_ICONERROR);
       					break;
   					 }
   			}

                int persons = atoi(persons_s);

                if (persons <= 0) { MessageBoxA(hwnd, "Persons must be a positive number.", "Error", MB_OK | MB_ICONERROR); break; }

                float bill = packages[pkg_index].price * persons;
                save_booking_to_file(name, cnic, persons, date, pkg_index+1, bill);

                char receipt[512];
                snprintf(receipt, sizeof(receipt),
                             "Booking Successful!\n----------------\nName: %s\nCNIC: %s\nPackage Number: %d\nTravelling Date: %s\nPersons: %d\nTotal: %.2f Rs",
                             name, cnic, pkg_index+1, date, persons, bill);
                MessageBoxA(hwnd, receipt, "Receipt", MB_OK);
                DestroyWindow(hwnd);
            }
            break;
        case WM_CLOSE:
            DestroyWindow(hwnd);
            break;
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

// Customize window
LRESULT CALLBACK CustomizeWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    static HWND heDest, heType, heDays, heDist, hbtnConfirm;
    switch(msg) {
        case WM_CREATE:
            SetWindowTextA(hwnd, "Customize Package");
            CreateLabel(hwnd, "Destination:", 10, 10, 80, 20, 0);
            heDest = CreateEditBox(hwnd, 120, 10, 180, 22, 0);
            CreateLabel(hwnd, "Package Type (1-3):", 10, 40, 100, 20, 0);
            heType = CreateEditBox(hwnd, 120, 40, 80, 22, 0);
            CreateLabel(hwnd, "Days:", 10, 70, 80, 20, 0);
            heDays = CreateEditBox(hwnd, 120, 70, 80, 22, 0);
            CreateLabel(hwnd, "Distance (km):", 10, 100, 80, 20, 0);
            heDist = CreateEditBox(hwnd, 120, 100, 80, 22, 0);
            hbtnConfirm = CreateWindowA("BUTTON", "Create Custom", WS_CHILD | WS_VISIBLE, 80, 140, 150, 30, hwnd, (HMENU)1, GetModuleHandle(NULL), NULL);
            break;
        
        case WM_COMMAND:
            if(LOWORD(wParam) == 1) {
                char dest[64], type_s[16], days_s[16], dist_s[16];
                GetWindowTextA(heDest, dest, sizeof(dest));
                GetWindowTextA(heType, type_s, sizeof(type_s));
                GetWindowTextA(heDays, days_s, sizeof(days_s));
                GetWindowTextA(heDist, dist_s, sizeof(dist_s));

                if(strlen(dest)==0 || strlen(type_s)==0 || strlen(days_s)==0 || strlen(dist_s)==0) {
                    MessageBoxA(hwnd, "All fields required", "Error", MB_OK | MB_ICONERROR);
                    break;
                }
                
                int ptype = atoi(type_s);
                int days = atoi(days_s);
                float distance = (float) atof(dist_s);
                float price = 0.0f;
                char hotel[20] = "", vehicle[20] = "";

                if(ptype < 1 || ptype > 3 || days <= 0 || distance <= 0) {
                    MessageBoxA(hwnd, "Invalid input: Type must be 1-3, Days/Distance must be positive.", "Validation Error", MB_OK | MB_ICONERROR);
                    break;
                }

                switch (ptype) {
                    case 1: price = 2500.0f * distance + (float)days * 1000.0f; strcpy(hotel, "First Class"); strcpy(vehicle, "First Class"); break;
                    case 2: price = 2000.0f * distance + (float)days * 800.0f; strcpy(hotel, "Business"); strcpy(vehicle, "Business"); break;
                    case 3: price = 1500.0f * distance + (float)days * 600.0f; strcpy(hotel, "Economy"); strcpy(vehicle, "Economy"); break;
                }
                

                if(package_count < MAX_PACKAGES) {
                    strncpy(packages[package_count].destination, dest, sizeof(packages[0].destination)-1);
                    strncpy(packages[package_count].hotel_type, hotel, sizeof(packages[0].hotel_type)-1);
                    strncpy(packages[package_count].vehicle_type, vehicle, sizeof(packages[0].vehicle_type)-1);
                    packages[package_count].days = days;
                    packages[package_count].price = price;
                    package_count++;
                }
                

                FILE *fp = fopen("packages.txt", "a");
                if(fp) {
                    fprintf(fp, "%s|%s|%s|%d|%f\n", dest, hotel, vehicle, days, price);
                    fclose(fp);
                } else {
                     MessageBoxA(hwnd, "Failed to write to packages.txt. Custom package not saved.", "File Error", MB_OK | MB_ICONERROR);
                }

                char info[256];
                snprintf(info, sizeof(info), "Custom Package Created!\nPrice per person: %.2f", price);
                MessageBoxA(hwnd, info, "Success", MB_OK);
                DestroyWindow(hwnd);
            }
            break;
        case WM_CLOSE:
            DestroyWindow(hwnd);
            break;
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}


// User window
LRESULT CALLBACK UserWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    static HWND hList;
    switch(msg) {
        case WM_CREATE:
            hList = CreateWindowA("LISTBOX", NULL, WS_CHILD | WS_VISIBLE | WS_VSCROLL | LBS_NOTIFY | WS_BORDER,
                                     10, 10, 800, 200, hwnd, (HMENU)ID_USER_LISTBOX, GetModuleHandle(NULL), NULL);
            CreateWindowA("BUTTON", "Book Selected", WS_CHILD | WS_VISIBLE, 10, 220, 120, 28, hwnd, (HMENU)ID_USER_BOOK, GetModuleHandle(NULL), NULL);
            CreateWindowA("BUTTON", "Refresh", WS_CHILD | WS_VISIBLE, 140, 220, 80, 28, hwnd, (HMENU)ID_USER_REFRESH, GetModuleHandle(NULL), NULL);
            CreateWindowA("BUTTON", "Customize", WS_CHILD | WS_VISIBLE, 230, 220, 100, 28, hwnd, (HMENU)ID_USER_CUSTOMIZE, GetModuleHandle(NULL), NULL);
            
            load_packages_from_file("packages.txt");
            populate_listbox_with_packages(hList, hwnd);
            break;
        case WM_COMMAND:
            if(LOWORD(wParam) == ID_USER_REFRESH) {
                load_packages_from_file("packages.txt");
                populate_listbox_with_packages(hList, hwnd);
            } else if(LOWORD(wParam) == ID_USER_CUSTOMIZE) {
                static int customizeClassRegistered = 0;
                if(!customizeClassRegistered) {
                    WNDCLASSA wc = {0};
                    wc.lpfnWndProc = CustomizeWndProc;
                    wc.hInstance = GetModuleHandle(NULL);
                    wc.lpszClassName = "CustomizeWndClass";
                    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
                    wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE+1);
                    RegisterClassA(&wc);
                    customizeClassRegistered = 1;
                }
                HWND hPopup = CreateWindowA("CustomizeWndClass", "Customize Package", WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU,
                                                 300, 200, 320, 220, hwnd, NULL, GetModuleHandle(NULL), NULL);
                ShowWindow(hPopup, SW_SHOW);

            } else if(LOWORD(wParam) == ID_USER_BOOK) {
                int sel = (int) SendMessage(hList, LB_GETCURSEL, 0, 0);
                if(sel == LB_ERR) {
                    MessageBoxA(hwnd, "Select a package to book", "Error", MB_OK | MB_ICONERROR);
                    break;
                }
                // Register booking window class if not already
                static int bookedClassRegistered = 0;
                if(!bookedClassRegistered) {
                    WNDCLASSA wc = {0};
                    wc.lpfnWndProc = BookingWndProc;
                    wc.hInstance = GetModuleHandle(NULL);
                    wc.lpszClassName = "BookingWndClass";
                    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
                    wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE+1);
                    RegisterClassA(&wc);
                    bookedClassRegistered = 1;
                }
                // Create the booking dialog window
                HWND hPopup = CreateWindowA("BookingWndClass", "Book Package", WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU,
                                                 300, 200, 320, 220, hwnd, NULL, GetModuleHandle(NULL), NULL);

                SetWindowLongPtr(hPopup, GWLP_USERDATA, (LONG_PTR)sel);
                ShowWindow(hPopup, SW_SHOW);
            }
            break;
        case WM_CLOSE:
            DestroyWindow(hwnd);
            hUserWnd = NULL;
            break;
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

// Main window 
LRESULT CALLBACK MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch(msg) {
        case WM_CREATE:
            CreateWindowA("BUTTON", "User", WS_CHILD | WS_VISIBLE, 50, 40, 100, 40, hwnd, (HMENU)ID_MAIN_USER, GetModuleHandle(NULL), NULL);
            CreateWindowA("BUTTON", "Admin", WS_CHILD | WS_VISIBLE, 200, 40, 100, 40, hwnd, (HMENU)ID_MAIN_ADMIN, GetModuleHandle(NULL), NULL);
            CreateWindowA("BUTTON", "Exit", WS_CHILD | WS_VISIBLE, 350, 40, 100, 40, hwnd, (HMENU)ID_MAIN_EXIT, GetModuleHandle(NULL), NULL);
            break;
        case WM_COMMAND:
            if(LOWORD(wParam) == ID_MAIN_USER) {
                if(hUserWnd) {
                    SetForegroundWindow(hUserWnd);
                    break;
                }
                // Register and create user window
                WNDCLASSA wc = {0};
                wc.lpfnWndProc = UserWndProc;
                wc.hInstance = GetModuleHandle(NULL);
                wc.lpszClassName = "UserWindowClass";
                wc.hCursor = LoadCursor(NULL, IDC_ARROW);
                wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
                RegisterClassA(&wc);
                hUserWnd = CreateWindowA("UserWindowClass", "User - Tourist System", WS_OVERLAPPEDWINDOW,
                                         100, 100, 560, 320, NULL, NULL, GetModuleHandle(NULL), NULL);
                ShowWindow(hUserWnd, SW_SHOW);
            } else if(LOWORD(wParam) == ID_MAIN_ADMIN) {
                if(hAdminWnd) { 
                    SetForegroundWindow(hAdminWnd); 
                    break; 
                }

                // LOGIN LOGIC
                static int loginClassRegistered = 0;
                if(!loginClassRegistered) {
                    WNDCLASSA wc = {0};
                    wc.lpfnWndProc = LoginWndProc;
                    wc.hInstance = GetModuleHandle(NULL);
                    wc.lpszClassName = "LoginWindowClass";
                    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
                    wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE+1);
                    RegisterClassA(&wc);
                    loginClassRegistered = 1;
                }

                HWND hLoginPopup = CreateWindowA("LoginWindowClass", "Admin Login", WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU,
                                 400, 250, 300, 150, hwnd, NULL, GetModuleHandle(NULL), NULL);
                ShowWindow(hLoginPopup, SW_SHOW);

            } else if(LOWORD(wParam) == ID_MAIN_EXIT) {
                PostQuitMessage(0);
            }
            break;
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrev, LPSTR lpCmdLine, int nShowCmd) {

    WNDCLASSA wc = {0};
    wc.lpfnWndProc = MainWndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "MainWindowClass";
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
    if(!RegisterClassA(&wc)) {
        MessageBoxA(NULL, "Failed to register window class", "Error", MB_OK | MB_ICONERROR);
        return 0;
    }

    hMainWnd = CreateWindowA("MainWindowClass", "Tourist Management System", WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU,
                             300, 200, 500, 150, NULL, NULL, hInstance, NULL);
    if(!hMainWnd) {
        MessageBoxA(NULL, "Failed to create main window", "Error", MB_OK | MB_ICONERROR);
        return 0;
    }

    ShowWindow(hMainWnd, SW_SHOW);
    UpdateWindow(hMainWnd);

    MSG msg;
    while(GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return (int)msg.wParam;
}
