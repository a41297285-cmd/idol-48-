// ==========================================
// IDOL PRODUCER 48 (Nintendo DS Homebrew)
// ==========================================
// 모든 화면 글자는 영어로 표시됩니다 (DS 기본 한글 미지원).
// 상단 화면: 로그 및 상태 / 하단 화면: 메뉴 조작
// ==========================================

#include <nds.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

// ==========================================
// [설정값] 여기 숫자를 바꾸면 게임이 달라져요!
// ==========================================
int STARTING_FUNDS = 10000;  // 시작 자금
int AUDITION_COST = 500;     // 오디션 진행 비용
int ALBUM_COST = 3000;       // 앨범 발매 비용
int CONCERT_COST = 2000;     // 콘서트 개최 비용
int MAX_MEMBERS = 16;        // 최대 선발 인원
int MAX_LOGS = 9;            // 화면에 보여줄 최대 로그 개수

// ==========================================
// 데이터 구조체
// ==========================================
typedef struct {
    char name[16];
    int popularity; // 1~5
    int skill;      // 1~5
    int visual;     // 1~5
    int fatigue;    // 0~100
    int is_active;  // 1 = 재적, 0 = 졸업
} Member;

Member members[100];
int member_count = 0;

int funds;
int fans;
int year = 1, month = 1, day = 1;
int album_count = 0;

// 게임 상태
typedef enum {
    STATE_TITLE,
    STATE_MAIN,
    STATE_GAMEOVER
} GameState;

GameState current_state = STATE_TITLE;
int menu_cursor = 0;
int menu_max = 5;

// 로그 시스템
char logs[10][64];

void add_log(const char* msg) {
    for(int i = MAX_LOGS; i > 0; i--) {
        strcpy(logs[i], logs[i-1]);
    }
    strcpy(logs[0], msg);
}

// 랜덤 텍스트용
const char* first_names[] = {"KIM", "LEE", "PARK", "CHOI", "JUNG", "KANG", "JO", "YUN", "JANG", "LIM"};
const char* last_names[] = {"MIN", "SU", "JIN", "YEON", "HEE", "JI", "EUN", "HYUN", "WOO", "AH"};

// ==========================================
// 게임 로직 함수
// ==========================================
void init_game() {
    funds = STARTING_FUNDS;
    fans = 0;
    year = 1; month = 1; day = 1;
    member_count = 0;
    album_count = 0;
    for(int i=0; i<=MAX_LOGS; i++) strcpy(logs[i], "");
    add_log("GAME STARTED! WELCOME, PRODUCER.");
}

void pass_day() {
    day++;
    if (day > 30) {
        day = 1;
        month++;
        if (month > 12) {
            month = 1;
            year++;
            add_log("A NEW YEAR HAS BEGUN!");
        }
    }

    // 매일 일어나는 확률 이벤트
    if (member_count > 0) {
        int event_rand = rand() % 100;
        
        // 5% 확률: 방송 출연 제의 (C~B급 상당)
        if (event_rand < 5) {
            int new_fans = 50 + (rand() % 100);
            fans += new_fans;
            funds += 20;
            char buf[64];
            sprintf(buf, "TV SHOW! EARNED %d FANS.", new_fans);
            add_log(buf);
        } 
        // 8% 확률: 누적 피로도 증가 및 랜덤 부상
        else if (event_rand >= 5 && event_rand < 13) {
            for(int i=0; i<member_count; i++) {
                if (members[i].is_active) members[i].fatigue += (rand() % 10);
            }
        }
        // 1% 확률: 스캔들 또는 졸업 (단기 역풍/팬 단결)
        else if (event_rand == 99) {
            int target = rand() % member_count;
            if (members[target].is_active) {
                int scandal_res = rand() % 100;
                if (scandal_res < 25) { // 25% 확률 팬 단결 폭발
                    fans += 500;
                    add_log("SCANDAL! BUT FANS UNITED!");
                } else {
                    members[target].is_active = 0;
                    char buf[64];
                    sprintf(buf, "SCANDAL... %s GRADUATED.", members[target].name);
                    add_log(buf);
                }
            }
        }
    }
}

