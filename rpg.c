#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <cJSON.h>

//FILE* output_file = NULL;

struct task {
    char* title;
    struct task* child;
    struct task* next;
		int depth;
};




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
  fclose(file);

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
	if(!node){
		printf("Не правильная ссылка на мир\n");
		return;
	}

	cJSON* existing = cJSON_GetObjectItem(territories, node->title);
	if(!existing){
		cJSON* new_odj = cJSON_CreateObject();

		//общие поля
		cJSON_AddStringToObject(new_obj, "status", "not_captured");
		cJSON_AddStringToObject(new_obj, "date_captured", "");
		cJSON_AddStringToObject(new_obj, "date_rebellion", "");
		cJSON_AddNumberToObject(new_obj, "time_captured", 0);

		if(node->depth == 0){
			cJSON_AddStringToObject(new_obj, "view", "KINGDOM");
			cJSON_AddNumberToObject(new_obj, "all_count_town", 0);
			cJSON_AddNumberToObject(new_obj, "all_captured_town", 0);
			cJSON_AddNumberToObject(new_obj, "all_count_village", 0);
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


int main(){
	struct task* world = load_from_file("world.km");
	if (!world) {
    fprintf(stderr, "Не удалось загрузить world.km\n");
    return 1;
	}
	
	// преобразуем json в текст
	char* json_text = read_file("progress.json");
	cJSON* progress = NULL;
		
	// если все ок, парсим 
	if(json_text){
		progress = cJSON_Parse(json_text);
		free(json_text);
	}

	// если файла нет - создаем пустой
	if(!progress){
		progress = cJSON_CreateObject();
		cJSON_AddItemToObject(progress, "territories", cJSON_CreateObject());
	}

	cJSON* territories = cJSON_GetObjectItem(progress, "territories");
	if(!territories){
		territories = cJSON_CreateObject();
		cJSON_AddItemToObject(progress, "territories", territories);
	}
	sync_node(world, territories);

	// Сохраняем обновлённый JSON
	char* output = cJSON_Print(progress);
  FILE* f = fopen("progress.json", "w");
  if (f) {
    fwrite(output, 1, strlen(output), f);
    fclose(f);
  }
  free(output);
  cJSON_Delete(progress);
  element_destroy(world);


	return 0;
}
