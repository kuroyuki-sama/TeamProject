// F4. 공부관리 리마인더 (담당: 황현준)
// 기능: 공부/시험 일정 등록 / 날짜순 목록 + D-day / 완료 처리 / 삭제

#define _CRT_SECURE_NO_WARNINGS   // Visual Studio 경고 끄기
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "study.h"

// 매크로
#define STUDY_FILE      "data/study.dat"
#define MAX_ITEM        100
#define SUBJ_LEN        30
#define TITLE_LEN       50
#define SECONDS_PER_DAY (24 * 60 * 60)

#define TYPE_ALL   (-1)   // 전체 보기용
#define TYPE_STUDY 0      // 공부 일정
#define TYPE_EXAM  1      // 시험 일정

// 두 값을 서로 바꾸는 매크로 (정렬에 사용)
#define SWAP(type, a, b)  do { type tmp = (a); (a) = (b); (b) = tmp; } while (0)

// 디버그용 출력 (파일 맨 위에 #define DEBUG 를 쓰면 켜짐)
#ifdef DEBUG
#define DEBUG_LOG(msg)  printf("[DEBUG] %s\n", msg)
#else
#define DEBUG_LOG(msg)
#endif

// 구조체
typedef struct {
    int  id;                // 일정 번호
    int  type;              // 0: 공부, 1: 시험
    char subject[SUBJ_LEN]; // 과목
    char title[TITLE_LEN];  // 내용
    int  date;              // 날짜 (예: 20261210)
    int  done;              // 0: 미완료, 1: 완료
} Item;

static Item *items;    // 동적 할당으로 만들 배열
static int count = 0;  // 등록된 일정 개수

// 도우미 함수

// 숫자 입력받기 (글자를 입력하면 다시 입력받음)
static int input_number(const char *message)
{
    int n;
    printf("%s", message);
    while (scanf("%d", &n) != 1) {
        while (getchar() != '\n');
        printf("숫자로 입력하세요: ");
    }
    return n;
}

// 날짜 입력받기 (YYYYMMDD) <- 이런식으로
static int input_date(void)
{
    int date, y, m, d;
    while (1) {
        date = input_number("날짜 (예: 20261210): ");
        y = date / 10000;
        m = (date / 100) % 100;
        d = date % 100;
        if (y >= 2000 && y <= 2100 && m >= 1 && m <= 12 && d >= 1 && d <= 31) {
            return date;
        }
        printf("날짜가 올바르지 않습니다.\n");
    }
}

// YYYYMMDD 숫자를 시간(time_t)으로 바꿈 (낮 12시 기준)
static time_t date_to_time(int date)
{
    struct tm t = {0};
    t.tm_year = date / 10000 - 1900;
    t.tm_mon = (date / 100) % 100 - 1;
    t.tm_mday = date % 100;
    t.tm_hour = 12;
    return mktime(&t);
}

// 오늘부터 date 까지 남은 일수 (지났으면 음수로 바꿔서)
static int days_left(int date)
{
    time_t now = time(NULL);
    struct tm *tm = localtime(&now);
    int today = (tm->tm_year + 1900) * 10000 + (tm->tm_mon + 1) * 100 + tm->tm_mday;
    double diff = difftime(date_to_time(date), date_to_time(today));

    // 반올림해서 일 단위로 바꿈
    return (int)(diff / SECONDS_PER_DAY + (diff >= 0 ? 0.5 : -0.5));
}

//  파일 입출력

// 배열의 내용을 파일에 저장
static void save_items(void)
{
    int i;
    FILE *fp = fopen(STUDY_FILE, "w");
    if (fp == NULL) {
        printf("파일 저장 실패! data 폴더가 있는지 확인하세요.\n");
        return;
    }
    DEBUG_LOG("save_items");
    for (i = 0; i < count; i++) {
        fprintf(fp, "%d %d %s %s %d %d\n", items[i].id, items[i].type,
                items[i].subject, items[i].title, items[i].date, items[i].done);
    }
    fclose(fp);
}

// 파일에서 읽어서 배열에 저장
static void load_items(void)
{
    FILE *fp = fopen(STUDY_FILE, "r");
    DEBUG_LOG("load_items");
    if (fp == NULL) {
        return;   // 처음 실행하면 파일이 없으므로 그냥 끝냄
    }
    while (count < MAX_ITEM &&
           fscanf(fp, "%d %d %s %s %d %d", &items[count].id, &items[count].type,
                  items[count].subject, items[count].title,
                  &items[count].date, &items[count].done) == 6) {
        count++;
    }
    fclose(fp);
}

// 기능 함수