void do_audition() {
    if (funds < AUDITION_COST) {
        add_log("NOT ENOUGH FUNDS (NEED 500)");
        return;
    }
    if (member_count >= MAX_MEMBERS) {
        add_log("MEMBER FULL! (MAX 16)");
        return;
    }
    funds -= AUDITION_COST;

    Member* m = &members[member_count];
    sprintf(m->name, "%s.%s", first_names[rand()%10], last_names[rand()%10]);
    m->popularity = 1 + (rand() % 3);
    m->skill = 1 + (rand() % 3);
    m->visual = 1 + (rand() % 3);
    
    // 팬이 많을수록 인재 등장 확률 증가 (올스탯 5/5/5 등)
    if (fans > 5000 && (rand() % 100) < 10) {
        m->popularity = 5; m->skill = 5; m->visual = 5;
        add_log("A GENIUS TRAINEE APPEARED!");
    }
    
    m->fatigue = 0;
    m->is_active = 1;
    member_count++;

    char buf[64];
    sprintf(buf, "NEW MEMBER: %s", m->name);
    add_log(buf);
}

void release_album() {
    if (member_count == 0) {
        add_log("NO MEMBERS TO SING!");
        return;
    }
    if (funds < ALBUM_COST) {
        add_log("NOT ENOUGH FUNDS (NEED 3000)");
        return;
    }
    funds -= ALBUM_COST;
    album_count++;

    // 판매량 산정 (기획서 반영: 인원 총합 및 현실적 난수)
    int pop_sum = 0;
    for(int i=0; i<member_count; i++) {
        if(members[i].is_active) pop_sum += members[i].popularity;
    }
    
    int random_multiplier = 500 + (rand() % 1500);
    int total_sales = (fans * 3) + (pop_sum * random_multiplier);
    int income = total_sales / 50; 

    funds += income;
    fans += (total_sales / 10);

    char buf[64];
    if (total_sales >= 1000000) sprintf(buf, "MEGA HIT! SOLD %d!", total_sales);
    else if (total_sales >= 100000) sprintf(buf, "HIT! SOLD %d!", total_sales);
    else if (total_sales <= 10000) sprintf(buf, "FLOP... SOLD %d", total_sales);
    else sprintf(buf, "ALBUM SOLD %d COPIES.", total_sales);
    
    add_log(buf);
}

void do_concert() {
    if (member_count == 0) {
        add_log("NO MEMBERS FOR CONCERT!");
        return;
    }
    if (funds < CONCERT_COST) {
        add_log("NOT ENOUGH FUNDS (NEED 2000)");
        return;
    }
    funds -= CONCERT_COST;

    // 완매도에 따른 등급 결정
    int attendance = (fans > 0) ? (rand() % fans) + 1000 : 1000;
    funds += (attendance / 10);
    fans += (attendance / 5);

    char buf[64];
    if (attendance > 30000) sprintf(buf, "DOME TOUR! %d CAME!", attendance);
    else if (attendance > 10000) sprintf(buf, "ARENA TOUR! %d CAME!", attendance);
    else sprintf(buf, "THEATER LIVE! %d CAME!", attendance);
    
    add_log(buf);
}

