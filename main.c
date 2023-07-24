#include <stdio.h>                                      // includes
#include <string.h>


typedef struct node {                                   // estructura de la lista linkeada
    char* content[64];
    struct node *next;
} node_t;


void show_help(){                                       // funcion de ayuda cuando hay errores
    printf( "Uso: surix_word_sorter [input_file] [output_file]\n"
            "   [input_file]    -   archivo con el contenido a ordenar\n"
            "   [output_file]   -   archivo de salida con las palabras ordenadas\n"
            "\nPrograma creado por Dante Acosta\n");
}

int save_list(node_t **header, char* filename) {        // guarda toda la lista linkeada en un archivo especificado
    FILE* out_file = fopen(filename, "w");

    if (out_file == NULL) {
        printf("[!] No se pudo abrir el archivo.\n");
        return 1;
    }

    node_t * current = *header;
    current = current->next;
    while (current != NULL) {
        fprintf(out_file, current->content);
        fprintf(out_file, "\n");
        current = current->next;
    }

    fclose(out_file);
    return 0;
}

void remove_linebreaks(char* string) {                  // sustituye \n por ' ' de string
    char* ptr = string;
    while (*ptr) {
        if (*ptr == '\n') {
            *ptr = ' ';
        }
        ptr++;
    }
}

void clean_text(char* string) {                         // elimina puntos y simbolos de string
    char* src = string;
    char* dest = string;

    while (*src) {
        if (!ispunct(*src)) {
            *dest = *src;
            dest++;
        }
        src++;
    }
    *dest = '\0';
}

int which_first(char* string1, char* string2){          // algoritmo principal de sorting: la idea es que diga cual de las dos palabras esta primero basado en el numero ASCII de cada caracter
    int min = (strlen(string1)>strlen(string2)?strlen(string2):strlen(string1));    // obtiene la longitud de la cadena mas corta, para evitar problemas de out of bounds
    for (int i = 0; i < min; i++){ 
        char st1 = (string1[i]>='A'&&string1[i]<='Z')?string1[i]+32:string1[i];     // mi implementacion de una funcion similar a .toLower() de python 3. es para que el algoritmo no distinga mayusculas de minsculas
        char st2 = (string2[i]>='A'&&string2[i]<='Z')?string2[i]+32:string2[i];     // dado que los offsets entre mayusculas y minusculas en ASCII son siempre de 32, es facil de reemplazar
        if (st1 == st2){
            if (i == min-1){                            // pequeño parche para que tengan prioridad las palabras mas cortas sobre otras (ej "a" antes que "auto")
                if (strlen(string1) == min){
                    return 0;
                }else{
                    return 1;
                }
            }
            continue;
        }
        if (st1 > st2){                                 // comparamos los valores ASCII para saber cual esta antes
            return 1;
        }else{
            return 0;
        }
    }
}

int add_sorted(node_t **header, char* word){            // agrega y modifica la lista linkeada para acomodar las palabras segun el resultado de which_first
    node_t *new_item = (node_t*)malloc(sizeof(node_t));
    if (new_item == NULL) {                         // control de error en caso de que no se pueda asignar la memoria para guardar el contenido del archivo
        printf("[!] Error de asignacion de memoria\n");
        return 1;
    }
    strcpy(new_item->content, word);
    new_item->next = NULL;

    if (*header == NULL){                               // el primer item solo funciona como header, los subsiguientes seran los datos importantes
        *header = (node_t*)malloc(sizeof(node_t));
        if (*header == NULL) {                         // control de error en caso de que no se pueda asignar la memoria para guardar el contenido del archivo
            printf("[!] Error de asignacion de memoria\n");
            return 1;
        }
        node_t *header_null = *header;
        header_null->next = new_item;
        return 0;
    }

    node_t *current = *header;
    node_t *previous = *header;                         // utilizo un previous para podes itinerar por cada item y en caso de cumplirse que el nuevo esta antes, puedo acceder al item anterior

    while (current->next != NULL){
        previous = current;
        current = current->next;
        if (which_first(current->content, new_item->content)){  // basicamente, si resulta que el nuevo item va antes que el actual, lo inserta antes del actual y termina el ciclo
            previous->next = new_item;
            new_item->next = current;
            return 0;
        }
    }
    current->next = new_item;
    return 0;
}

