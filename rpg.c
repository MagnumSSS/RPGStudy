#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include "cJSON.h"
#include <time.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdbool.h>

//FILE* output_file = NULL;


struct task {
    char* title;
    struct task* child;
    struct task* next;
		int depth;
};

typedef struct {
	struct task* world;
	cJSON* progress; // территории 
	cJSON* events; // ивенты пользователя
	cJSON* user; // прокачка юзера
	cJSON* library; //библиотека
} GameWorld;

// Сигнатуры для этапа 4 // 
void show_help();
void save_user(GameWorld* gw);
void ensure_user_sections(GameWorld* gw);
void load_user(GameWorld* gw);
void add_kingdom_xp(GameWorld* gw, int xp);
void add_total_push(GameWorld* gw);
void load_library(GameWorld* gw);
void save_library(GameWorld* gw);
void add_book(GameWorld* gw, const char* title, const char* author, int pages);
void read_book(GameWorld* gw, const char* title, int pages);
void create_scroll(GameWorld* gw, const char* book_title, const char* scroll_title, const char* content);
void show_library(GameWorld* gw);
void craft_weapon(GameWorld* gw);
void use_equipment(GameWorld* gw, const char* title);
void add_element_xp(GameWorld* gw, const char* element_name);
void show_status(GameWorld* gw);
void show_kingdom_status(GameWorld* gw);
void show_forge_status(GameWorld* gw);
void show_elements_status(GameWorld* gw);
void show_rebellions_status(GameWorld* gw);




// Сигнатуры для этапа 3 //
void check_for_custom_events(GameWorld* gw);
void handle_complete(GameWorld* gw, const char* title);
void handle_push_c(GameWorld* gw, const char* title, const char* date_str);
void save_events(GameWorld* gw);
void load_events(GameWorld* gw);


// Сигнатуры для этапа 2 //
struct task* find_parent(struct task* world, const char* child_title);
char* get_current_date();
void log_text_in_file(char* text_push, char* title);
bool get_bool_field(cJSON* obj, char* title_obj);
int get_int_field(cJSON* obj, char* title_obj);
char* get_string_field(cJSON* obj, char* title_obj);
bool schedule_rebellion(cJSON* obj, struct task* node);
int8_t is_date_today_or_earler(const char* date_versus);
void reset_object_to_not_captured(GameWorld* gw, cJSON* obj_json, const char* title);
void fresh_news(GameWorld* gw);
bool is_object_in_kingdom(GameWorld* gw, const char* title, const char* kingdom_title);
void cancel_all_regular_rebellions(GameWorld* gw, const char* kingdom_title);
void activate_rebellion_at(cJSON* obj, time_t when);
void trigger_multiple_rebellion(GameWorld* gw, struct task* kingdom);
bool can_capture_node(GameWorld* gw, char* title);
void handle_prep(cJSON* obj, GameWorld* gw, char* title);
void handle_rebellion(cJSON* obj);
void handle_xp(cJSON* obj);
void handle_push_t(GameWorld* gw, char* title);
void handle_push(GameWorld* gw, char* flag, char* text_push, char* title);
struct task* find_by_title(struct task* node, const char* title);


// Сигнатуры для Этапа 1 //
GameWorld* load_game_state();
size_t element_length(const struct task* element);
void element_destroy(struct task* element);
struct task* element_last(struct task* element);
struct task* create_node_from_title(const char* title);
struct task* load_from_file(char* filename);
char* read_file(const char* filename);
void sync_node(struct task* node, cJSON* territories);
void calculate_kingdoms_town(struct task* kingdom, int* town, int* villages);
void save_game(GameWorld* gw);


/// 4 /// 
void show_help() {
    printf("\n");
    printf("╔══════════════════════════════════════════════════════════════╗\n");
    printf("║                    📖 СПРАВКА ПО RPG 📖                      ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n");
    
    printf("\n");
    printf("════════════════════════════════════════════════════════════════\n");
    printf("                    🎮 ОСНОВНЫЕ КОМАНДЫ                        \n");
    printf("════════════════════════════════════════════════════════════════\n");
    printf("\n");
    
    printf("  🔹 ./rpg push \"текст\"\n");
    printf("     Обычный пуш — добавляет опыт королевству (+1 XP)\n");
    printf("\n");
    
    printf("  🔹 ./rpg push -t \"текст\" \"территория\"\n");
    printf("     Захват территории — добавляет прогресс к захвату территории\n");
    printf("     Пример: ./rpg push -t \"изучил open\" \"сисвыз_open\"\n");
    printf("\n");
    
    printf("  🔹 ./rpg push -c \"событие\" \"дата\"\n");
    printf("     Создать событие с датой (формат: ГГГГ-ММ-ДД)\n");
    printf("     Пример: ./rpg push -c \"Экзамен по C\" \"2026-02-21\"\n");
    printf("\n");
    
    printf("  🔹 ./rpg push complete \"событие\"\n");
    printf("     Завершить событие (отметить как выполненное)\n");
    printf("     Пример: ./rpg push complete \"Экзамен по C\"\n");
    printf("\n");
    
    printf("════════════════════════════════════════════════════════════════\n");
    printf("                    ⚡ СТИХИИ И РАЗВИТИЕ                       \n");
    printf("════════════════════════════════════════════════════════════════\n");
    printf("\n");
    
    printf("  🔹 ./rpg push -s \"стихия\"\n");
    printf("     Прокачать стихию (+10 XP к стихии)\n");
    printf("     Пример: ./rpg push -s \"C\"\n");
    printf("\n");
    
    printf("════════════════════════════════════════════════════════════════\n");
    printf("                    ⚒️  КУЗНИЦА                                \n");
    printf("════════════════════════════════════════════════════════════════\n");
    printf("\n");
    
    printf("  🔹 ./rpg push -fc\n");
    printf("     Создать оружие в кузнице (+1 к запасу)\n");
    printf("\n");
    
    printf("  🔹 ./rpg push -fu \"территория\"\n");
    printf("     Использовать оружие на территории (уменьшает сложность захвата)\n");
    printf("     ⚠️ Ограничение: нельзя уменьшить сложность ниже половины от исходной\n");
    printf("     Пример: ./rpg push -fu \"сисвыз_open\"\n");
    printf("\n");
    
    printf("════════════════════════════════════════════════════════════════\n");
    printf("                    📚 БИБЛИОТЕКА                              \n");
    printf("════════════════════════════════════════════════════════════════\n");
    printf("\n");
    
    printf("  🔹 ./rpg library add \"название\" --author \"автор\" --pages <число>\n");
    printf("     Добавить книгу в библиотеку\n");
    printf("     Пример: ./rpg library add \"C Programming Language\" --author \"K&R\" --pages 272\n");
    printf("\n");
    
    printf("  🔹 ./rpg library read \"название\" --pages <число>\n");
    printf("     Прочитать страницы книги (+1 XP королевству за каждые 10 страниц)\n");
    printf("     Пример: ./rpg library read \"C Programming Language\" --pages 20\n");
    printf("\n");
    
    printf("  🔹 ./rpg library scroll \"название\" --title \"свиток\" --content \"текст\"\n");
    printf("     Создать свиток (конспект) для книги\n");
    printf("     Пример: ./rpg library scroll \"C Programming Language\" --title \"Указатели\" --content \"Указатель — это адрес...\"\n");
    printf("\n");
    
    printf("  🔹 ./rpg library show\n");
    printf("     Показать всю библиотеку с прогрессом чтения и свитками\n");
    printf("\n");
    
    printf("════════════════════════════════════════════════════════════════\n");
    printf("                    📊 СТАТУС И ГАЗЕТЧИКИ                      \n");
    printf("════════════════════════════════════════════════════════════════\n");
    printf("\n");
    
    printf("  🔹 ./rpg status\n");
    printf("     Показать общий статус королевства (уровень, прогресс, события)\n");
    printf("\n");
    
    printf("  🔹 ./rpg kingdom\n");
    printf("     Детальный статус королевства и зданий (кузница, библиотека, арена)\n");
    printf("\n");
    
    printf("  🔹 ./rpg forge\n");
    printf("     Статус кузницы (запас оружия, всего создано)\n");
    printf("\n");
    
    printf("  🔹 ./rpg elements\n");
    printf("     Статус стихий (уровни, опыт)\n");
    printf("\n");
    
    printf("  🔹 ./rpg rebellions\n");
    printf("     Активные мятежи и их прогресс подавления\n");
    printf("\n");
    
    printf("  🔹 ./rpg events\n");
    printf("     Планируемые события (сегодня, впереди, завершённые)\n");
    printf("\n");
    
    printf("════════════════════════════════════════════════════════════════\n");
    printf("                    🔄 ИНИЦИАЛИЗАЦИЯ                           \n");
    printf("════════════════════════════════════════════════════════════════\n");
    printf("\n");
    
    printf("  🔹 ./rpg --init\n");
    printf("     Инициализация игры (первый запуск) + показать газетчики за сегодня\n");
    printf("\n");
    
    printf("════════════════════════════════════════════════════════════════\n");
    printf("                    💡 ПОЛЕЗНЫЕ СОВЕТЫ                         \n");
    printf("════════════════════════════════════════════════════════════════\n");
    printf("\n");
    
    printf("  ✅ Регулярность важнее интенсивности — подавляйте мятежи вовремя!\n");
    printf("  ✅ Крафтите оружие заранее — оно поможет на сложных территориях.\n");
    printf("  ✅ Читайте книги — это даёт опыт королевству и развивает стихии.\n");
    printf("  ✅ Захватывайте территории по иерархии: улица → город → страна.\n");
    printf("  ✅ Используйте стихии — они дают бонусы к связанным территориям.\n");
    printf("\n");
    
    printf("════════════════════════════════════════════════════════════════\n");
    printf("                    🎯 БЫСТРЫЙ СТАРТ                            \n");
    printf("════════════════════════════════════════════════════════════════\n");
    printf("\n");
    
    printf("  1. ./rpg --init                    # Инициализация игры\n");
    printf("  2. ./rpg push -t \"изучил\" \"сисвыз_open\"  # Захват улицы\n");
    printf("  3. ./rpg push -s \"C\"              # Прокачать стихию\n");
    printf("  4. ./rpg push -fc                  # Создать оружие (на будущее)\n");
    printf("  5. ./rpg status                    # Проверить прогресс\n");
    printf("\n");
    
    printf("╔══════════════════════════════════════════════════════════════╗\n");
    printf("║                    УДАЧИ В РАЗВИТИИ! 🏰                      ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n");
    printf("\n");
}

void save_user(GameWorld* gw){
	if(!gw->user) return;

	char* json_str = cJSON_Print(gw->user);
	if (!json_str) {
		printf("Ошибка создания JSON-строки для событий\n");
    return;
  }

	FILE* f = fopen("user.json", "w");
	if(f){
		fprintf(f, "%s", json_str);
		fclose(f);
	}
	free(json_str);
}

