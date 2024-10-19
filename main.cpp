constexpr int RECORDS_COUT = 4000;
constexpr int RECORDS_ON_PAGE = 20;
constexpr int PAGES_COUNT = RECORDS_COUT / RECORDS_ON_PAGE;

#include <iostream>
#include <cstring>
#include <format>
#include <string>
#include <algorithm>
#include <ncurses.h>
#include <cstdlib>

using namespace std;

struct record
{
    char author[12];
    char title[32];
    char publisher[16];
    short int year;
    short int num_of_page;
};

class Queue {
private:
    struct Node {
        record* data;
        Node* next;
    };
    Node* front;
    Node* rear;

public:
    Queue() : front(nullptr), rear(nullptr) {}

    ~Queue() {
        while (front) {
            Node* temp = front;
            front = front->next;
            delete temp;
        }
    }

    void enqueue(record* rec) {
        Node* new_node = new Node{ rec, nullptr };
        if (rear) {
            rear->next = new_node;
        }
        else {
            front = new_node;
        }
        rear = new_node;
    }

    bool dequeue(record*& rec) {
        if (!front) {
            return false;
        }
        rec = front->data;
        Node* temp = front;
        front = front->next;
        if (!front) {
            rear = nullptr;
        }
        delete temp;
        return true;
    }

    bool isEmpty() const {
        return front == nullptr;
    }
};

static void print_records_with_pagination(int current_page, int pages_count, int records_on_page, record** records);
static void quick_sort(record** a, int l, int r);
static void loop(record** indexes);
static void search(record** indexes);
static void binary_search(record** records, string& key, Queue& queue);
static char* get_surname(const char full_name[32]);

int main()
{
    FILE* fp;
    fp = fopen("testBase.dat", "rb");
    record records[RECORDS_COUT] = { 0 };

    int records_count = fread((record*)records, sizeof(record), RECORDS_COUT, fp);
    int pages_count = RECORDS_COUT / RECORDS_ON_PAGE;

    record** indexes = new record * [RECORDS_COUT];

    for (int i = 0; i < RECORDS_COUT; i++) {
        indexes[i] = &records[i];
    }

    initscr();
    loop(indexes);
    endwin();

    fclose(fp);
    return 0;
}

static void print_records_with_pagination(int current_page, int pages_count, int records_on_page, record** records) {
    clear();
    refresh();
    printw("Page: %d / %d\n", current_page + 1, pages_count);
    printw("%12s\t%30s\t%s\t%s\t%s\n", "Author", "Title", "Publisher", "Year", "Page");

    for (int i = current_page * records_on_page; (i < 4000) && (i < current_page * records_on_page + records_on_page); ++i)
        printw("%12s\t%30s\t%s\t%d\t%d\n",records[i]->author, records[i]->title, records[i]->publisher, records[i]->year, records[i]->num_of_page);
    
    int input_key = getch();

    if (input_key == 27) return;
    if ((input_key == 110 || input_key == 226) && current_page < pages_count - 1) current_page += 1;
    if ((input_key == 112 || input_key == 167) && current_page > 0) current_page -= 1;

    return print_records_with_pagination(current_page, pages_count, records_on_page, records);
}

static void quick_sort(record** a, int l, int r) {
    if (l >= r) return;

    char* key_element = get_surname(a[l]->title);

    int i = l, j = r;

    while (i <= j) {
        while (i <= r && strcmp(get_surname(a[i]->title), key_element) < 0) i++;
        while (j >= l && strcmp(get_surname(a[j]->title), key_element) > 0) j--;

        if (i <= j) {
            record* temp = a[i];
            a[i] = a[j];
            a[j] = temp;
            i++;
            j--;
        }
    }

    if (l < j) quick_sort(a, l, j);
    if (i < r) quick_sort(a, i, r);
}

static char* get_surname(const char full_name[32]) {
    const char* firstSpace = strchr(full_name, ' ');
    if (firstSpace == nullptr) return nullptr;

    const char* secondSpace = strchr(firstSpace + 1, ' ');
    if (secondSpace == nullptr) return nullptr;

    return (char*)(secondSpace + 1);
}

static void loop(record** indexes) {
    int input_key = 0;
    bool is_sorted = false;

    while (input_key != 27) {
        clear();
        refresh();
        printw("[I] - initial data | [S] - sorted data | [F] - search data\n");
        input_key = getch();

        if (input_key == 105 || input_key == 232) {
            print_records_with_pagination(0, PAGES_COUNT, RECORDS_ON_PAGE, indexes);
        }

        if (input_key == 115 || input_key == 235) {
            if (!is_sorted) {
                quick_sort(indexes, 0, RECORDS_COUT - 1);
                is_sorted = true;
            }
            print_records_with_pagination(0, PAGES_COUNT, RECORDS_ON_PAGE, indexes);
        }

        if (input_key == 102 || input_key == 160) {
            if (!is_sorted) {
                quick_sort(indexes, 0, RECORDS_COUT - 1);
                is_sorted = true;
            }
            search(indexes);
        }
    }
}

static void search(record** indexes) {
    char input[10];
    printw("Input key: ");
    getstr(input);
    printw("\n");
    string key(input);

    Queue queue;
    binary_search(indexes, key, queue);

    if (!queue.isEmpty()) {
        printw("Records:\n");
        record* rec;
        while (queue.dequeue(rec)) {
            printw("%12s\t%30s\t%s\t%d\t%d\n",rec->author, rec->title, rec->publisher, rec->year, rec->num_of_page);
        }

        getch();
    }
    else {
        printw("Error! Not found [%s]\n", key.c_str());
    }
    
    getch();
}

void binary_search(record** records, string& key, Queue& queue) {
    int left = 0;
    int right = RECORDS_COUT - 1;

    string lower_key = key.substr(0, 3);
    transform(lower_key.begin(), lower_key.end(), lower_key.begin(), ::tolower);

    int firstIndex = -1;

    while (left <= right) {
        int mid = left + (right - left) / 2;
        char* surname = get_surname(records[mid]->title);
        string lower_surname = (surname != nullptr) ? string(surname).substr(0, 3) : "";

        transform(lower_surname.begin(), lower_surname.end(), lower_surname.begin(), ::tolower);

        if (lower_surname == lower_key) {
            firstIndex = mid;
            right = mid - 1;
        }
        else if (lower_surname < lower_key) {
            left = mid + 1;
        }
        else {
            right = mid - 1;
        }
    }

    if (firstIndex == -1) return;

    int index = firstIndex;

    while (index < RECORDS_COUT) {
        char* surname = get_surname(records[index]->title);
        string lower_surname = (surname != nullptr) ? string(surname).substr(0, 3) : "";

        transform(lower_surname.begin(), lower_surname.end(), lower_surname.begin(), ::tolower);

        if (lower_surname == lower_key) {
            queue.enqueue(records[index]);
            index++;
        }
        else {
            break;
        }
    }
}
