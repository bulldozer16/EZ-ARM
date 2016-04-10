#include "ezarm.h"

#define SYMB 100	// Máximo número de etiquetas.

/* Estructura que se emplea para la tabla de símbolos.
   Contiene el nombre de la entrada, el número de línea,
   si es utilizada, si es parámetro o cuál es el parámetro
   y en qué función se encuentra. */

struct std_sym{ 		
	char *nombre;	 
	int dir;	
};

/* Se crea la tabla de símbolos. En este caso se implementa mediante un 
   arreglo de estructuras del tipo std_sym definido arriba, de tamaño SYMB. */

struct std_sym symtable[SYMB];	

/* Estructura para almacenar información importante sobre un branch.
   Se vuelve necesario dado que no se van a conocer todas las etiquetas hasta 
   haber analizado por completo el archivo fuente. */

/* Se declara una variable par llevar control del número de línea.
   Se inicializa con valor de 1, pues no puede existir línea 0. */

int linenumber = 1;
int err_bool = 0;	// Indica si ya apareció algún error.

/* Función que verifica si existe un elemento cuyo campo "nombre" coincide con el
   argumento str. Si existe se retorna un uno, de lo contrario un cero. */

int exists_symb(char* str){
	struct std_sym* sp;
  	for (sp = symtable; sp < &symtable[SYMB]; sp++) {
		if (sp->nombre && strcmp(sp->nombre, str) == 0 ) {
			return 1;
		}
	}
	return 0;
}

/* Retorna el código de condición a partir de la representación literal de la
   misma (cond_str) (AL -> 14) */

int get_cond_code(char* cond_str) {
	str_tolower(cond_str);
	int cond_code = 0;
	if (strcmp(cond_str, "eq") == 0) 
		{
		  cond_code = 0;		// cond = 0000
		}   
	else if (strcmp(cond_str, "ne") == 0) 
		{
		  cond_code = 1;		// cond = 0001
		} 
	else if (strcmp(cond_str, "hs") == 0) 
		{
		  cond_code = 2;		// cond = 0010
		} 
	else if (strcmp(cond_str, "lo") == 0) 
		{
		  cond_code = 3;		// cond = 0011
		} 
	else if (strcmp(cond_str, "mi") == 0) 
		{
		  cond_code = 4;		// cond = 0100
		} 
	else if (strcmp(cond_str, "pl") == 0) 
		{
		  cond_code = 5;		// cond = 0101
		} 
	else if (strcmp(cond_str, "vs") == 0) 
		{
		  cond_code = 6;		// cond = 0110
		} 
	else if (strcmp(cond_str, "vc") == 0) 
		{
		  cond_code = 7;		// cond = 0111
		} 
	else if (strcmp(cond_str, "hi") == 0) 
		{
		  cond_code = 8;		// cond = 1000
		} 
	else if (strcmp(cond_str, "ls") == 0) 
		{
		  cond_code = 9;		// cond = 1001
		} 
	else if (strcmp(cond_str, "ge") == 0) 
		{
		  cond_code = 10;		// cond = 1010
		} 
	else if (strcmp(cond_str, "lt") == 0) 
		{
		  cond_code = 11;		// cond = 1011
		} 
	else if (strcmp(cond_str, "gt") == 0) 
		{
		  cond_code = 12;		// cond = 1100
		} 
	else if (strcmp(cond_str, "le") == 0) 
		{
		  cond_code = 13;		// cond = 1101
		} 
	else if (strcmp(cond_str, "al") == 0) 
		{
		  cond_code = 14;		// cond = 1110
		} 
	cond_code = cond_code << 28;
	return cond_code;
}

/* Retorna el número de registro a partir de su representación literal (reg_str).
   (R10 -> 10) */

