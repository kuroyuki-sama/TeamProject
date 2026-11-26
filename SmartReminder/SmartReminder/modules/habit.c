// F2. 습관형성 리마인더 (담당: 황현준)
// 기능: 습관 등록 / 목록 보기 / 오늘 완료 체크 / 삭제 / 연속 달성 기록

#define _CRT_SECURE_NO_WARNINGS   // Visual Studio 경고 끄기
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "habit.h"

// ---------- 매크로 ----------
#define HABIT_FILE      "data/habit.dat"
#define MAX_HABIT       100
#define NAME_LEN        30
#define SECONDS_PER_DAY (24 * 60 * 60)
#define MAX(a, b)       ((a) > (b) ? (a) : (b))

// 디버그용 출력 (파일 맨 위에 #define DEBUG 를 쓰면 켜짐)
#ifdef DEBUG
#define DEBUG_LOG(msg)  printf("[DEBUG] %s\n", msg)
#else
#define DEBUG_LOG(msg)
#endif

// ---------- 구조체 ----------
typedef struct {
    int  id;             // 습관 번호
    char name[NAME_LEN]; // 습관 이름
    int  streak;         // 현재 연속 달성 일수
    int  best;           // 최고 연속 달성 일수
    int  lastDone;       // 마지막 완료 날짜 (예: 20261126, 없으면 0)
} Habit;

static Habit *habits;    // 동적 할당으로 만들 배열
static int count = 0;    // 등록된 습관 개수

// ---------- 도우미 함수 ----------

// 숫자 입력받기 (글자를 입력하면 다시 입력받음)
static int input_number(const char *message)
{
    int n;
    printf("%s", message);
    while (scanf("%d", &n) != 1) {
        while (getchar() != '\n');   // 잘못 입력한 내용 버리기
        printf("숫자로 입력하세요: ");
    }
    return n;
}

// 오늘 기준 daysAgo 일 전 날짜를 YYYYMMDD 숫자로 반환
// get_date(0) = 오늘, get_date(1) = 어제
static int get_date(int daysAgo)
{
    time_t t = time(NULL) - daysAgo * SECONDS_PER_DAY;
    struct tm *tm = localtime(&t);
    return (tm->tm_year + 1900) * 10000 + (tm->tm_mon + 1) * 100 + tm->tm_mday;
}

// 지금 연속 일수 (어제나 오늘 완료하지 않았으면 끊긴 것이므로 0)
static int current_streak(const Habit *h)
{
    if (h->lastDone == get_date(0) || h->lastDone == get_date(1))
        return h->streak;
    return 0;
}

// ---------- 파일 입출력 ----------

// 배열의 내용을 파일에 저장
static void save_habits(void)
{
    int i;
    FILE *fp = fopen(HABIT_FILE, "w");
    if (fp == NULL) {
        printf("파일 저장 실패! data 폴더가 있는지 확인하세요.\n");
        return;
    }
    DEBUG_LOG("save_habits");
    for (i = 0; i < count; i++) {
        fprintf(fp, "%d %s %d %d %d\n", habits[i].id, habits[i].name,
                habits[i].streak, habits[i].best, habits[i].lastDone);
    }
    fclose(fp);
}

// 파일에서 읽어서 배열에 저장
static void load_habits(void)
{
    FILE *fp = fopen(HABIT_FILE, "r");
    DEBUG_LOG("load_habits");
    if (fp == NULL) {
        return;   // 처음 실행하면 파일이 없으므로 그냥 끝냄
    }
    while (count < MAX_HABIT &&
           fscanf(fp, "%d %s %d %d %d", &habits[count].id, habits[count].name,
                  &habits[count].streak, &habits[count].best,
                  &habits[count].lastDone) == 5) {
        count++;
    }
    fclose(fp);
}

// ---------- 기능 함수 ----------

// 습관 목록 출력
static void show_habits(void)
{
    int i;
    if (count == 0) {
        printf("등록된 습관이 없습니다.\n");
        return;
    }
    printf("\n번호  오늘  연속  최고  습관\n");
    printf("---------------------------\n");
    for (i = 0; i < count; i++) {
        printf("%-4d  %-4s  %-4d  %-4d  %s\n", habits[i].id,
               habits[i].lastDone == get_date(0) ? "[v]" : "[ ]",
               current_streak(&habits[i]), habits[i].best, habits[i].name);
    }
}

// 습관 등록
static void add_habit(void)
{
    if (count >= MAX_HABIT) {
        printf("더 이상 등록할 수 없습니다.\n");
        return;
    }
    // 새 번호 = 마지막 번호 + 1
    habits[count].id = (count == 0) ? 1 : habits[count - 1].id + 1;

    printf("습관 이름(띄어쓰기 없이): ");
    scanf("%s", habits[count].name);
    habits[count].streak = 0;
    habits[count].best = 0;
    habits[count].lastDone = 0;
    count++;

    save_habits();
    printf("등록되었습니다.\n");
}

// 오늘 완료 체크
static void check_habit(void)
{
    int i, id;
    Habit *p = NULL;   // 포인터로 해당 습관을 가리킴

    if (count == 0) {
        printf("등록된 습관이 없습니다.\n");
        return;
    }
    show_habits();
    id = input_number("완료한 습관 번호: ");

    for (i = 0; i < count; i++) {
        if (habits[i].id == id) {
            p = &habits[i];
        }
    }
    if (p == NULL) {
        printf("없는 번호입니다.\n");
        return;
    }
    if (p->lastDone == get_date(0)) {
        printf("이미 오늘 완료했습니다.\n");
        return;
    }

    if (p->lastDone == get_date(1)) {
        p->streak++;       // 어제도 했으면 연속 +1
    } else {
        p->streak = 1;     // 끊겼으면 다시 1부터
    }
    p->best = MAX(p->best, p->streak);
    p->lastDone = get_date(0);

    save_habits();
    printf("완료! 연속 %d일째입니다.\n", p->streak);
}

// 습관 삭제
static void delete_habit(void)
{
    int i, j, id;

    if (count == 0) {
        printf("등록된 습관이 없습니다.\n");
        return;
    }
    show_habits();
    id = input_number("삭제할 습관 번호: ");

    for (i = 0; i < count; i++) {
        if (habits[i].id == id) {
            for (j = i; j < count - 1; j++) {
                habits[j] = habits[j + 1];   // 뒤의 것을 앞으로 당김
            }
            count--;
            save_habits();
            printf("삭제되었습니다.\n");
            return;
        }
    }
    printf("없는 번호입니다.\n");
}

// ---------- 메뉴 ----------
void habit_menu(void)
{
    int sel;

    habits = (Habit *)malloc(sizeof(Habit) * MAX_HABIT);   // 동적 메모리 할당
    if (habits == NULL) {
        printf("메모리 할당 실패\n");
        return;
    }
    count = 0;
    load_habits();

    do {
        printf("\n===== 습관형성 리마인더 =====\n");
        printf("1. 습관 등록\n");
        printf("2. 습관 목록\n");
        printf("3. 오늘 완료 체크\n");
        printf("4. 습관 삭제\n");
        printf("0. 뒤로가기\n");
        sel = input_number("선택: ");

        if (sel == 1)      add_habit();
        else if (sel == 2) show_habits();
        else if (sel == 3) check_habit();
        else if (sel == 4) delete_habit();
        else if (sel != 0) printf("잘못된 선택입니다.\n");
    } while (sel != 0);

    free(habits);   // 메모리 해제
    habits = NULL;
}