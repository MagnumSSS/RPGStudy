#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include "cJSON.h"
#include <time.h>


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
} GameWorld;

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
struct task* find_by_title(struct task* node, const char* title); 

/// 2 ///

struct task* find_parent(struct task* node, const char* title){
	if(!node || !title){
		printf("Не удалось прочитать struct task* и title в find_parent");
		return NULL;
	}
	struct task* node_child = node->child;
	while(node_child){
		if(strcmp(node_child->title, title)){
			return node;
		}
		node_child = node_child->next;
	}
	
	struct task* res = find_parent(node->child, title);
	if(res) return res;
	return find_parent(node->next, title);
}


// функция времени
char* get_current_date() {
    time_t now = time(NULL);
    struct tm* local = localtime(&now);
    static char date[11];  // "YYYY-MM-DD\0" → 11 байт
    strftime(date, sizeof(date), "%Y-%m-%d", local);
    return date;
}

void log_text_in_file(char* title){
	int fd = open("history.log", O_WRONLY | O_CREAT | O_APPEND, 644);
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
	size_t len = strlen(title);
	ssize_t n = write(fd, title, len);
	ssize_t n2 = write(fd, " | ", 3);
	ssize_t n_date = write(fd, , 10);
	if(n != (ssize_t)len || n_date != 10 || n2 != 3){
		printf("Не удалось записать в лог файл\n");
		perror("write");
		close(fd);
		return;
	}
	write(fd, "\n", 1);
	close(fd);
}

void handle_push(GameWorld* gw, char* flag, char* text_push, char* title){
	if(!text_push){
		printf("Текст Пуша не найден\n");
	}
	log_text_in_file(text_push);

	if(!flag){
		//ничего не делаем - обычный пуш
		return;	
	}

	else if(strcmp(flag, "-t") == 0){
		if(!title){
			printf("Для флага -t нужно указать город/cело\n");
			return;
		}
		// обработчик пуша с -t
	}
	else if(strcmp(flag, "-c") == 0){
		if(!title){
			printf("Для флага -c нужно указать дату события[dd-mm-YYYY]\n");
			return;
		}
		// обработчик пуша с -c
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
            cJSON_AddBoolToObject(obj, "multiple_rebellion_town", 0);

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
	GameWorld* gw = load_game_state();
	if (!gw) {
    fprintf(stderr, "Ошибка загрузки\n");
    return 1;
	}
	/*
	if(argc < 2){
		printf("Введите полную команду\n");
		return 1;
	}

	if(strcmp(argv[1], "study") == 0){
		if(argc < 3){
			printf("Укажите название села/улицы\n");
		}
		else{
			handle_study(gw, argv[2]);
		}
	} else {
		printf("Неизвестная команда!\n");
	}
	*/
	save_game(gw);

	return 0;
}