int get_reg_num(char* reg_str) {
	str_tolower(reg_str);
	int reg_num = 0;
	if (strcmp(reg_str, "r0") == 0) 
		{
		  reg_num = 0;
		}   
	else if (strcmp(reg_str, "r1") == 0) 
		{
		  reg_num = 1;
		} 
	else if (strcmp(reg_str, "r2") == 0) 
		{
		  reg_num = 2;
		} 
	else if (strcmp(reg_str, "r3") == 0) 
		{
		  reg_num = 3;
		} 
	else if (strcmp(reg_str, "r4") == 0) 
		{
		  reg_num = 4;
		} 
	else if (strcmp(reg_str, "r5") == 0) 
		{
		  reg_num = 5;
		} 
	else if (strcmp(reg_str, "r6") == 0) 
		{
		  reg_num = 6;
		} 
	else if (strcmp(reg_str, "r7") == 0) 
		{
		  reg_num = 7;
		} 
	else if (strcmp(reg_str, "r8") == 0) 
		{
		  reg_num = 8;
		} 
	else if (strcmp(reg_str, "r9") == 0) 
		{
		  reg_num = 9;
		} 
	else if (strcmp(reg_str, "r10") == 0) 
		{
		  reg_num = 10;
		} 
	else if (strcmp(reg_str, "r11") == 0) 
		{
		  reg_num = 11;
		} 
	else if (strcmp(reg_str, "r12") == 0) 
		{
		  reg_num = 12;
		} 
	else if (strcmp(reg_str, "r13") == 0) 
		{
		  reg_num = 13;
		} 
	else if (strcmp(reg_str, "r14") == 0) 
		{
		  reg_num = 14;
		} 
	else if (strcmp(reg_str, "r15") == 0) 
		{
		  reg_num = 15;
		} 
	return reg_num;
}

/* Para procesar un símbolo se obtiene una entrada vacía en la tabla de símbolos
   y se le asigna el argumento correspondiente a cada espacio:
   name a "nombre", lineno a "linea", use a "uso" pparam a "param" y pfunc a "func".
   Adicionalmente, esta función se podría utilizar para modificar una entrada 
   existente en la tabla de símbolos. */

void process_sym(char *name, int mem_dir) {
	struct std_sym *tmp = symlook(name);
	tmp->dir = mem_dir;	
}

/* Función de búsqueda. Se busca una entrada cuyo campo "nombre"
   coincida con el argumento s, si existe, se retorna un puntero a dicha entrada, 
   si no, se retorna la última entrada de la tabla. Se debe tener precaución,
   pues si no existe la entrada que se busca, se retorna la última, lo que podría
   ocasionar un comportamiento errático del programa. Es recomendable utilizar
   esta función en conjunto con existsSymb, para asegurarse de que la entrada 
   existe. */

struct std_sym* search(char *s) { 
	struct std_sym* sp;
  	for (sp = symtable; sp < &symtable[SYMB]; sp++) {
		if (sp->nombre && (strcmp(sp->nombre, s) == 0))
			return sp;
	}
	return sp;
}

/* Convierte su entrada (string) a una cadena de caracteres en minúscula. */

char* str_tolower(char *string) {
	for ( ; *string; ++string) *string = tolower(*string);
	return string;
}

/* Función de inserción y edición de la tabla. Se busca una entrada cuyo campo "nombre"
   coincida con el argumento s, si existe, se retorna un puntero a dicha entrada, si no
   se crea una entrada con el argumento s en el campo "nombre" y se retorna un puntero
   a la entrada recién creada.
   Existe un error si se excede la capacidad de la tabla. */

struct std_sym* symlook(char *s) { 		
	struct std_sym* sp;
  	for (sp = symtable; sp < &symtable[SYMB]; sp++) {
		if (sp->nombre && (strcmp(sp->nombre, s) == 0)) {
			return sp;
		}
		if (!sp->nombre) { 
			sp->nombre = strdup(s);
			return sp;
		}
  	}
  	yyerror("Las variables exceden la capacidad de la tabla de símbolos.");
  	exit(1);
}
