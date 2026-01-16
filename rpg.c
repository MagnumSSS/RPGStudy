#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include "cJSON.h"

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

void sync_node(struct task* node, cJSON* territories){
	if(!node) return;


	cJSON* existing = cJSON_GetObjectItem(territories, node->title);
	if(!existing){
		cJSON* new_obj = cJSON_CreateObject();
		int town = 0;
		int villages = 0;
		calculate_kingdoms_town(node, &town, &villages);

		//общие поля
		cJSON_AddStringToObject(new_obj, "status", "not_captured");
		cJSON_AddStringToObject(new_obj, "date_captured", "");
		cJSON_AddStringToObject(new_obj, "date_rebellion", "");
		cJSON_AddNumberToObject(new_obj, "time_captured", 0);

		if(node->depth == 0){
			cJSON_AddStringToObject(new_obj, "view", "KINGDOM");
			cJSON_AddNumberToObject(new_obj, "all_count_town", town);
			cJSON_AddNumberToObject(new_obj, "all_captured_town", 0);
			cJSON_AddNumberToObject(new_obj, "all_count_village", villages);
			cJSON_AddNumberToObject(new_obj, "all_captured_village", 0);
			cJSON_AddBoolToObject(new_obj, "multiple_rebellion_kingdom", 0);
			cJSON_AddNumberToObject(new_obj, "all_stages", 0);
			cJSON_AddNumberToObject(new_obj, "count_stages", 0);
			cJSON_AddBoolToObject(new_obj, "multiple_rebellion_town", 0);
			cJSON_AddItemToObject(territories, node->title, new_obj);
			printf("Добавлен новый объект: %s\n", node->title);
		}
		else if(node->depth == 1){	
			cJSON_AddNumberToObject(new_obj, "all_stages", 0);
			cJSON_AddNumberToObject(new_obj, "count_stages", 0);
			cJSON_AddBoolToObject(new_obj, "multiple_rebellion_town", 0);
			cJSON_AddStringToObject(new_obj, "view", "TOWN");
			cJSON_AddItemToObject(territories, node->title, new_obj);
			printf("Добавлен новый объект: %s\n", node->title);
		}
		else if(node->depth == 2){	
			cJSON_AddStringToObject(new_obj, "view", "VILLAGE");
			cJSON_AddItemToObject(territories, node->title, new_obj);
			printf("Добавлен новый объект: %s\n", node->title);
		}

	}
		sync_node(node->child, territories);
		sync_node(node->next, territories);
}

void calculate_kingdoms_town(struct task* kingdom, int* town, int* villages){
	// на всякий случай
	*villages = 0;
	*town = 0;

	if(!kingdom){
		printf("Неправильная ссылка на королевство\n");
		return;
	}
	// создаем отправную точку
	struct task* temp = kingdom->child;
	// считаем количество городов через функцию (которая считает вбок)
	*town = (int)element_length(kingdom->child);
	while(temp){
		// считаем детей каждого города
		*villages += (int)element_length(temp->child);
		// перемещаем отправную точку - указатель
		temp = temp->next;
	}
	
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

int main(){
	GameWorld* gw = load_game_state();
	if (!gw) {
    fprintf(stderr, "Ошибка загрузки\n");
    return 1;
	}
	printf("Готово.\n");
	save_game(gw);

	return 0;
}
