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
	cJSON* progress;
	cJSON* events;
} GameWorld;

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

	// достаю обьект (страна или город)
	struct task* node = find_by_title(gw->world, title);
	if(!node) return false;

	cJSON* territories = cJSON_GetObjectItem(gw->progress, "territories");
	if(!territories) return false;

	cJSON* obj = cJSON_GetObjectItem(territories, title);
	if(!obj) return false;

	// теперь проверяем 
	// если город, достаем статы деревень
	if(node->depth == 1){
		int all_count = get_int_field(obj, "all_count_village");
		int captured_count = get_int_field(obj, "captured_villages");
		return (captured_count >= all_count);
	} 
	// если страна, достаем статы городов
	else if(node->depth == 0){
		int all_count = get_int_field(obj, "all_count_town");
		int captured_count = get_int_field(obj, "captured_towns");
		return (captured_count >= all_count);
	}
	
	// если село то всегда можно
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
	// если захватил город
	if(prep_scores >= count_scores){

		// проверяем можно ли захватить объект(всегда смотрите что удаляете)
		if(!can_capture_node(gw, title)){
			printf("Нельзя захватывать %s, пока не захвачены объекты грейдом ниже\n", title);
			return;
		}

		cJSON_ReplaceItemInObject(obj, "status", cJSON_CreateString("captured"));
							
		// старт мятежа через n-дней
		struct task* object_node = find_by_title(gw->world, title);
		if(schedule_rebellion(obj, object_node)){
			printf("Скоро начнется мятеж\n");
		}

		struct task* parent = find_parent(gw->world, title);
		if(!parent){
			printf("Не удалось найти родителя\n");
			return;
		}
		cJSON* territories = cJSON_GetObjectItem(gw->progress, "territories");
		if(!territories){
			printf("Не удалось взять поле territories в handle_grep\n");
			return;
		}
		cJSON* parent_item = cJSON_GetObjectItem(territories, parent->title);
		if(!parent_item){
			printf("Не удалось найти родителя в json\n");
			return;
		}
		// После cJSON* parent_item = ...
		printf("🔍 Обновление родителя:\n");
		printf("  - Ребёнок: '%s'\n", title);
		printf("  - Родитель найден: '%s' (depth=%d)\n", parent->title, parent->depth);
		printf("  - Родитель в JSON: %s\n", parent_item ? "да" : "НЕТ!");

		// увеличиваем колво завоеванных для города
		if(parent->depth == 1){
			int captured = get_int_field(parent_item, "captured_villages");
			if(captured == -1){
				printf("Не удалось получить количество captured_villages\n");
				return;
			}
			captured++;
			cJSON_ReplaceItemInObject(parent_item, "captured_villages", cJSON_CreateNumber(captured));
			
			// также увеличиваемм общее колво захваченных деревней
			struct task* kingdom = find_parent(gw->world, parent->title);
			if (kingdom && kingdom->depth == 0) {
				cJSON* kingdom_item = cJSON_GetObjectItem(territories, kingdom->title);
				if (kingdom_item) {
					int total = get_int_field(kingdom_item, "total_captured_villages");
          if (total != -1) {
              cJSON_ReplaceItemInObject(kingdom_item, "total_captured_villages", 
              cJSON_CreateNumber(total + 1));
          }
				}
			}
		}
		// увеличиваем колво завоеванных для страны
		else if(parent->depth == 0){
			int captured = get_int_field(parent_item, "captured_towns");
			if(captured == -1){
				printf("Не удалось получить количество captured_villages\n");
				return;
			}
			captured++;
			cJSON_ReplaceItemInObject(parent_item, "captured_towns", cJSON_CreateNumber(captured));
			
			// если захватил все города, то множественный бунт(нужно также добавить условие для захвата -t)
			int total = get_int_field(parent_item, "all_count_town");
			if(total == -1){
				printf("Не удалось получить колво all_count_town\n");
				return;
			}
			if(captured == total){
				// страна захвачена
				printf("Сработала заглушка для множественного бунта\n");
				trigger_multiple_rebellion(gw, parent);
			}
		}
	}
	// если еще не захватил
	else {
		cJSON_ReplaceItemInObject(obj, "prep_points", cJSON_CreateNumber(prep_scores));
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
	if(!text_push){
		printf("Текст Пуша не найден\n");
	} 

	log_text_in_file(text_push, title);

	if(!flag){
		//ничего не делаем - обычный пуш
		return;	
	}

	else if(strcmp(flag, "-t") == 0){
		if(!title){
			printf("Для флага -t нужно указать город/cело\n");
			return;
		}
		//printf("Сработала заглушка для -t\n");
		handle_push_t(gw, title);
		printf("Сработала функция для -t\n");
	}
	else if(strcmp(flag, "-c") == 0){
		if(!title){
			printf("Для флага -c нужно указать дату события[dd-mm-YYYY]\n");
			return;
		}
		// обработчик пуша с -c
		printf("Сработала заглушка для -c\n");
		handle_push_c(gw, text_push, title);
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
				// статус города - захвачен и т.д
        cJSON_AddStringToObject(obj, "status", "not_captured");
				// дата захвата
        cJSON_AddStringToObject(obj, "date_captured", "");
				// дата мятежа 
        cJSON_AddStringToObject(obj, "date_rebellion", "");
				// время захвата 
        cJSON_AddNumberToObject(obj, "time_captured", 0);
				// очки опыта после пуша
        cJSON_AddNumberToObject(obj, "xp", 0);
				// уровень статуса
        cJSON_AddNumberToObject(obj, "level", 0);
				// очки подготовки к захвату(только к незахваченному d)
        cJSON_AddNumberToObject(obj, "prep_points", 0);

        // Установка view и all_stages по depth
        if (node->depth == 0) {
						// вид обьекта
            cJSON_AddStringToObject(obj, "view", "KINGDOM");
						// минимум очков 
            cJSON_AddNumberToObject(obj, "count_scores", 12); // сложнее захватывать

            // Поля королевства
						// колво городов
            cJSON_AddNumberToObject(obj, "all_count_town", element_length(node->child));
						// колво захваченных городов
            cJSON_AddNumberToObject(obj, "captured_towns", 0);
						// общее колво захваченных деревней
            cJSON_AddNumberToObject(obj, "total_captured_villages", 0);
						// статус национального мятежа
            cJSON_AddBoolToObject(obj, "multiple_rebellion_kingdom", 0);

        } else if (node->depth == 1) {
						
            cJSON_AddStringToObject(obj, "view", "TOWN");
            cJSON_AddNumberToObject(obj, "count_scores", 8);

            // Поля города
            cJSON_AddNumberToObject(obj, "all_count_village", element_length(node->child));
            cJSON_AddNumberToObject(obj, "captured_villages", 0);

        } else if (node->depth == 2) {
            cJSON_AddStringToObject(obj, "view", "VILLAGE");
            cJSON_AddNumberToObject(obj, "count_scores", 5); // проще всего
        }

        cJSON_AddItemToObject(territories, node->title, obj);
        printf("Добавлен: %s\n", node->title);
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

void save_game(GameWorld* gw){
	// переносим в текстовый массив
	char* output = cJSON_Print(gw->progress);
	// если все норм то открываем файл и записываем в него
	if(output){
		FILE* fp = fopen("progress.json", "w");
		if(fp){
			fwrite(output, 1, strlen(output), fp);
			fclose(fp);
		}
		free(output);
	}
	// все удаляем
	cJSON_Delete(gw->progress);
	element_destroy(gw->world);
	free(gw);
}

int main(int argc, char* argv[]){
	srand(time(NULL));
	GameWorld* gw = load_game_state();
	if (!gw) {
    fprintf(stderr, "Ошибка загрузки\n");
    return 1;
	}		
	load_events(gw);
	

	if(argc < 2){
		printf("Введите полную команду\n");
		return 1;
	}


	//handle_push(GameWorld* gw, char* flag, char* text_push, char* title)
	if(argc == 3 && strcmp(argv[1], "push") == 0){
		handle_push(gw, NULL, argv[2], NULL);		
	}
	else if(argc == 5 && strcmp(argv[1], "push") == 0 && (strcmp(argv[2], "-t") == 0 || strcmp(argv[2], "-c") == 0)){
		handle_push(gw, argv[2], argv[3], argv[4]);
	}
	else if(argc == 2 && strcmp(argv[1], "--init") == 0){
		fresh_news(gw);
		check_for_custom_events(gw);
		save_game(gw);
		return 0;
	}
	else if(argc == 4 && strcmp(argv[1], "push") == 0 && strcmp(argv[2], "complete") == 0){
		handle_complete(gw, argv[3]);
	}
	
	check_for_custom_events(gw);
	fresh_news(gw);
	save_game(gw);
	save_events(gw);
	return 0;
}