void ensure_user_sections(GameWorld* gw) {
		/// если нет объекта, добавляем
    // Королевство
    if (!cJSON_GetObjectItem(gw->user, "kingdom")) {
        cJSON* kingdom = cJSON_CreateObject();
        cJSON_AddNumberToObject(kingdom, "level", 1);
        cJSON_AddNumberToObject(kingdom, "xp", 0);
        cJSON_AddNumberToObject(kingdom, "xp_to_next", 100);
        cJSON_AddNumberToObject(kingdom, "total_pushes", 0);
        cJSON_AddStringToObject(kingdom, "last_active_date", get_current_date());
        cJSON_AddItemToObject(gw->user, "kingdom", kingdom);
    }
    
    // Здания
    if (!cJSON_GetObjectItem(gw->user, "buildings")) {
        cJSON* buildings = cJSON_CreateObject();
        cJSON_AddItemToObject(gw->user, "buildings", buildings);
    }
    
    cJSON* buildings = cJSON_GetObjectItem(gw->user, "buildings");
    
    // Кузница
    if (!cJSON_GetObjectItem(buildings, "forge")) {
        cJSON* forge = cJSON_CreateObject();
        cJSON_AddNumberToObject(forge, "level", 1);
        cJSON_AddNumberToObject(forge, "weapons_crafted", 0);
        cJSON_AddNumberToObject(forge, "equipment_stock", 0);
        cJSON_AddStringToObject(forge, "last_craft_date", "");
        cJSON_AddItemToObject(buildings, "forge", forge);
    }
    
    // Библиотека
    if (!cJSON_GetObjectItem(buildings, "library")) {
        cJSON* library = cJSON_CreateObject();
        cJSON_AddNumberToObject(library, "level", 1);
        cJSON_AddNumberToObject(library, "books_read", 0);
				cJSON_AddNumberToObject(library, "pages_read_today", 0);
				cJSON_AddNumberToObject(library, "pages_read_total", 0);
        cJSON_AddNumberToObject(library, "scrolls_created", 0);
        cJSON_AddItemToObject(buildings, "library", library);
    }
    
    // Стихии
    if (!cJSON_GetObjectItem(gw->user, "elements")) {
        cJSON_AddItemToObject(gw->user, "elements", cJSON_CreateObject());
    }
}

void load_user(GameWorld* gw){
	FILE* f = fopen("user.json", "r");
	if(!f){
		gw->user = cJSON_CreateObject();
		
		// королество
		cJSON* kingdom = cJSON_CreateObject();
		cJSON_AddNumberToObject(kingdom, "level", 1);
    cJSON_AddNumberToObject(kingdom, "xp", 0);
    cJSON_AddNumberToObject(kingdom, "xp_to_next", 10);
    cJSON_AddNumberToObject(kingdom, "total_pushes", 0);
    cJSON_AddStringToObject(kingdom, "last_active_date", get_current_date());
    cJSON_AddItemToObject(gw->user, "kingdom", kingdom);
		

		// все здания
		cJSON* buildings = cJSON_CreateObject();

		// Кузница
		// cJSON_CreateObject(); - создание объекта
    cJSON* forge = cJSON_CreateObject();
		// добавление ему полей
    cJSON_AddNumberToObject(forge, "level", 1);
    cJSON_AddNumberToObject(forge, "weapons_crafted", 0);
    cJSON_AddNumberToObject(forge, "equipment_stock", 0);
    cJSON_AddStringToObject(forge, "last_craft_date", "");
		// cJSON_AddItemToObject - добавление объекта b в объект a
    cJSON_AddItemToObject(buildings, "forge", forge);
		
		// Библиотека
    cJSON* library = cJSON_CreateObject();
		cJSON_AddNumberToObject(library, "level", 1);
    cJSON_AddNumberToObject(library, "books_read", 0);
    cJSON_AddNumberToObject(library, "scrolls_created", 0);
    cJSON_AddItemToObject(buildings, "library", library);  

    cJSON_AddItemToObject(gw->user, "buildings", buildings);
		
		// стихии, пока пусто
		cJSON_AddItemToObject(gw->user, "elements", cJSON_CreateObject());

		save_user(gw);
		return;
	}

	// Загружаем существующий файл
  fseek(f, 0, SEEK_END);
  long len = ftell(f);
  fseek(f, 0, SEEK_SET);
    
	// делаем все также как и в 3-ем этапе
  char* data = malloc(len + 1);
  fread(data, 1, len, f);
  data[len] = '\0';
  fclose(f);
    

  gw->user = cJSON_Parse(data);
  free(data);
    
  if (!gw->user) {
    printf("Ошибка парсинга user.json\n");
    gw->user = cJSON_CreateObject();
  }

	// проверка
	ensure_user_sections(gw);
}

// прокачка королевства
void add_kingdom_xp(GameWorld* gw, int xp) {
    cJSON* kingdom = cJSON_GetObjectItem(gw->user, "kingdom");
    if (!kingdom) return;
    
    int current_xp = get_int_field(kingdom, "xp");
    int xp_to_next = get_int_field(kingdom, "xp_to_next");
    int level = get_int_field(kingdom, "level");
    
    current_xp += xp;
    
    // Проверка уровня
    if (current_xp >= xp_to_next) {
        level++;
        current_xp = 0;
        xp_to_next *= 2; // экспоненциальный рост
        
        cJSON_ReplaceItemInObject(kingdom, "level", cJSON_CreateNumber(level));
        cJSON_ReplaceItemInObject(kingdom, "xp_to_next", cJSON_CreateNumber(xp_to_next));
        
        printf("👑 Королевство достигло уровня %d!\n", level);
    }
    
    cJSON_ReplaceItemInObject(kingdom, "xp", cJSON_CreateNumber(current_xp));
}

// увеличение колва всех пушей
void add_total_push(GameWorld* gw) {
    cJSON* kingdom = cJSON_GetObjectItem(gw->user, "kingdom");
    if (!kingdom) return;
    
    int total = get_int_field(kingdom, "total_pushes");
    total++;
    cJSON_ReplaceItemInObject(kingdom, "total_pushes", cJSON_CreateNumber(total));
}

// библиотека - загрузка
void load_library(GameWorld* gw) {
    FILE* f = fopen("library.json", "r");
    if (!f) {
        // Создаём новый файл
        gw->library = cJSON_CreateObject();
        cJSON_AddItemToObject(gw->library, "books", cJSON_CreateArray());
        save_library(gw);
        return;
    }
    
    // Загрузка существующего файла (аналогично другим)
    fseek(f, 0, SEEK_END);
    long len = ftell(f);
    fseek(f, 0, SEEK_SET);
    
    char* data = malloc(len + 1);
    fread(data, 1, len, f);
    data[len] = '\0';
    fclose(f);
    
    gw->library = cJSON_Parse(data);
    free(data);
    
    if (!gw->library) {
        gw->library = cJSON_CreateObject();
    }
    
    // Гарантия наличия массива книг
    if (!cJSON_GetObjectItem(gw->library, "books")) {
        cJSON_AddItemToObject(gw->library, "books", cJSON_CreateArray());
    }
}

// сохранение библиотеки
void save_library(GameWorld* gw) {
    if (!gw->library) return;
    
    char* json_str = cJSON_Print(gw->library);
    if (!json_str) return;
    
    FILE* f = fopen("library.json", "w");
    if (f) {
        fprintf(f, "%s", json_str);
        fclose(f);
    }
    free(json_str);
}

// добавление книги
void add_book(GameWorld* gw, const char* title, const char* author, int pages) {
    if (!gw->library) {
        printf("❌ Библиотека не инициализирована!\n");
        return;
    }
    
    cJSON* books = cJSON_GetObjectItem(gw->library, "books");
    if (!books) {
        books = cJSON_CreateArray();
        cJSON_AddItemToObject(gw->library, "books", books);
    }
    
    // Проверяем, существует ли уже такая книга
    cJSON* book = books->child;
    while (book) {
        char* book_title = get_string_field(book, "title");
        if (book_title && strcmp(book_title, title) == 0) {
            printf("⚠️ Книга '%s' уже существует!\n", title);
            return;
        }
        book = book->next;
    }
    
    // Создаём новую книгу
    cJSON* new_book = cJSON_CreateObject();
    cJSON_AddStringToObject(new_book, "title", title);
    cJSON_AddStringToObject(new_book, "author", author);
    cJSON_AddNumberToObject(new_book, "total_pages", pages);
    cJSON_AddNumberToObject(new_book, "read_pages", 0);
    cJSON_AddStringToObject(new_book, "status", "unread");
    cJSON_AddItemToObject(new_book, "scrolls", cJSON_CreateArray());
    
    cJSON_AddItemToArray(books, new_book);
    
    // Обновляем статистику в user.json
    cJSON* lib_stats = cJSON_GetObjectItem(cJSON_GetObjectItem(gw->user, "buildings"), "library");
    if (lib_stats) {
        int total = get_int_field(lib_stats, "books_read");
        cJSON_ReplaceItemInObject(lib_stats, "books_read", cJSON_CreateNumber(total + 1));
    }
    
    printf("📚 Добавлена книга: %s (%s)\n", title, author);
}

// прочтение книги
void read_book(GameWorld* gw, const char* title, int pages) {
    if (!gw->library) {
        printf("❌ Библиотека не инициализирована!\n");
        return;
    }
    
    cJSON* books = cJSON_GetObjectItem(gw->library, "books");
    if (!books) {
        printf("❌ Нет книг в библиотеке!\n");
        return;
    }
    
    cJSON* book = books->child;
    while (book) {
        char* book_title = get_string_field(book, "title");
        if (book_title && strcmp(book_title, title) == 0) {
            int read_pages = get_int_field(book, "read_pages");
            int total_pages = get_int_field(book, "total_pages");
            
            read_pages += pages;
            if (read_pages > total_pages) {
                read_pages = total_pages;
            }
            
            cJSON_ReplaceItemInObject(book, "read_pages", cJSON_CreateNumber(read_pages));
            
            // Обновляем статус
            char* status = "reading";
            if (read_pages == total_pages) {
                status = "read";
            }
            cJSON_ReplaceItemInObject(book, "status", cJSON_CreateString(status));
            
            printf("📖 Прочитано %d страниц книги '%s' (%d/%d)\n", pages, title, read_pages, total_pages);
            
            // Добавляем опыт королевству
            add_kingdom_xp(gw, pages / 10); // 1 XP за каждые 10 страниц
            
            return;
        }
        book = book->next;
    }
    
    printf("❌ Книга не найдена: %s\n", title);
}