// 날짜 빠른 순으로 정렬 (버블 정렬)
static void sort_by_date(void)
{
    int i, j;
    for (i = 0; i < count - 1; i++) {
        for (j = 0; j < count - 1 - i; j++) {
            if (items[j].date > items[j + 1].date) {
                SWAP(Item, items[j], items[j + 1]);
            }
        }
    }
}

// 일정 목록 출력 (type: TYPE_ALL, TYPE_STUDY, TYPE_EXAM)
static void show_items(int type)
{
    int i, left, shown = 0;
    char dday[20];

    sort_by_date();
    printf("\n번호  구분  D-day     날짜        과목          내용\n");
    printf("--------------------------------------------------------\n");
    for (i = 0; i < count; i++) {
        if (type != TYPE_ALL && items[i].type != type) {
            continue;   // 원하는 종류가 아니면 건너뜀
        }
        left = days_left(items[i].date);
        if (items[i].done)  strcpy(dday, "완료");
        else if (left == 0) strcpy(dday, "D-Day");
        else if (left > 0)  sprintf(dday, "D-%d", left);
        else                sprintf(dday, "D+%d", -left);

        printf("%-4d  %-4s  %-8s  %-10d  %-12s  %s\n", items[i].id,
               items[i].type == TYPE_EXAM ? "시험" : "공부",
               dday, items[i].date, items[i].subject, items[i].title);
        shown++;
    }
    if (shown == 0) {
        printf("등록된 일정이 없습니다.\n");
    }
}

// 일정 등록 (type: TYPE_STUDY 또는 TYPE_EXAM)
static void add_item(int type)
{
    int i, maxId = 0;

    if (count >= MAX_ITEM) {
        printf("더 이상 등록할 수 없습니다.\n");
        return;
    }
    // 가장 큰 번호 + 1 을 새 번호로 사용
    for (i = 0; i < count; i++) {
        if (items[i].id > maxId) maxId = items[i].id;
    }
    items[count].id = maxId + 1;
    items[count].type = type;
    items[count].done = 0;

    printf("과목(띄어쓰기 없이): ");
    scanf("%s", items[count].subject);
    printf("내용(띄어쓰기 없이): ");
    scanf("%s", items[count].title);
    items[count].date = input_date();
    count++;

    save_items();
    printf("등록되었습니다.\n");
}

// 완료 처리
static void complete_item(void)
{
    int i, id;
    Item *p = NULL;   // 포인터로 해당 일정을 가리킴

    if (count == 0) {
        printf("등록된 일정이 없습니다.\n");
        return;
    }
    show_items(TYPE_ALL);
    id = input_number("완료할 일정 번호: ");

    for (i = 0; i < count; i++) {
        if (items[i].id == id) {
            p = &items[i];
        }
    }
    if (p == NULL) {
        printf("없는 번호입니다.\n");
        return;
    }
    p->done = 1;
    save_items();
    printf("완료 처리되었습니다.\n");
}

// 일정 삭제
static void delete_item(void)
{
    int i, j, id;

    if (count == 0) {
        printf("등록된 일정이 없습니다.\n");
        return;
    }
    show_items(TYPE_ALL);
    id = input_number("삭제할 일정 번호: ");

    for (i = 0; i < count; i++) {
        if (items[i].id == id) {
            for (j = i; j < count - 1; j++) {
                items[j] = items[j + 1];   // 뒤의 것을 앞으로 당김
            }
            count--;
            save_items();
            printf("삭제되었습니다.\n");
            return;
        }
    }
    printf("없는 번호입니다.\n");
}

// 메뉴
void study_menu(void)
{
    int sel;

    items = (Item *)malloc(sizeof(Item) * MAX_ITEM);   // 동적 메모리 할당
    if (items == NULL) {
        printf("메모리 할당 실패\n");
        return;
    }
    count = 0;
    load_items();

    do {
        printf("\n===== 공부관리 리마인더 =====\n");
        printf("1. 공부 일정 등록\n");
        printf("2. 시험 일정 등록\n");
        printf("3. 전체 일정 보기\n");
        printf("4. 시험 일정만 보기\n");
        printf("5. 완료 처리\n");
        printf("6. 일정 삭제\n");
        printf("0. 뒤로가기\n");
        sel = input_number("선택: ");

        if (sel == 1)      add_item(TYPE_STUDY);
        else if (sel == 2) add_item(TYPE_EXAM);
        else if (sel == 3) show_items(TYPE_ALL);
        else if (sel == 4) show_items(TYPE_EXAM);
        else if (sel == 5) complete_item();
        else if (sel == 6) delete_item();
        else if (sel != 0) printf("잘못된 선택입니다.\n");
    } while (sel != 0);

    free(items);   // 메모리 해제
    items = NULL;
}