// ==========================================
// 메인 엔트리
// ==========================================
int main(void) {
    // 닌텐도 DS 비디오 모드 및 VRAM 설정
    videoSetMode(MODE_0_2D);
    videoSetModeSub(MODE_0_2D);
    vramSetBankA(VRAM_A_MAIN_BG);
    vramSetBankC(VRAM_C_SUB_BG);

    PrintConsole topScreen;
    PrintConsole bottomScreen;

    consoleInit(&topScreen, 3, BgType_Text4bpp, BgSize_T_256x256, 31, 0, true, true);
    consoleInit(&bottomScreen, 3, BgType_Text4bpp, BgSize_T_256x256, 31, 0, false, true);

    srand(time(NULL));

    while(1) {
        swiWaitForVBlank(); // 60fps 동기화
        scanKeys();
        int pressed = keysDown();

        if (current_state == STATE_TITLE) {
            consoleSelect(&bottomScreen);
            consoleClear();
            printf("\n\n\n\n\n\n      IDOL PRODUCER 48\n\n");
            printf("       PRESS START\n");

            consoleSelect(&topScreen);
            consoleClear();
            printf("==============================\n");
            printf("      IDOL PRODUCER 48\n");
            printf("==============================\n\n");
            printf(" Produce your dream idols!\n");
            printf(" Audition, Albums, Concerts!\n");
            printf(" Become a National Idol!\n\n");

            if (pressed & KEY_START) {
                init_game();
                current_state = STATE_MAIN;
            }
        }
        else if (current_state == STATE_MAIN) {
            // [상단 화면 렌더링]
            consoleSelect(&topScreen);
            consoleClear();
            printf("========= PRODUCER DESK ========\n");
            printf(" DATE: Y:%d M:%d D:%d\n", year, month, day);
            printf(" FUNDS: %d  |  FANS: %d\n", funds, fans);
            printf(" ALBUMS: %d  |  MEMBERS: %d/%d\n", album_count, member_count, MAX_MEMBERS);
            printf("--------------------------------\n");
            printf(" [ SYSTEM LOG ]\n");
            for(int i=0; i<MAX_LOGS; i++) {
                if(strlen(logs[i]) > 0) {
                    printf(" > %s\n", logs[i]);
                }
            }

            // [하단 화면 렌더링]
            consoleSelect(&bottomScreen);
            consoleClear();
            printf("=========== SCHEDULE ===========\n\n");
            
            const char* menu_opts[] = {
                "NEXT DAY / SKIP",
                "HOLD AUDITION  (-500)",
                "RELEASE ALBUM  (-3000)",
                "HOLD CONCERT   (-2000)",
                "REST MEMBERS   (Heal Fatigue)"
            };

            for(int i=0; i<menu_max; i++) {
                if(i == menu_cursor) printf("  ▶ %s\n\n", menu_opts[i]);
                else printf("     %s\n\n", menu_opts[i]);
            }

            // 조작 처리
            if (pressed & KEY_UP) {
                menu_cursor--;
                if (menu_cursor < 0) menu_cursor = menu_max - 1;
            }
            if (pressed & KEY_DOWN) {
                menu_cursor++;
                if (menu_cursor >= menu_max) menu_cursor = 0;
            }
            if (pressed & KEY_A) {
                switch(menu_cursor) {
                    case 0: pass_day(); break;
                    case 1: do_audition(); pass_day(); break;
                    case 2: release_album(); pass_day(); break;
                    case 3: do_concert(); pass_day(); break;
                    case 4:
                        for(int i=0; i<member_count; i++) {
                            if (members[i].fatigue > 0) members[i].fatigue -= 20;
                            if (members[i].fatigue < 0) members[i].fatigue = 0;
                        }
                        add_log("MEMBERS RESTED!");
                        pass_day();
                        break;
                }

                // 파산 조건
                if (funds < 0) {
                    current_state = STATE_GAMEOVER;
                }
            }
        }
        else if (current_state == STATE_GAMEOVER) {
            consoleSelect(&topScreen);
            consoleClear();
            printf("\n\n\n\n      ==================\n");
            printf("          BANKRUPT...\n");
            printf("      ==================\n");
            printf("\n       FUNDS: %d", funds);

            consoleSelect(&bottomScreen);
            consoleClear();
            printf("\n\n\n\n         GAME OVER\n\n");
            printf("    PRESS START TO RETRY");

            if (pressed & KEY_START) {
                current_state = STATE_TITLE;
            }
        }
    }
    return 0;
}