// создание свитков краткого содержания для прочитанного
void create_scroll(GameWorld* gw, const char* book_title, const char* scroll_title, const char* content) {
    if (!gw->library) {
        printf("❌ Библиотека не инициализирована!\n");
        return;
    }
    
    cJSON* books = cJSON_GetObjectItem(gw->library, "books");
    if (!books) {
        printf("❌ Нет книг в библиотеке!\n");
        return;
    }
    
    cJSON* book = books->child;
    while (book) {
        char* title = get_string_field(book, "title");
        if (title && strcmp(title, book_title) == 0) {
            cJSON* scrolls = cJSON_GetObjectItem(book, "scrolls");
            if (!scrolls) {
                scrolls = cJSON_CreateArray();
                cJSON_AddItemToObject(book, "scrolls", scrolls);
            }
            
            // Создаём свиток
            cJSON* scroll = cJSON_CreateObject();
            cJSON_AddStringToObject(scroll, "title", scroll_title);
            cJSON_AddStringToObject(scroll, "content", content);
            cJSON_AddStringToObject(scroll, "date", get_current_date());
            
            cJSON_AddItemToArray(scrolls, scroll);
            
            // Обновляем статистику в user.json
            cJSON* lib_stats = cJSON_GetObjectItem(cJSON_GetObjectItem(gw->user, "buildings"), "library");
            if (lib_stats) {
                int total = get_int_field(lib_stats, "scrolls_created");
                cJSON_ReplaceItemInObject(lib_stats, "scrolls_created", cJSON_CreateNumber(total + 1));
            }
            
            printf("📜 Создан свиток '%s' для книги '%s'\n", scroll_title, book_title);
            return;
        }
        book = book->next;
    }
    
    printf("❌ Книга не найдена: %s\n", book_title);
}

// показать библиотеку
void show_library(GameWorld* gw) {
    if (!gw->library) {
        printf("❌ Библиотека не загружена!\n");
        return;
    }
    
    printf("\n");
    printf("╔══════════════════════════════════════════════════════════════╗\n");
    printf("║                    📚 МОЯ БИБЛИОТЕКА 📚                      ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n");
    
    cJSON* books = cJSON_GetObjectItem(gw->library, "books");
    if (!books || books->child == NULL) {
        printf("\n📭 Библиотека пуста. Добавьте книги: ./rpg library add\n");
        return;
    }
    
    cJSON* book = books->child;
    int unread = 0, reading = 0, read = 0;
    
    while (book) {
        char* title = get_string_field(book, "title");
        char* author = get_string_field(book, "author");
        int total = get_int_field(book, "total_pages");
        int read_pages = get_int_field(book, "read_pages");
        char* status = get_string_field(book, "status");
        
        printf("\n");
        printf("   📖 %s\n", title);
        printf("      Автор: %s\n", author);
        printf("      Страниц: %d/%d\n", read_pages, total);
        printf("      Статус: %s", status);
        
        if (strcmp(status, "unread") == 0) unread++;
        else if (strcmp(status, "reading") == 0) reading++;
        else if (strcmp(status, "read") == 0) read++;
        
        // Прогресс бар
        int progress = (read_pages * 30) / total;
        printf(" [");
        for (int i = 0; i < 30; i++) {
            printf("%s", (i < progress) ? "█" : "░");
        }
        printf("] %d%%\n", progress * 3);
        
        // Свитки
        cJSON* scrolls = cJSON_GetObjectItem(book, "scrolls");
        if (scrolls && scrolls->child) {
            printf("      📜 Свитки: ");
            cJSON* scroll = scrolls->child;
            while (scroll) {
                char* scroll_title = get_string_field(scroll, "title");
                printf("%s", scroll_title);
                scroll = scroll->next;
                if (scroll) printf(", ");
            }
            printf("\n");
        }
        
        book = book->next;
    }
    
    printf("\n");
    printf("📊 Статистика:\n");
    printf("   📖 Всего книг: %d\n", unread + reading + read);
    printf("   ✅ Прочитано: %d | 📝 Читаю: %d | ❌ Не читал: %d\n", read, reading, unread);
    
    printf("\n");
    printf("📖 Команды:\n");
    printf("   ./rpg library add <название> --author <автор> --pages <страниц>\n");
    printf("   ./rpg library read <название> --pages <страниц>\n");
    printf("   ./rpg library scroll <название> --title <свиток> --content <текст>\n");
    
    printf("\n");
}

// кузня
void craft_weapon(GameWorld* gw) {
    cJSON* buildings = cJSON_GetObjectItem(gw->user, "buildings");
    if (!buildings) return;
    
    cJSON* forge = cJSON_GetObjectItem(buildings, "forge");
    if (!forge) return;
    
    // крафтим оружие (без ограничений!)
		// сколько оружий есть сейчас
    int stock = get_int_field(forge, "equipment_stock");
    stock++;
    
		// сколько оружий было создано в принципе
    int crafted = get_int_field(forge, "weapons_crafted");
    crafted++;
    
    cJSON_ReplaceItemInObject(forge, "equipment_stock", cJSON_CreateNumber(stock));
    cJSON_ReplaceItemInObject(forge, "weapons_crafted", cJSON_CreateNumber(crafted));
    cJSON_ReplaceItemInObject(forge, "last_craft_date", cJSON_CreateString(get_current_date()));
    printf("⚔️ Создано новое оружие! Запас: %d (всего создано: %d)\n", stock, crafted);
}

// использование оружия для захвата
void use_equipment(GameWorld* gw, const char* title) {
    cJSON* buildings = cJSON_GetObjectItem(gw->user, "buildings");
    if (!buildings) return;
    
    cJSON* forge = cJSON_GetObjectItem(buildings, "forge");
    if (!forge) return;
    
    int stock = get_int_field(forge, "equipment_stock");
    
    if (stock <= 0) {
        printf("❌ Нет оружия в запасе!\n");
        return;
    }
    
    // Находим территорию
    cJSON* territories = cJSON_GetObjectItem(gw->progress, "territories");
    cJSON* obj = cJSON_GetObjectItem(territories, title);
    
    if (!obj) {
        printf("❌ Территория не найдена: %s\n", title);
        return;
    }
    
    // Читаем текущую и исходную сложность
    int current_scores = get_int_field(obj, "count_scores");
    int original_scores = get_int_field(obj, "original_count_scores");
    
    if (original_scores == -1) {
        // Если поле не существует, используем текущее как исходное
        original_scores = current_scores;
    }
    
    // Вычисляем минимальную допустимую сложность (половина от исходной)
    int min_scores = (original_scores + 1) / 2; // Округление вверх
    
    // Проверяем, можно ли уменьшить
    if (current_scores <= min_scores) {
        printf("⚠️ Сложность уже на минимальном уровне (%d/%d)\n", current_scores, original_scores);
        return;
    }
    
    // Уменьшаем сложность
    current_scores--;
    
    cJSON_ReplaceItemInObject(obj, "count_scores", cJSON_CreateNumber(current_scores));
    stock--;
    
    cJSON_ReplaceItemInObject(forge, "equipment_stock", cJSON_CreateNumber(stock));
    
    printf("🛡️ Использовано оружие для %s!\n", title);
    printf("   Сложность: %d → %d (минимум: %d)\n", current_scores + 1, current_scores, min_scores);
}

// создание стихии и его прокачка
void add_element_xp(GameWorld* gw, const char* element_name) {
		// проверка на наличие
    cJSON* elements = cJSON_GetObjectItem(gw->user, "elements");
    if (!elements) return;
    
		
    cJSON* element = cJSON_GetObjectItem(elements, element_name);
		// если такого элемента не найдено, создаем
    if (!element) {
        // Создаём новую стихию
        element = cJSON_CreateObject();
        cJSON_AddNumberToObject(element, "level", 1);
        cJSON_AddNumberToObject(element, "xp", 0);
        cJSON_AddNumberToObject(element, "xp_to_next", 100);
        cJSON_AddItemToObject(elements, element_name, element);
    }
    // прокачиваем 
    int current_xp = get_int_field(element, "xp");
    int xp_to_next = get_int_field(element, "xp_to_next");
    int level = get_int_field(element, "level");
    
    current_xp++;
    
    // Проверка уровня
    if (current_xp >= xp_to_next) {
        level++;
        current_xp = 0;
        xp_to_next *= 2;
        
        cJSON_ReplaceItemInObject(element, "level", cJSON_CreateNumber(level));
        cJSON_ReplaceItemInObject(element, "xp_to_next", cJSON_CreateNumber(xp_to_next));
        
        printf("✨ Стихия '%s' достигла уровня %d!\n", element_name, level);
    }
    
    cJSON_ReplaceItemInObject(element, "xp", cJSON_CreateNumber(current_xp));
}

// красивый вывод
// ./rpg status - общий статус
void show_status(GameWorld* gw) {
    printf("\n");
    printf("╔══════════════════════════════════════════════════════════════╗\n");
    printf("║                    🏰 СТАТУС КОРОЛЕВСТВА 🏰                  ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n");
    
    // Королевство
    cJSON* kingdom = cJSON_GetObjectItem(gw->user, "kingdom");
    if (kingdom) {
        int level = get_int_field(kingdom, "level");
        int xp = get_int_field(kingdom, "xp");
        int xp_to_next = get_int_field(kingdom, "xp_to_next");
        int total_pushes = get_int_field(kingdom, "total_pushes");
        
        printf("\n👑 Уровень королевства: %d (XP: %d/%d)\n", level, xp, xp_to_next);
        printf("📊 Всего пушей: %d\n", total_pushes);
        
        // Прогресс бар
        int progress = (xp * 50) / xp_to_next;
        printf("📈 Прогресс: [");
        for (int i = 0; i < 50; i++) {
            printf("%s", (i < progress) ? "█" : "░");
        }
        printf("] %d%%\n", progress * 2);
    }
    
    // Захваченные территории
    cJSON* territories = cJSON_GetObjectItem(gw->progress, "territories");
    if (territories) {
        int captured = 0;
        int total = 0;
        
        cJSON* obj = territories->child;
        while (obj) {
            char* status = get_string_field(obj, "status");
            if (status && strcmp(status, "captured") == 0) {
                captured++;
            }
            total++;
            obj = obj->next;
        }
        
        printf("\n🌍 Территории: %d/%d захвачено (%.1f%%)\n", captured, total, (captured * 100.0) / total);
        
        // Прогресс бар
        int progress = (captured * 50) / total;
        printf("🗺️  Прогресс: [");
        for (int i = 0; i < 50; i++) {
            printf("%s", (i < progress) ? "█" : "░");
        }
        printf("] %d%%\n", progress * 2);
    }
    
    // Активные мятежи
    int rebellions = 0;
    if (territories) {
        cJSON* obj = territories->child;
        while (obj) {
            if (get_bool_field(obj, "is_in_rebellion")) {
                rebellions++;
            }
            obj = obj->next;
        }
    }
    
    printf("\n⚔️  Активных мятежей: %d\n", rebellions);
    
    printf("\n");
    printf("╔══════════════════════════════════════════════════════════════╗\n");
    printf("║                    📅 БЛИЖАЙШИЕ СОБЫТИЯ 📅                   ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n");
    
    check_for_custom_events(gw);
    
    printf("\n");
}