void print_list(node_t **header) {                      // funcion para debug
    node_t * current = *header;
    current = current->next;
    while (current != NULL) {
        printf("%s\n", current->content);
        current = current->next;
    }
}

int main(int argc, char *argv[]){

    if (argc < 3){                                      // control de error, se necesitan 2 argumentos obligatoriamente (usamos 3 porque agrv[0] es el nombre del archivo)
        printf("[!] ERROR: Faltan argumentos\n");
        show_help();
        return 1;
    }

    FILE* input_file;                                   // definiciones de variables que usaremos
    char* file_content;
    long file_size;
    size_t result;

    printf("[i] Leyendo %s ...\n", argv[1]);

    input_file = fopen(argv[1], "r");
    if (input_file == NULL) {                           // control de error en caso de que no se pueda abrir el archivo
        printf("[!] Error al abrir el archivo %s\n", argv[1]);
        return 1;
    }

    fseek(input_file, 0, SEEK_END);                     // vamos al final del archivo para obtener su tamaño
    file_size = ftell(input_file);
    fseek(input_file, 0, SEEK_SET);
    file_content = (char*)malloc(file_size + 2);        // + 2 por la terminacion \0 y ` ` que agregamos luego

    if (file_content == NULL) {                         // control de error en caso de que no se pueda asignar la memoria para guardar el contenido del archivo
        printf("[!] Error de asignacion de memoria\n");
        return 1;
    }

    result = fread(file_content, 1, file_size, input_file);
    file_content[result-1] = ' ';                       // agregamos 0 porque usaremos strtok con ' ' y sin este agregado, falla en la palabra final
    file_content[result] = '\0';                        // agregamos \0 para indicar final de cadena
    fclose(input_file);
    remove_linebreaks(file_content);                    // cambiamos \n por ' ' ya que usamos strtok con ' '


    int word_count = 0;                                 // con este bloque obtenemos el numero de palabras y el tamaño de la palabra mas larga, para la asignacion dinamica de la memoria con malloc
    int max_size = 0;
    int size_count = 0;
    for (int i = 0; i < strlen(file_content); i++){
        if(file_content[i] == ' ' || file_content[i] == '\n'  || file_content[i] == '\0'){
            word_count++;
            max_size = size_count > max_size ? size_count : max_size;   // actualiza el maximo valor, si es que la palabra actual lo supera
            size_count = 0;
            continue;
        }
        size_count++;
    }
    
    int wc = 0;                                         // asignacion de memoria para todas las palabras de la lista
    char** split_data = (char**)malloc(word_count * sizeof(char*));
    if (split_data == NULL) {
        printf("[!] Error de asignacion de memoria\n");
        return 1;
    }
    for (int i = 0; i < word_count; i++){
        split_data[i] = (char*)malloc(max_size * sizeof(char));
        if (split_data[i] == NULL) {
            printf("[!] Error de asignacion de memoria\n");
            return 1;
        }
    }


    int word_pos = 0;                                   // particion del texto de entrada ( este bloque es como hacer .split(" ") en python 3)
    char* token = strtok(file_content, " ");
    while (token != NULL){
        split_data[word_pos] = strdup(token);
        token = strtok(NULL, " ");
        word_pos++;
    }

    node_t *sorted_list_header = NULL;                  // inicializamos la lista linkeada

    printf("[i] Ordenando...\n");

    for (int i = 0; i < word_count; i++){
        clean_text(split_data[i]);                      // limpiamos puntos, comas y simbolos, para evitarq ue interfieran con el algoritmo
        add_sorted(&sorted_list_header, split_data[i]); // agregamos cada palabra, la funcion la ordena automaticamente
    }

    printf("[i] Guardando en %s ...\n", argv[2]);

    save_list(&sorted_list_header, argv[2]);            // guardamos los datos en el archivo


    free(file_content);                                 // liberamos todos los mallocs que hicimos
    for (int i = 0; i < word_count; i++){
        free(split_data[i]);
    }
    free(split_data);

    printf("[i] Completado. Programa creado por Dante Acosta. Saliendo...\n");

    return 0;                                           // fin del programa
}