// ./rpg kingdom - статус королевства
void show_kingdom_status(GameWorld* gw) {
    cJSON* kingdom = cJSON_GetObjectItem(gw->user, "kingdom");
    if (!kingdom) {
        printf("❌ Королевство не найдено\n");
        return;
    }
    
    int level = get_int_field(kingdom, "level");
    int xp = get_int_field(kingdom, "xp");
    int xp_to_next = get_int_field(kingdom, "xp_to_next");
    int total_pushes = get_int_field(kingdom, "total_pushes");
    char* last_active = get_string_field(kingdom, "last_active_date");
    
    printf("\n");
    printf("╔══════════════════════════════════════════════════════════════╗\n");
    printf("║                    👑 КОРОЛЕВСТВО 👑                         ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n");
    printf("\n");
    printf("   🏰 Уровень: %d\n", level);
    printf("   ⭐ Опыт: %d / %d\n", xp, xp_to_next);
    
    int progress = (xp * 30) / xp_to_next;
    printf("   📊 Прогресс: [");
    for (int i = 0; i < 30; i++) {
        printf("%s", (i < progress) ? "█" : "░");
    }
    printf("] %d%%\n", progress * 3);
    
    printf("\n");
    printf("   📈 Всего пушей: %d\n", total_pushes);
    printf("   📅 Последняя активность: %s\n", last_active ? last_active : "неизвестно");
    
    printf("\n");
    printf("╔══════════════════════════════════════════════════════════════╗\n");
    printf("║                    🏗️  ЗДАНИЯ 🏗️                             ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n");
    
    cJSON* buildings = cJSON_GetObjectItem(gw->user, "buildings");
    if (buildings) {
        // Кузница
        cJSON* forge = cJSON_GetObjectItem(buildings, "forge");
        if (forge) {
            int level = get_int_field(forge, "level");
            int stock = get_int_field(forge, "equipment_stock");
            int crafted = get_int_field(forge, "weapons_crafted");
            
            printf("\n   ⚒️  КУЗНИЦА (Уровень %d)\n", level);
            printf("      Оружие в запасе: %d\n", stock);
            printf("      Всего создано: %d\n", crafted);
        }
        
        // Библиотека
        cJSON* library = cJSON_GetObjectItem(buildings, "library");
        if (library) {
            int level = get_int_field(library, "level");
            int books = get_int_field(library, "books_read");
            int scrolls = get_int_field(library, "scrolls_created");
            
            printf("\n   📚 БИБЛИОТЕКА (Уровень %d)\n", level);
            printf("      Прочитано книг: %d\n", books);
            printf("      Создано свитков: %d\n", scrolls);
        }
    }
    
    printf("\n");
}

// ./rpg forge — Статус кузницы
void show_forge_status(GameWorld* gw) {
    cJSON* buildings = cJSON_GetObjectItem(gw->user, "buildings");
    if (!buildings) {
        printf("❌ Здания не найдены\n");
        return;
    }
    
    cJSON* forge = cJSON_GetObjectItem(buildings, "forge");
    if (!forge) {
        printf("❌ Кузница не найдена\n");
        return;
    }
    
    int level = get_int_field(forge, "level");
    int stock = get_int_field(forge, "equipment_stock");
    int crafted = get_int_field(forge, "weapons_crafted");
    
    printf("\n");
    printf("╔══════════════════════════════════════════════════════════════╗\n");
    printf("║                    ⚒️  КУЗНИЦА ⚒️                            ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n");
    printf("\n");
    printf("   🏭 Уровень: %d\n", level);
    printf("   🗡️  Оружие в запасе: %d шт.\n", stock);
    printf("   🔨 Всего создано: %d шт.\n", crafted);
    
    printf("\n");
    printf("   📖 Команды:\n");
    printf("      ./rpg push -fc        — создать оружие\n");
    printf("      ./rpg push -fu <терр> — использовать на территории\n");
    
    printf("\n");
}

// ./rpg elements — Статус стихий
void show_elements_status(GameWorld* gw) {
    cJSON* elements = cJSON_GetObjectItem(gw->user, "elements");
    if (!elements) {
        printf("❌ Стихии не найдены\n");
        return;
    }
    
    printf("\n");
    printf("╔══════════════════════════════════════════════════════════════╗\n");
    printf("║                    🌊🔥🌪️🌍 СТИХИИ 🌍🌪️🔥🌊                   ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n");
    
    cJSON* elem = elements->child;
    if (!elem) {
        printf("\n📭 Нет изученных стихий. Начните с: ./rpg push -s <стихия>\n");
    }
    
    while (elem) {
        int level = get_int_field(elem, "level");
        int xp = get_int_field(elem, "xp");
        int xp_to_next = get_int_field(elem, "xp_to_next");
        
        printf("\n");
        printf("   🌟 %s (Уровень %d)\n", elem->string, level);
        printf("      XP: %d / %d\n", xp, xp_to_next);
        
        int progress = (xp * 30) / xp_to_next;
        printf("      [");
        for (int i = 0; i < 30; i++) {
            printf("%s", (i < progress) ? "█" : "░");
        }
        printf("] %d%%\n", progress * 3);
        
        elem = elem->next;
    }
    
    printf("\n");
    printf("   📖 Команды:\n");
    printf("      ./rpg push -s <стихия>        — прокачать стихию +10 XP\n");
    printf("      ./rpg push -ts <стихия> <терр> — захватить + прокачать стихию\n");
    
    printf("\n");
}

// ./rpg rebellions — Активные мятежи
void show_rebellions_status(GameWorld* gw) {
    cJSON* territories = cJSON_GetObjectItem(gw->progress, "territories");
    if (!territories) {
        printf("❌ Территории не найдены\n");
        return;
    }
    
    printf("\n");
    printf("╔══════════════════════════════════════════════════════════════╗\n");
    printf("║                    ⚔️  АКТИВНЫЕ МЯТЕЖИ ⚔️                    ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n");
    
    int count = 0;
    cJSON* obj = territories->child;
    
    while (obj) {
        if (get_bool_field(obj, "is_in_rebellion")) {
            char* title = obj->string;
            char* date_reb = get_string_field(obj, "date_rebellion");
            int pushes_done = get_int_field(obj, "rebellion_pushes_done");
            int pushes_needed = get_int_field(obj, "rebellion_pushes_needed");
            
            if (pushes_needed == -1) pushes_needed = 3;
            if (pushes_done == -1) pushes_done = 0;
            
            printf("\n");
            printf("   🟥 %s\n", title);
            printf("      Начало мятежа: %s\n", date_reb ? date_reb : "неизвестно");
            printf("      Прогресс подавления: %d / %d\n", pushes_done, pushes_needed);
            
            int progress = (pushes_done * 30) / pushes_needed;
            printf("      [");
            for (int i = 0; i < 30; i++) {
                printf("%s", (i < progress) ? "█" : "░");
            }
            printf("] %d%%\n", progress * 3);
            
            count++;
        }
        obj = obj->next;
    }
    
    if (count == 0) {
        printf("\n📭 Нет активных мятежей. Мир в королевстве!\n");
    } else {
        printf("\n📊 Всего активных мятежей: %d\n", count);
    }
    
    printf("\n");
}

/// 3 ///

// газетчик ивентов пользователя
void check_for_custom_events(GameWorld* gw) {
		// получаем ивенты пользователя
    cJSON* events_array = cJSON_GetObjectItem(gw->events, "custom_events");
    if (!events_array) return;
    
		// получаем объекты родителя
    cJSON* event = events_array->child;
    int today_count = 0;
    int upcoming_count = 0;
    int completed_count = 0;
    
    printf("\n════════════════════════════════════════\n");
    printf("📅 ПЛАНИРУЕМЫЕ СОБЫТИЯ:\n");
    printf("════════════════════════════════════════\n\n");
    
    while (event) {
        char* title = get_string_field(event, "title");
        char* date = get_string_field(event, "date");
        bool completed = get_bool_field(event, "completed");
        
				// если завершено, ивент сегодня или еще не начался - говорим
        if (completed) {
            printf("✅ %s | %s\n", date, title);
            completed_count++;
        } else if (date) {
            if (is_date_today_or_earler(date)) {
                printf("🔔 %s | %s\n", date, title);
                today_count++;
            } else {
                printf("⏳ %s | %s\n", date, title);
                upcoming_count++;
            }
        }
        event = event->next;
    }
    
    if (today_count == 0 && upcoming_count == 0 && completed_count == 0) {
        printf("Нет запланированных событий\n");
    }
    
    printf("\nАктивных: %d | Завершённых: %d | Впереди: %d\n", 
           today_count, completed_count, upcoming_count);
    printf("════════════════════════════════════════\n\n");
}

void handle_complete(GameWorld* gw, const char* title){
	cJSON* events = cJSON_GetObjectItem(gw->events, "custom_events");
	if(!events){
		printf("Нет событий в принципе\n");
		return;
	}

	cJSON* event = events->child;
	while(event){
		char* event_title = get_string_field(event, "title");
		bool completed = get_bool_field(event, "completed");
		
		// если не завершено и все ок, то завершаем
		if(!completed && event_title && strcmp(event_title, title) == 0){
			// Завершаем событие
      cJSON_ReplaceItemInObject(event, "completed", cJSON_CreateBool(1));
            
      // Логируем
      char log_msg[256];
      sprintf(log_msg, "Завершил событие: %s", title);
      log_text_in_file("СОБЫТИЕ", log_msg);
            
      printf("✅ Событие завершено: %s\n", title);
      return;
		}
		event = event->next;
	}
	printf("Событие не найдено: %s\n", title);

}

// обработчик -с
void handle_push_c(GameWorld* gw, const char* title, const char* date_str){
	if(!gw || !title || !date_str){
		return;
	}

	if(strlen(date_str) != 10){
		printf("Неверный формат даты (ожидается ГГГГ-ММ-ДД)\n");
		return;
	}

	if (date_str[4] != '-' || date_str[7] != '-') {
    printf("❌ Неверный формат даты (ожидается ГГГГ-ММ-ДД)\n");
    return;
  }
	
	// создаем ивент
	// новый объект
	cJSON* event = cJSON_CreateObject();
	// поля нового объекта
	cJSON_AddStringToObject(event, "title", title);
  cJSON_AddStringToObject(event, "date", date_str);
  cJSON_AddBoolToObject(event, "completed", 0);
  cJSON_AddNumberToObject(event, "duration_days", 3);
	
	// засовываем в массив
	cJSON* events_array = cJSON_GetObjectItem(gw->events, "custom_events");
  if (!events_array) {
    events_array = cJSON_CreateArray();
    cJSON_AddItemToObject(gw->events, "custom_events", events_array);
  }
  cJSON_AddItemToArray(events_array, event);


	// логируем
	char log_msg[256];
	sprintf(log_msg, "Запланировал событие: %s", title);
	log_text_in_file("СОБЫТИЕ", log_msg);


	printf("📅 Событие запланировано: %s (%s)\n", title, date_str);
}


// сохранение ивентов
void save_events(GameWorld* gw){
	if(!gw->events) return;

	char* json_str = cJSON_Print(gw->events);
	if (!json_str) {
		printf("❌ Ошибка создания JSON-строки для событий\n");
    return;
  }

	FILE* f = fopen("events.json", "w");
	if(f){
		fprintf(f, "%s", json_str);
		fclose(f);
	}
	free(json_str);

}

// загрузка ивентов пользователя
void load_events(GameWorld* gw){
	FILE* fp = fopen("events.json", "r");
	// если не открыли, создаем
	if(!fp){
		gw->events = cJSON_CreateObject();
		cJSON_AddItemToObject(gw->events, "custom_events", cJSON_CreateArray());
		// сохранение ивентов
		save_events(gw);
		return;
	}
	
	// ставим курсор в конец файла, читаем сколько байтов весит, возвращаем обратно
	fseek(fp, 0, SEEK_END);
	long size = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	
	// создаем буфер на размер_файла+1 и записываем в него содержимое файла
	char* buffer = malloc(size + 1);
	fread(buffer, 1, size, fp);
	buffer[size] = '\0';
	fclose(fp);

	// записываем все в формат cJSON
	gw->events = cJSON_Parse(buffer);
	free(buffer);

	if(!gw->events){
		gw->events = cJSON_CreateObject();
		cJSON_AddItemToObject(gw->events, "custom_events", cJSON_CreateArray());
	}

	cJSON* events_array = cJSON_GetObjectItem(gw->events, "custom_events");
	if(!events_array){
		cJSON_AddItemToObject(gw->events, "custom_events", cJSON_CreateArray());
	}
}


/// 2 ///

struct task* find_parent(struct task* world, const char* child_title){
	if(!world || !child_title){
		printf("Не удалось прочитать struct task* и title в find_parent");
		return NULL;
	}
	
	struct task* kingdom = world;
	// для удобства называем kingdom
	while(kingdom){
		// в теории след дитя должно быть городом
		struct task* town = kingdom->child;
		while(town){
			// тут та же теория город->дитя
			struct task* village = town->child;
			while(village){
				// проверяем имя дитя, если да возвращаем его родителя
				if(strcmp(village->title, child_title) == 0){
					return town;
				}
				// перемещаемся по городам
				village = village->next;
			}
			// а если теория не работает и дитя - город
			if(strcmp(town->title, child_title) == 0){
				return kingdom;
			}
			// перемещаемся по городам
			town = town->next;
		}
		// перемещаемся по странам
		kingdom = kingdom->next;
	}
	return NULL;
}


// функция времени
char* get_current_date() {
    time_t now = time(NULL);
    struct tm* local = localtime(&now);
    static char date[11];  // "YYYY-MM-DD\0" → 11 байт
    strftime(date, sizeof(date), "%Y-%m-%d", local);
    return date;
}


void log_text_in_file(char* text_push, char* title){
	int fd = open("history.log", O_WRONLY | O_CREAT | O_APPEND, 0644);
	if(fd == -1){
		printf("Не удалось открыть или создать файл history.log\n");
		perror("open");
		return;
	}
	char* time = get_current_date();
	if(!time){
		printf("Не удалось получить время в log_text_in_file\n");
		close(fd);
		return;
	}
	if(title){
		dprintf(fd, "%s | %s | %s \n", time, text_push, title);
	} else {
		dprintf(fd, "%s | %s \n", time, text_push);
	}
	close(fd);
}

// для этих функций сигна всегда одна - сам объект и его поле
bool get_bool_field(cJSON* obj, char* title_obj){
	cJSON* item = cJSON_GetObjectItem(obj, title_obj);
	if(item && cJSON_IsBool(item)){
		return cJSON_IsTrue(item);
	}
	else {
		return false;
	}
}

// одна из функций что возвращает значение int поля с проверкой
int get_int_field(cJSON* obj, char* title_obj){
	cJSON* item = cJSON_GetObjectItem(obj, title_obj);
	if(item && cJSON_IsNumber(item)){
		return (int)item->valuedouble;
	}
	else{
		//printf("Не удалось получить поле элемента %s\n", title_obj);
		return -1;
	}
}

// одна из функций что возвращает значение string поля с проверкой
char* get_string_field(cJSON* obj, char* title_obj){
	cJSON* item = cJSON_GetObjectItem(obj, title_obj);
	if(item && cJSON_IsString(item)){
		return item->valuestring;
	}
	else{
		//printf("Не удалось получить поле элемента %s\n", title_obj);
		return NULL;
	}
}

// функция мятежа
bool schedule_rebellion(cJSON* obj, struct task* node){
	if(!obj || !node){
		printf("Не получилось получить obj и node в start_rebellion\n");
		return false;
	}
	
	// проверяем есть ли уже мятеж
	cJSON* data_reb = cJSON_GetObjectItem(obj, "data_rebellion");
	if(data_reb && strlen(data_reb->valuestring) > 0){
		return false;
	}
	
	// вычисляем время: сейчас, через сколько дней, окончательная дата
	time_t now = time(NULL);
	int days = (node->depth==2) ? (3 + rand() % 5) : (5 + rand() % 6);
	time_t rebellion_time = now + days * 24 * 3600;

	char date_str[11];
	strftime(date_str, sizeof(date_str), "%Y-%m-%d", localtime(&rebellion_time));
	
	// добавляем timestamp время - время в секундах, которое отсчитывается с 1 января 1970 года + наши дни 
	cJSON_AddNumberToObject(obj, "rebellion_start_timestamp", (double)rebellion_time);

	// добавляем отдельные поля
	cJSON_ReplaceItemInObject(obj, "date_rebellion", cJSON_CreateString(date_str));
	cJSON_AddNumberToObject(obj, "rebellion_pushes_needed", 2 + rand() % 3);
	cJSON_AddBoolToObject(obj, "is_in_rebellion", 1);
	cJSON_AddNumberToObject(obj, "rebellion_pushes_done", 0);

	return true;
}


int8_t is_date_today_or_earler(const char* date_versus) {
    if (!date_versus) return -2;

    char* today = get_current_date();
    //strftime(today, sizeof(today), "%Y-%m-%d", localtime(time(NULL)));

    // cравниваем как строки: "2026-01-30" <= "2026-02-01" → true
    int cmp = strcmp(date_versus, today);
    if (cmp <= 0) {
        return 1; // сегодня или раньше
    }
    return 0; // будущее
}

// отнимаем обьект у игрока, если не остановил мятеж
void reset_object_to_not_captured(GameWorld* gw, cJSON* obj_json, const char* title){
		// обнуляем статы обьекта
		cJSON_ReplaceItemInObject(obj_json, "status", cJSON_CreateString("not_captured"));
    cJSON_ReplaceItemInObject(obj_json, "is_in_rebellion", cJSON_CreateBool(0));
    cJSON_ReplaceItemInObject(obj_json, "date_rebellion", cJSON_CreateString(""));
    cJSON_ReplaceItemInObject(obj_json, "rebellion_start_timestamp", cJSON_CreateNumber(0));
    cJSON_ReplaceItemInObject(obj_json, "prep_points", cJSON_CreateNumber(0));
    cJSON_ReplaceItemInObject(obj_json, "xp", cJSON_CreateNumber(0));
    cJSON_ReplaceItemInObject(obj_json, "level", cJSON_CreateNumber(0));

		struct task* node = find_by_title(gw->world, title);
		if(!node){
			printf("Обьект %s не найден\n", title);
			return;
		}	

		// находим родителя обьекта
		struct task* parent = find_parent(gw->world, title);
		if(!parent){
			printf("У обьекта %s нет родителя(обьект страна?)\n", title);
			return;
		}


		cJSON* territories = cJSON_GetObjectItem(gw->progress, "territories");
		if(!territories){
			printf("Не найдено поле territories в JSON\n");
			return;
		}
		cJSON* parent_json = cJSON_GetObjectItem(gw->progress, parent->title);
		if(!territories){
			printf("Не найден родитель в JSON\n");
			return;
		}
		
		// если деревня то уменьшаем счетчик у родителя(города)
		if(node->depth == 2){
			int current = get_int_field(parent_json, "captured_villages");
			if (current > 0) {
        cJSON_ReplaceItemInObject(parent_json, "captured_villages", cJSON_CreateNumber(current - 1));
      }
		}
		else if(node->depth == 1){
			int current = get_int_field(parent_json, "captured_towns");
			if (current > 0) {
				cJSON_ReplaceItemInObject(parent_json, "captured_towns", cJSON_CreateNumber(current - 1));
			}

			int current_villages = get_int_field(parent_json, "total_captured_villages");
			if(current_villages > 0){
				cJSON_ReplaceItemInObject(parent_json, "total_captured_villages", cJSON_CreateNumber(current - 1));
			}
		}
	
	printf("Обьект %s был потерян из-за бездействия!!!!!!!!\n", title);
}

// газетчик или новости, смотрит у каких обьектов скоро будет мятеж или уже есть
void fresh_news(GameWorld* gw){
	  cJSON* territories = cJSON_GetObjectItem(gw->progress, "territories");
    if (!territories) return;
		
		// новая техника - получаем объекты parent->child
		cJSON* obj =  territories->child;
		while(obj){
			if(get_bool_field(obj, "is_in_rebellion")){
				double start_ts = get_int_field(obj, "rebellion_start_timestamp");
				if(start_ts > 0){
					// получаем нынешние секунды
					time_t now = time(NULL);
					// отсчитываем сколько прошло
					int days_passed = (int)(now - (time_t)start_ts) / (24 * 3600);

					// если больше 3 дней, то отнимаем город
					if(days_passed > 3){
						reset_object_to_not_captured(gw, obj, obj->string /*поле имени объекта*/);
					}
					else {
						printf("⚠️ Мятеж в %s (%d/3 дней)\n", obj->string, days_passed);
					}
				}
			}
			// перемещаемся по объектам
			obj = obj->next;
		}
}

// находится ли объект в нашем королевстве
bool is_object_in_kingdom(GameWorld* gw, const char* title, const char* kingdom_title){
	if(!gw || !title || !kingdom_title){
		printf("Невалидные указатели аргументы в  is_object_in_kingdom\n");
		return false;
	}

	// Если сам объект — королевство
	if (strcmp(title, kingdom_title) == 0) {
    return false; // королевство не может быть в своём же мятеже
	}

	struct task* parent = find_parent(gw->world, title);
	if(!parent){
		return false;
	}
	if(parent->depth == 1){
		struct task* kingdom = find_parent(gw->world, parent->title);
		if(!kingdom){
			return false;
		}
		if(strcmp(kingdom->title, kingdom_title) == 0){
			return true;
		}
	}
	else {
		if(strcmp(parent->title, kingdom_title) == 0){
			return true;
		}
	}

	return false;

}

// функция отмены всех мятежей
void cancel_all_regular_rebellions(GameWorld* gw, const char* kingdom_title){
	cJSON* ter = cJSON_GetObjectItem(gw->progress, "territories");
	cJSON* obj = ter->child;

	while(obj){
		// пропускаем королевства
		char* view = get_string_field(obj, "view");
    if (view && strcmp(view, "KINGDOM") == 0) {
      obj = obj->next;
      continue;
    }

		if(is_object_in_kingdom(gw, obj->string, kingdom_title)){
			cJSON_ReplaceItemInObject(obj, "is_in_rebellion", cJSON_CreateBool(0));
      cJSON_ReplaceItemInObject(obj, "date_rebellion", cJSON_CreateString(""));
      cJSON_ReplaceItemInObject(obj, "rebellion_start_timestamp", cJSON_CreateNumber(0));
		}
		obj = obj->next;
	}
}

// активация мятежа с отсрочкой или без
void activate_rebellion_at(cJSON* obj, time_t when) {
    char date_str[11];
    strftime(date_str, sizeof(date_str), "%Y-%m-%d", localtime(&when));
    
    cJSON_ReplaceItemInObject(obj, "is_in_rebellion", cJSON_CreateBool(1));
    cJSON_ReplaceItemInObject(obj, "date_rebellion", cJSON_CreateString(date_str));
    cJSON_ReplaceItemInObject(obj, "rebellion_start_timestamp", cJSON_CreateNumber((double)when));
    cJSON_ReplaceItemInObject(obj, "rebellion_pushes_needed", cJSON_CreateNumber(3 + rand() % 3));
}


void trigger_multiple_rebellion(GameWorld* gw, struct task* kingdom){
	// отменяем все обычные мятежи в стране
	cancel_all_regular_rebellions(gw, kingdom->title);
	
	// теперь 10-ым городам объявят мятежи 
	
	// считаем города
	size_t count_towns = element_length(kingdom);
	if(count_towns <= 0){
		printf("Не удалось посчитать количество городов\n");
		return;
	}

	// сколько городов будет бунтовать (максимум 10)
	int count_rebellion = (count_towns < 10) ? count_towns : 10;
	
	// выбираем рандом индексы
	bool arr_idx[count_towns];
	memset(arr_idx, 0, sizeof(arr_idx));	

	size_t activated = 0;
	while(activated < count_rebellion){
		int idx = rand() % count_towns;
		if(!arr_idx[idx]){
			arr_idx[idx] = true;
			activated++;
		}
	}

	// активируем мятежи
	struct task* town = kingdom->child;
	for(size_t i = 0; i < count_towns && town; i++){
		if(arr_idx[i]){
			cJSON* territories = cJSON_GetObjectItem(gw->progress, "territories");
      cJSON* town_obj = cJSON_GetObjectItem(territories, town->title);
      if (town_obj) {
				// активирует мятеж сразу с отсрочкой,
				int days = 5; 
				time_t when = time(NULL) + days * 24 * 60 * 60;
        activate_rebellion_at(town_obj, when);
      }
		}
		town = town->next;
	}

	// обновляем статус королевства
	cJSON* territories = cJSON_GetObjectItem(gw->progress, "territories");
  cJSON* kingdom_obj = cJSON_GetObjectItem(territories, kingdom->title);
  if (kingdom_obj) {
    cJSON_ReplaceItemInObject(kingdom_obj, "multiple_rebellion_kingdom", cJSON_CreateBool(1));
    printf("🔥 Множественный бунт в %s (%d городов)!\n", kingdom->title, count_rebellion);
  }
}


bool can_capture_node(GameWorld* gw, char* title){
    if(!gw || !title){
        printf("Невалидные указатели gw и title в can_capture_node\n");
        return false;
    }
    
    struct task* node = find_by_title(gw->world, title);
    if(!node) return false;
    
    cJSON* territories = cJSON_GetObjectItem(gw->progress, "territories");
    if(!territories) return false;
    
    cJSON* obj = cJSON_GetObjectItem(territories, title);
    if(!obj) return false;
    
    // Проверяем по глубине
    if(node->depth == 0) {
        // Королевство: все регионы захвачены
        int all_count = get_int_field(obj, "all_count_town");
        int captured_count = get_int_field(obj, "captured_towns");
        return (captured_count >= all_count);
    }
    else if(node->depth == 1) {
        // Регион: все подрегионы захвачены
        int all_count = get_int_field(obj, "all_count_subregions");
        int captured_count = get_int_field(obj, "captured_subregions");
        return (captured_count >= all_count);
    }
    else if(node->depth == 2) {
        // Город: все районы захвачены
        int all_count = get_int_field(obj, "all_count_village");
        int captured_count = get_int_field(obj, "captured_villages");
        return (captured_count >= all_count);
    }
    // depth 3 и 4 всегда можно захватывать
    return true;
}

// обработчик для событий когда статус == "not captured" 
void handle_prep(cJSON* obj, GameWorld* gw, char* title){
    if(!obj){
        printf("Не удалось получить обьект в handle_prep\n");
        return;
    }
    
    int prep_scores = get_int_field(obj, "prep_points");
    if(prep_scores == -1){
        return;
    }
    
    int count_scores = get_int_field(obj, "count_scores");
    if(count_scores == -1 || count_scores == 0){
        return;
    }

    // проверяем можно ли в принципе захватывать объект
    if(!can_capture_node(gw, title)){
        printf("Нельзя захватывать %s, пока не захвачены объекты грейдом ниже\n", title);
        return;
    }
    
    prep_scores++;
    
    // если захватил объект
    if(prep_scores >= count_scores){
        // финальная проверка перед захватом
        if(!can_capture_node(gw, title)){
            printf("Нельзя захватывать %s, пока не захвачены объекты грейдом ниже\n", title);
            return;
        }

        cJSON_ReplaceItemInObject(obj, "status", cJSON_CreateString("captured"));
                            
        // старт мятежа через n-дней
        struct task* object_node = find_by_title(gw->world, title);
        if(schedule_rebellion(obj, object_node)){
            printf("Скоро начнется мятеж в %s!\n", title);
        }

        // находим родителя для обновления статистики
        struct task* parent = find_parent(gw->world, title);
        if(!parent){
            printf("⚠️ У объекта %s нет родителя (возможно, это королевство)\n", title);
            return;
        }
        
        cJSON* territories = cJSON_GetObjectItem(gw->progress, "territories");
        if(!territories){
            printf("Не удалось взять поле territories в handle_prep\n");
            return;
        }
        
        cJSON* parent_item = cJSON_GetObjectItem(territories, parent->title);
        if(!parent_item){
            printf("Не удалось найти родителя %s в JSON\n", parent->title);
            return;
        }
        
        // Отладочный вывод
        printf("🔍 Обновление родителя:\n");
        printf("  - Ребёнок: '%s' (depth=%d)\n", title, object_node->depth);
        printf("  - Родитель: '%s' (depth=%d)\n", parent->title, parent->depth);
        
        // Обновляем статистику родителя в зависимости от глубины
        // Родитель на уровне 3 (Район) → увеличиваем у города (depth=2)
        if(parent->depth == 3){
            int captured = get_int_field(parent_item, "captured_districts");
            if(captured == -1){
                printf("Не удалось получить captured_districts\n");
                return;
            }
            captured++;
            cJSON_ReplaceItemInObject(parent_item, "captured_districts", cJSON_CreateNumber(captured));
            
            printf("✅ Захвачен район '%s', обновлён город '%s' (%d/?)\n", 
                   title, parent->title, captured);
        }
        // Родитель на уровне 2 (Город) → увеличиваем у региона (depth=1)
        else if(parent->depth == 2){
            int captured = get_int_field(parent_item, "captured_towns");
            if(captured == -1){
                printf("Не удалось получить captured_towns\n");
                return;
            }
            captured++;
            cJSON_ReplaceItemInObject(parent_item, "captured_towns", cJSON_CreateNumber(captured));
            
            // Также увеличиваем общее количество захваченных районов
            struct task* region = find_parent(gw->world, parent->title);
            if (region && region->depth == 1) {
                cJSON* region_item = cJSON_GetObjectItem(territories, region->title);
                if (region_item) {
                    int total = get_int_field(region_item, "total_captured_districts");
                    if (total != -1) {
                        cJSON_ReplaceItemInObject(region_item, "total_captured_districts", 
                                                  cJSON_CreateNumber(total + 1));
                    }
                }
            }
            
            printf("✅ Захвачен город '%s', обновлён регион '%s' (%d/?)\n", 
                   title, parent->title, captured);
        }
        // Родитель на уровне 1 (Регион) → увеличиваем у королевства (depth=0)
        else if(parent->depth == 1){
            int captured = get_int_field(parent_item, "captured_regions");
            if(captured == -1){
                printf("Не удалось получить captured_regions\n");
                return;
            }
            captured++;
            cJSON_ReplaceItemInObject(parent_item, "captured_regions", cJSON_CreateNumber(captured));
            
            // Также увеличиваем общее количество захваченных городов
            struct task* kingdom = find_parent(gw->world, parent->title);
            if (kingdom && kingdom->depth == 0) {
                cJSON* kingdom_item = cJSON_GetObjectItem(territories, kingdom->title);
                if (kingdom_item) {
                    int total = get_int_field(kingdom_item, "total_captured_towns");
                    if (total != -1) {
                        cJSON_ReplaceItemInObject(kingdom_item, "total_captured_towns", 
                                                  cJSON_CreateNumber(total + 1));
                    }
                }
            }
            
            printf("✅ Захвачен регион '%s', обновлено королевство '%s' (%d/?)\n", 
                   title, parent->title, captured);
        }
        // Родитель на уровне 0 (Королевство) → множественный бунт при полном захвате
        else if(parent->depth == 0){
            int captured = get_int_field(parent_item, "captured_kingdoms");
            if(captured == -1){
                printf("Не удалось получить captured_kingdoms\n");
                return;
            }
            captured++;
            cJSON_ReplaceItemInObject(parent_item, "captured_kingdoms", cJSON_CreateNumber(captured));
            
            // если захватил все регионы, то множественный бунт
            int total = get_int_field(parent_item, "all_count_regions");
            if(total == -1){
                printf("Не удалось получить all_count_regions\n");
                return;
            }
            
            if(captured == total){
                printf("🔥 Королевство %s полностью захвачено!\n", parent->title);
                trigger_multiple_rebellion(gw, parent);
            }
            
            printf("✅ Захвачено королевство '%s' (%d/%d)\n", 
                   title, captured, total);
        }
    }
    // если еще не захватил
    else {
        cJSON_ReplaceItemInObject(obj, "prep_points", cJSON_CreateNumber(prep_scores));
        printf("📊 Прогресс захвата %s: %d/%d\n", title, prep_scores, count_scores);
    }
}


// обработчик событий когда у объекта мятеж
void handle_rebellion(cJSON* obj){
	if(!obj){
		printf("Не удалось получить обьект в handle_rebellion\n");
		return;
	}
	int pushes_needed = get_int_field(obj, "rebellion_pushes_needed");
	if(pushes_needed == -1){
		return;
	}
	int pushes_done =  get_int_field(obj, "rebellion_pushes_done");
	if(pushes_done == -1){
		pushes_done = 0;
	}

	pushes_done++;
	if(pushes_done >= pushes_needed){
		printf("Вы смогли успокоить народ\n");
		cJSON_ReplaceItemInObject(obj, "rebellion_pushes_needed", cJSON_CreateNumber(0));
		cJSON_ReplaceItemInObject(obj, "rebellion_pushes_done", cJSON_CreateNumber(0));
		cJSON_ReplaceItemInObject(obj, "is_in_rebellion", cJSON_CreateBool(0));
		cJSON_ReplaceItemInObject(obj, "date_rebellion", cJSON_CreateString(""));
	} else {
		printf("Мятеж подавляется: %d/%d\n", pushes_done, pushes_needed);
		cJSON_ReplaceItemInObject(obj, "rebellion_pushes_done", cJSON_CreateNumber(pushes_done));
	}
}

// обработчик для событий когда статус == "captured"
void handle_xp(cJSON* obj){
	if(!obj){
		printf("Не удалось получить обьект в handle_xp\n");
		return;
	}

	int xp = get_int_field(obj, "xp");
	//int level = get_int_field(obj, "level");

	xp++;
	int new_level = xp / 5;
	cJSON_ReplaceItemInObject(obj, "xp", cJSON_CreateNumber(xp));
	cJSON_ReplaceItemInObject(obj, "level", cJSON_CreateNumber(new_level));
	if(new_level == 1){
		cJSON_ReplaceItemInObject(obj, "status", cJSON_CreateString("researcher"));
	}
	else if(new_level == 2){
		cJSON_ReplaceItemInObject(obj, "status", cJSON_CreateString("expert"));
	}
	else if(new_level == 3){
		cJSON_ReplaceItemInObject(obj, "status", cJSON_CreateString("master"));
	}
	else if(new_level == 4){
		cJSON_ReplaceItemInObject(obj, "status", cJSON_CreateString("arcmage"));
	}
	else if(new_level >= 5){
		cJSON_ReplaceItemInObject(obj, "status", cJSON_CreateString("teacher"));
	}
}

// главная функция обработчик push -t
void handle_push_t(GameWorld* gw, char* title){
	// достаем одну большую структуру территории
	cJSON* territories = cJSON_GetObjectItem(gw->progress, "territories");
	if(!territories){
		printf("Не удалось записать территорию\n");
		return;
	}
	// достаем обьект dn
	cJSON* obj = cJSON_GetObjectItem(territories, title);
	if(!obj){
		printf("Не удалось найти элемент в json структуре\n");
		return;
	}

	// достаем статус - именно от него и зависит условие, что увеличивать
	char* status_item = get_string_field(obj, "status");
	if(!status_item){
		return;
	}
	
	bool is_rebellion = get_bool_field(obj, "is_in_rebellion");
	// если у объекта мятеж увеличиваем кол-во пуша для успокоения
	if(is_rebellion){
		// проверяем дату мятежа, если сегодня, то handle_rebellion(obj);, если нет то xp
		char* rebellion_date = get_string_field(obj, "date_rebellion");
		int8_t st = is_date_today_or_earler(rebellion_date);
		if(st == 1){
			handle_rebellion(obj);
		}
		else {
			handle_xp(obj);
		}

	}
	// если "не захват" увеличиваем очки захвата, когда будет ==, обьект обязательно захватился
	else if(strcmp(status_item, "not_captured") == 0){
		handle_prep(obj, gw, title); // готова, не обработаны ошибки
	}
	// если "захват" увеличиваем очки опыта, когда будет достигнут определенный уровень, меняем статус




	else{
		handle_xp(obj); // готова, не обработаны ошибки
	}

}

void handle_push(GameWorld* gw, char* flag, char* text_push, char* title){
    if (!flag) {
        // Обычный пуш без флага
        if (text_push) {
            log_text_in_file(text_push, title);
            add_kingdom_xp(gw, 1);
        }
        return;
    }

    // Флаги с параметрами
    if (strcmp(flag, "-t") == 0) {
        if (!title) {
            printf("Для флага -t нужно указать территорию\n");
            return;
        }
        handle_push_t(gw, title);
        add_kingdom_xp(gw, 1);
        printf("Сработала функция для -t\n");
    }
    else if (strcmp(flag, "-c") == 0) {
        if (!title) {
            printf("Для флага -c нужно указать дату [ГГГГ-ММ-ДД]\n");
            return;
        }
        printf("Сработала заглушка для -c\n");
        handle_push_c(gw, text_push, title);
        add_kingdom_xp(gw, 1);
    }
    else if (strcmp(flag, "-s") == 0) {
        if (!title) {
            printf("Для флага -s нужно указать стихию\n");
            return;
        }
        add_element_xp(gw, title);
        add_kingdom_xp(gw, 1);
    }
    else if (strcmp(flag, "-fc") == 0) {
        printf("Метка создания оружия\n");
        craft_weapon(gw);
				add_kingdom_xp(gw, 1);
    }
    else if (strcmp(flag, "-fu") == 0) {
        if (!title) {
            printf("Для флага -fu нужно указать территорию\n");
            return;
        }
        use_equipment(gw, title);
    }
    else {
        printf("Неизвестный флаг: %s\n", flag);
        printf("Доступные флаги:\n");
        printf("  -t    захват территории\n");
        printf("  -c    создание события\n");
        printf("  -s    прокачка стихии\n");
        printf("  -fc   создание оружия\n");
        printf("  -fu   использование оружия\n");
    }
}


//функция нахождения элемента по имени через рекурсию
struct task* find_by_title(struct task* node, const char* title) {
    if (!node) return NULL;
    if (strcmp(node->title, title) == 0) {
        return node;
    }
    // Ищем в детях
    struct task* child_result = find_by_title(node->child, title);
    if (child_result) return child_result;
    // Ищем в соседях
    return find_by_title(node->next, title);
}

/// 1 ///

//расчет колво элементов
size_t element_length(const struct task* element){
        size_t count = 0;
        while(element != NULL){
                count++;
                element = element->next;
        }
        return count;
}

// уничтожение рекурсией
void element_destroy(struct task* element){
        if(element == NULL) return;

        element_destroy(element->child);
        element_destroy(element->next);

        free(element->title);
        free(element);
}


// нахождение последнего элемента
struct task* element_last(struct task* element){
        if(element == NULL){
                return NULL;
        }

        while(element != NULL){
                if(element->next == NULL){
                        return element;
                }
                element = element->next;
        }
				return NULL;
}


// вспомогательная функция для записи из файла
struct task* create_node_from_title(const char* title) {
    if (!title) return NULL;
    struct task* node = malloc(sizeof(struct task));
    if (!node) return NULL;

    size_t len = strlen(title);

    node->title = malloc(len + 1);
    if (!node->title) {
        free(node);
        return NULL;
    }
    strcpy(node->title, title);

    node->child = NULL;
    node->next = NULL;
    return node;
}

// записывает с файла и создает списки
struct task* load_from_file(char* filename) {
    //сохраняет указатель на файл
    FILE* fp = fopen(filename, "r");
    if (!fp) return NULL;

    //подгатавливаем мини стек
    #define MAX_DEPTH 20
    struct task* stack[MAX_DEPTH] = {NULL};

    //создаем корень списка
    struct task* root = NULL;

    //читаем файл построчно - максимум 512 байт
    char line[512];

    // читаем файл
    while (fgets(line, sizeof(line), fp)) {
        //меняет символы переноса, на завершающую
        line[strcspn(line, "\n")] = '\0';
        //если символ завершающий, то continue
        if (line[0] == '\0') continue;

        // Считаем отступы (4 пробела = 1 уровень)
        int spaces = 0;
        while (line[spaces] == ' ') spaces++;
        //узнаем глубину, через пробелы
        int depth = spaces / 4;
        if (depth >= MAX_DEPTH) continue;
				
        //подгатавливаем строку
        char* title = line + spaces;
        //создаем элемент, через аналогичную функцию
        struct task* node = create_node_from_title(title);
        if (!node) continue;
				node->depth = depth;
        // если глубина 0 - корень
        if (depth == 0) {
            // если root == NULL, указываем на первый элемент, заносим в стек
            if (!root) {
                root = node;
                stack[0] = node;
            } else {
                struct task* last = root;
                while (last->next) last = last->next;
                last->next = node;
                stack[0] = node;
            }
        } else {
            struct task* parent = stack[depth - 1];
            if (!parent) {
                element_destroy(node); // некорректная строка
                continue;
            }
            if (!parent->child) {
                parent->child = node;
            } else {
                struct task* last_child = element_last(parent->child);
                last_child->next = node;
            }
            stack[depth] = node;
        }
    }

    fclose(fp);
    return root;
}


char* read_file(const char* filename){
	if(!filename){
		printf("Имя файла не найдено\n");
		return NULL;
	}

	FILE* fp = fopen(filename, "rb");
	if(!fp){
		printf("Файл не найден или не удалось открыть файл\n");
		return NULL;
	}

	// Узнаём размер файла
  fseek(fp, 0, SEEK_END);
  size_t size = ftell(fp);
  rewind(fp);  // возвращаемся в начало
	
	// выделяем буфер для записи
	char* buffer = (char*)malloc(size+1);
	if(!buffer){
		printf("Не выделен буфер для записи\n");
		fclose(fp);
		return NULL;
	}

	// Читаем всё содержимое
	size_t bytes_read = fread(buffer, 1, size, fp);
  fclose(fp);

	// если колво символов меньше чем задано, то отмена
	if(bytes_read != size){
		printf("Ошибка записи\n");
		free(buffer);
		return NULL;
	}

	buffer[size] = '\0';
	return buffer;
}

void sync_node(struct task* node, cJSON* territories) {
    if (!node) return;
    
    cJSON* existing = cJSON_GetObjectItem(territories, node->title);
    if (!existing) {
        cJSON* obj = cJSON_CreateObject();
        
        // Базовые поля для всех
        cJSON_AddStringToObject(obj, "status", "not_captured");
        cJSON_AddStringToObject(obj, "date_captured", "");
        cJSON_AddStringToObject(obj, "date_rebellion", "");
        cJSON_AddNumberToObject(obj, "time_captured", 0);
        cJSON_AddNumberToObject(obj, "xp", 0);
        cJSON_AddNumberToObject(obj, "level", 0);
        cJSON_AddNumberToObject(obj, "prep_points", 0);
        
        // Установка сложности по глубине
        if (node->depth == 0) {
            // Королевство - самый сложный
            cJSON_AddStringToObject(obj, "view", "KINGDOM");
            cJSON_AddNumberToObject(obj, "count_scores", 25); // ×2.5 от базового
            cJSON_AddNumberToObject(obj, "all_count_town", element_length(node->child));
            cJSON_AddNumberToObject(obj, "captured_towns", 0);
            cJSON_AddNumberToObject(obj, "total_captured_villages", 0);
            cJSON_AddBoolToObject(obj, "multiple_rebellion_kingdom", 0);
        } 
        else if (node->depth == 1) {
            // Регион/Ядро
            cJSON_AddStringToObject(obj, "view", "REGION");
            cJSON_AddNumberToObject(obj, "count_scores", 20); // ×2 от базового
            cJSON_AddNumberToObject(obj, "all_count_subregions", element_length(node->child));
            cJSON_AddNumberToObject(obj, "captured_subregions", 0);
        }
        else if (node->depth == 2) {
            // Город/Системные вызовы
            cJSON_AddStringToObject(obj, "view", "TOWN");
            cJSON_AddNumberToObject(obj, "count_scores", 15); // ×1.5 от базового
            cJSON_AddNumberToObject(obj, "all_count_village", element_length(node->child));
            cJSON_AddNumberToObject(obj, "captured_villages", 0);
        }
        else if (node->depth == 3) {
            // Район/Категория
            cJSON_AddStringToObject(obj, "view", "DISTRICT");
            cJSON_AddNumberToObject(obj, "count_scores", 10); // базовый
        }
        else if (node->depth >= 4) {
            // Улица/Конкретная тема
            cJSON_AddStringToObject(obj, "view", "VILLAGE");
            cJSON_AddNumberToObject(obj, "count_scores", 5); // самый простой
        }
        
        cJSON_AddItemToObject(territories, node->title, obj);
        printf("Добавлен: %s (depth=%d)\n", node->title, node->depth);
    }
    
    sync_node(node->child, territories);
    sync_node(node->next, territories);
}

GameWorld* load_game_state(){
	GameWorld* all_world = (GameWorld*)malloc(sizeof(GameWorld));
	if(!all_world){
		printf("Не удалось выделить память под структуру GameWorld\n");
		return NULL;
	}
	all_world->world = load_from_file("world.km");
	if(!all_world->world){
		printf("Не удалось загрузить world.km\n");
		return NULL;
	}
	char* json_text = read_file("progress.json");
	all_world->progress = NULL;

	if(json_text){
		all_world->progress = cJSON_Parse(json_text);
		free(json_text);
	}
	
	if(!all_world->progress){
		all_world->progress = cJSON_CreateObject();
		cJSON_AddItemToObject(all_world->progress, "territories", cJSON_CreateObject());
	}

	cJSON* territories = cJSON_GetObjectItem(all_world->progress, "territories");
	if(!territories){
		territories = cJSON_CreateObject();
		cJSON_AddItemToObject(all_world->progress, "territories", territories);
	}
	sync_node(all_world->world, territories);

  return all_world;
}

// Только сохранение, без освобождения
void save_game(GameWorld* gw){
    char* output = cJSON_Print(gw->progress);
    if(output){
        FILE* fp = fopen("progress.json", "w");
        if(fp){
            fwrite(output, 1, strlen(output), fp);
            fclose(fp);
        }
        free(output);
    }
    // ← УБРАТЬ освобождение памяти отсюда!
}

// Отдельная функция для освобождения
void cleanup_game(GameWorld* gw){
    if (!gw) return;
    cJSON_Delete(gw->progress);
    element_destroy(gw->world);
    free(gw);
}

int main(int argc, char* argv[]){
    srand(time(NULL));
    
    // Загрузка данных
    GameWorld* gw = load_game_state();
    if (!gw) {
        fprintf(stderr, "Ошибка загрузки мира\n");
        return 1;
    }
    
    load_events(gw);
    load_user(gw);
    load_library(gw);

    // ===== СПРАВКА =====
    if (argc == 2 && strcmp(argv[1], "--help") == 0) {
        show_help();
				cleanup_game(gw);
        return 0;
    }

    // ===== ИНИЦИАЛИЗАЦИЯ =====
    if (argc == 2 && strcmp(argv[1], "--init") == 0) {
        fresh_news(gw);
        check_for_custom_events(gw);
        save_user(gw);
        save_events(gw);
        save_library(gw);
				save_game(gw);
				cleanup_game(gw);
        return 0;
    }

    // ===== ПРОВЕРКА АРГУМЕНТОВ =====
    if (argc < 2) {
        printf("Введите команду. Используйте ./rpg --help для справки.\n");
        return 1;
    }

    // ===== ОБРАБОТКА КОМАНД =====
    
    // === ПУШИ ===
		if (strcmp(argv[1], "push") == 0) {
			if (argc == 4 && strcmp(argv[2], "complete") == 0) {
				// push complete "событие"
        handle_complete(gw, argv[3]);
        add_total_push(gw);
				log_text_in_file("Завершил событие\n", argv[3]);
			}
			else if (argc == 3) {
        // push "текст"  ИЛИ  push -fc
        if (strcmp(argv[2], "-fc") == 0) {
            handle_push(gw, argv[2], NULL, NULL);
            add_total_push(gw);
						log_text_in_file("Создал оружие\n", NULL);
        } else {
            // Обычный пуш
            handle_push(gw, NULL, argv[2], NULL);
            add_total_push(gw);
						log_text_in_file("Что то сделал\n", NULL);
        }
			}
			else if (argc == 4) {
        // push -s "стихия"  ИЛИ  push -fu "территория"
        handle_push(gw, argv[2], NULL, argv[3]);
        add_total_push(gw);
				log_text_in_file("Либо прокачал элемент либо использовал оружие на объект\n", NULL);
			}
			else if (argc == 5) {
        // push -t/-c "текст" "территория/дата"
        handle_push(gw, argv[2], argv[3], argv[4]);
        add_total_push(gw);
				log_text_in_file("Либо запушил объект либо добавил ивент\n", NULL);
			}
			else {
        printf("Неверное количество аргументов для push. Используйте ./rpg --help\n");
        return 1;
			}
		}
    // === БИБЛИОТЕКА ===
    else if (argc >= 2 && strcmp(argv[1], "library") == 0) {
				if (argc == 3 && strcmp(argv[2], "show") == 0) {
					// library show
					show_library(gw);
					log_text_in_file("Посмотрел статус БИБЛИОТЕКИ\n", NULL);
					return 0;
				}
				else if (argc >= 6 && strcmp(argv[2], "add") == 0) {
            // library add "название" --author "автор" --pages <число>
            char* title = argv[3];
            char* author = (argc >= 8 && strcmp(argv[4], "--author") == 0) ? argv[5] : "Неизвестен";
            int pages = (argc >= 10 && strcmp(argv[6], "--pages") == 0) ? atoi(argv[7]) : 100;
            add_book(gw, title, author, pages);
						log_text_in_file("Добавил книгу\n", NULL);
        }
        else if (argc >= 6 && strcmp(argv[2], "read") == 0) {
            // library read "название" --pages <число>
            char* title = argv[3];
            int pages = (argc >= 6 && strcmp(argv[4], "--pages") == 0) ? atoi(argv[5]) : 10;
            read_book(gw, title, pages);
						log_text_in_file("Немного прочитал книгу\n", NULL);
        }
        else if (argc >= 8 && strcmp(argv[2], "scroll") == 0) {
            // library scroll "название" --title "свиток" --content "текст"
            char* book_title = argv[3];
            char* scroll_title = (argc >= 6 && strcmp(argv[4], "--title") == 0) ? argv[5] : "Без названия";
            char* content = (argc >= 8 && strcmp(argv[6], "--content") == 0) ? argv[7] : "";
            create_scroll(gw, book_title, scroll_title, content);
						log_text_in_file("Сделал свиток\n", NULL);
        }
        else {
            printf("Неверная команда library. Используйте ./rpg --help\n");
            return 1;
        }
    }

    // === СТАТУСЫ ===
    else if (argc == 2) {
        if (strcmp(argv[1], "status") == 0) {
            show_status(gw);
						log_text_in_file("Посмотрел ОБЩИЙ СТАТУС \n", NULL);
            return 0;
        }
        else if (strcmp(argv[1], "kingdom") == 0) {
            show_kingdom_status(gw);
						log_text_in_file("Посмотрел статус КОРОЛЕВСТВА\n", NULL);
            return 0;
        }
        else if (strcmp(argv[1], "forge") == 0) {
            show_forge_status(gw);
						log_text_in_file("Посмотрел статус КУЗНИЦЫ\n", NULL);
            return 0;
        }
        else if (strcmp(argv[1], "elements") == 0) {
            show_elements_status(gw);
						log_text_in_file("Посмотрел статус ЭЛЕМЕНТОВ\n", NULL);
            return 0;
        }
        else if (strcmp(argv[1], "rebellions") == 0) {
            show_rebellions_status(gw);
						log_text_in_file("Посмотрел статус МЯТЕЖЕЙ\n", NULL);
            return 0;
        }
        else if (strcmp(argv[1], "events") == 0) {
            check_for_custom_events(gw);
						log_text_in_file("Посмотрел статус ИВЕНТОВ\n", NULL);
            return 0;
        }
        else {
            printf("Неизвестная команда: %s. Используйте ./rpg --help\n", argv[1]);
            return 1;
        }
    }


    // ===== СОХРАНЕНИЕ =====
    save_user(gw);
    save_events(gw);
    save_library(gw);
		save_game(gw);
		cleanup_game(gw);
    
    return 0;
}
