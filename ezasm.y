/* 
 * File:   ezasm.y
 * Author: Sergio Vargas
 *
 * Created on 10 de marzo de 2016, 01:47
 */

%{

/* Se incluyen las cabeceras necesarias */

#include "ezarm.c"
#include <errno.h>

/* Declaración de constantes */

#define BRANCH 200	// Máximo número de instrucciones branch.

/* Variables de soporte (lex-yacc) */

extern int yylex();
extern int yyparse();
extern int err_bool;
extern int linenumber;
extern FILE *yyin;

/* Punteros de archivos */

FILE *asm_output; 	// Puntero al archivo de salida en ensamblador.
FILE *inputf;		// Puntero al archivo con el programa fuente.
FILE *err_file;		// Puntero al archivo de manejo de errores.

/* Variables de control */

int inst = 0x0;			// Representación númerica de instrucción actual
int branch_count = 0;		// Conteo de instrucciones branch
char br_str[18];		// Cadena de caracteres para colocación de branch

struct branches{ 		
	char* label;	 
	int dir;
	int code;	
};

/* Arreglo que contiene todas las instrucciones branch en el código fuente */

struct branches branch_table[BRANCH];

%}

/* Definición de los tipos que puede adquirir cada uno de los elementos 
   terminales y no terminales */

%union {int ival; char *string;};

/* Definición del símbolo inicial y de los tokens de la gramática */

%start program

%token NEWLINE
%token NUM
%token NUMH
%token ID
%token PRE

%token COND
%token SET
%token REGISTER

%token INSTR_AND
%token INSTR_EOR
%token INSTR_SUB
%token INSTR_RSB
%token INSTR_ADD
%token INSTR_ADC
%token INSTR_SBC
%token INSTR_RSC
%token INSTR_CMP
%token INSTR_CMN
%token INSTR_ORR
%token INSTR_MOV
%token INSTR_LSL
%token INSTR_ASR
%token INSTR_RRX
%token INSTR_ROR
%token INSTR_BIC
%token INSTR_MVN 
%token INSTR_MUL
%token INSTR_MLA
%token INSTR_STR
%token INSTR_LDR
%token INSTR_STRB
%token INSTR_LDRB
%token INSTR_B
%token INSTR_LSR

%%

program: 
	instruction newline program
	| NEWLINE program
	| label program
	|
	;

newline: 
	NEWLINE							{++linenumber;
						 		 if (linenumber > 256) {
									yyerror("Se excede el espacio de memoria de instrucciones");
						 		 }
						 		 fprintf(asm_output, "%08X\t%08X\n", ((linenumber - 2) * 4), inst);
						 		 inst = 0;}
	;

label: 
	ID							{if (exists_symb($<string>1) == 0) {
						 			process_sym($<string>1, linenumber);
						 		 } else {
						  			yyerror("Ya se ha definido la etiqueta");
						 		 }}
	;

cond:	
	COND							{inst = inst | get_cond_code($<string>1);}
	|							{inst = inst | 0xE0000000;}	
	;

set: 
	SET							{inst = inst | 0x100000;}
	|
	;

Src2:	
	REGISTER						{inst = inst | data_reg_code($<string>1, 'm');}
	| REGISTER ',' sh_inst immediate			{inst = inst | encode_register($<string>1, $<ival>3, $<ival>4);}
	| immediate						{inst = inst | 0x2000000;		
						 		 inst = inst | encode_immediate($<ival>1);}
	;

Src2_mem_sign: 
	Src2_mem						{inst = inst | 0x800000;}
	| '-' Src2_mem
	;

Src2_mem:	
	REGISTER						{inst = inst | 0x2000010;
							 	 inst = inst | encode_register($<string>1, 0, 0);}
	| REGISTER ',' sh_inst immediate			{inst = inst | 0x2000010;
						 	 	 inst = inst | encode_register($<string>1, $<ival>3, $<ival>4);}
	| immediate						{inst = inst | encode_imm_mem($<ival>1);}
	;

Src2_sh: 
	REGISTER 						{inst = inst | 0x10;
						 		 $<ival>$ = data_reg_code($<string>1, 's');
						 		 inst = inst | $<ival>$;
						 		 $<ival>$ += 1;}
	| immediate						{$<ival>$ = encode_imm_sh($<ival>1);
						 		 inst = inst | $<ival>$;}
	;

immediate: 
	PRE NUM							{$<ival>$ = $<ival>2;}
	| PRE NUMH						{$<ival>$ = $<ival>2;}
	;	

sh_inst: 
	INSTR_LSL						{$<ival>$ = 0;}	
	| INSTR_LSR						{$<ival>$ = 1;}	
	| INSTR_ASR						{$<ival>$ = 2;}
	| INSTR_ROR						{$<ival>$ = 3;}
	;

mem_mode: 
	'[' reg_n ']'						{inst = inst | 0x1000000;}
	| '[' reg_n ',' Src2_mem_sign ']'			{inst = inst | 0x1000000;}
	| '[' reg_n ',' Src2_mem_sign ']' '!'			{inst = inst | 0x1200000;}
	| '[' reg_n ']' ',' Src2_mem_sign			
	;

reg_d: 
	REGISTER						{inst = inst | data_reg_code($<string>1, 'd');}
	;

reg_n: 
	REGISTER						{inst = inst | data_reg_code($<string>1, 'n');}
	;

reg_m: 
	REGISTER						{inst = inst | data_reg_code($<string>1, 'm');}
	;

reg_s: 
	REGISTER						{inst = inst | data_reg_code($<string>1, 's');}
	;

instruction: 
	instr_and						{inst = inst | 0x000000;}
	| instr_eor						{inst = inst | 0x200000;}
	| instr_sub						{inst = inst | 0x400000;}
	| instr_rsb						{inst = inst | 0x600000;}
	| instr_add						{inst = inst | 0x800000;}
	| instr_adc						{inst = inst | 0xA00000;}
	| instr_sbc						{inst = inst | 0xC00000;}
	| instr_rsc						{inst = inst | 0xE00000;}
	| instr_cmp						{inst = inst | 0x1400000;}
	| instr_cmn						{inst = inst | 0x1600000;}
	| instr_orr						{inst = inst | 0x1800000;}
	| instr_mov						{inst = inst | 0x1A00000;}
	| instr_lsl						{inst = inst | 0x1A00000;}
	| instr_asr						{inst = inst | 0x1A00000;}
	| instr_rrx						{inst = inst | 0x1A00000;}
	| instr_ror						{inst = inst | 0x1A00000;}
	| instr_bic						{inst = inst | 0x1C00000;}
	| instr_mvn						{inst = inst | 0x1E00000;}
	| instr_mul						{inst = inst | 0x000000;}
	| instr_mla						{inst = inst | 0x200000;}
	| instr_str						{inst = inst | 0x4000000;}
	| instr_ldr						{inst = inst | 0x4000000;}
	| instr_strb						{inst = inst | 0x4000000;}
	| instr_ldrb						{inst = inst | 0x4000000;}
	| instr_b			
	;

/*-------------------------------- DATOS ----------------------------------*/

instr_and: 
	INSTR_AND set cond reg_d ',' reg_n ',' Src2
	;

instr_eor: 
	INSTR_EOR set cond reg_d ',' reg_n ',' Src2
	;

instr_sub: 
	INSTR_SUB set cond reg_d ',' reg_n ',' Src2
	;

instr_rsb: 
	INSTR_RSB set cond reg_d ',' reg_n ',' Src2
	;

instr_add: 
	INSTR_ADD set cond reg_d ',' reg_n ',' Src2
	;

instr_adc: 
	INSTR_ADC set cond reg_d ',' reg_n ',' Src2
	;

instr_sbc: 
	INSTR_SBC set cond reg_d ',' reg_n ',' Src2
	;

instr_rsc: 
	INSTR_RSC set cond reg_d ',' reg_n ',' Src2
	;

instr_cmp: 
	INSTR_CMP cond reg_n ',' Src2				{inst = inst | 0x100000;} 
	;

instr_cmn: 
	INSTR_CMN cond reg_n ',' Src2				{inst = inst | 0x100000;} 
	;

instr_orr: 
	INSTR_ORR set cond reg_d ',' reg_n ',' Src2
	;

instr_mov: 
	INSTR_MOV set cond reg_d ',' Src2
	;

instr_lsl: 
	INSTR_LSL set cond reg_d ',' reg_m ',' Src2_sh		{if ($<ival>8 == 0) {
									yyerror("shamt5 no puede ser 0");
								 }} 
	;

instr_asr: 
	INSTR_ASR set cond reg_d ',' reg_m ',' Src2_sh		{inst = inst | 0x40;} 
	;

instr_rrx: 
	INSTR_RRX set cond reg_d ',' reg_m			{inst = inst | 0x60;} 
	;

instr_ror: 
	INSTR_ROR set cond reg_d ',' reg_m ',' Src2_sh		{if ($<ival>8 == 0) {
									yyerror("shamt5 no puede ser 0");
								 }
								 inst = inst | 0x60;} 
	;

instr_bic: 
	INSTR_BIC set cond reg_d ',' reg_n ',' Src2
	;

instr_mvn: 
	INSTR_MVN set cond reg_d ',' Src2
	;

/*--------------------------- MULTIPLICACIÓN ------------------------------*/

instr_mul: 
	INSTR_MUL set cond reg_n ',' reg_m ',' reg_s		{inst = inst | 0x90;} 
	;

instr_mla: 
	INSTR_MLA set cond reg_n ',' reg_m ',' reg_s ',' reg_d	{inst = inst | 0x90;} 
	;

/*------------------------------- MEMORIA ---------------------------------*/

instr_str: 
	INSTR_STR cond reg_d ',' mem_mode					
	;

instr_ldr: 
	INSTR_LDR cond reg_d ',' mem_mode			{inst = inst | 0x100000;}
	;

instr_strb: 
	INSTR_STRB cond reg_d ',' mem_mode			{inst = inst | 0x400000;}
	;

instr_ldrb: 
	INSTR_LDRB cond reg_d ',' mem_mode			{inst = inst | 0x500000;}
	;

/*-------------------------------- SALTOS ---------------------------------*/

instr_b	: 
	INSTR_B cond ID						{branch_table[branch_count].label = $<string>3;
						 		 branch_table[branch_count].dir = linenumber;
						 		 branch_table[branch_count].code = inst | 0xA000000;;
						 		 branch_count++;}			
	;

%%

/* Retorna el código del registro en la respectiva posición de la instrucción, 
   dependiendo del tipo de registro (Rd, Rn, ...). Obsérvese que dicho código
   es con el que debe hacerse OR al valor numérico de la instrucción actual. */

int data_reg_code(char *reg_str, char reg_type) {
	int reg_code;
	int reg_offset;	
	switch (reg_type) {
		  case 'm': 
		    reg_offset = 0;
		    break;
		  case 's': 
		    reg_offset = 8;
		    break;
		  case 'd': 
		    reg_offset = 12;
		    break;
		  case 'n': 
		    reg_offset = 16;
		    break;
	}
	reg_code = get_reg_num(reg_str);
	reg_code = reg_code << reg_offset;
	return reg_code;
}

/* Algoritmo de codificación de inmediatos, de acuerdo con la representación 
   basada en rotación que se emplea en la arquitectura ARMv4.
   Su entrada es el valor que se desea codificar. Retorna el valor codificado. */

int encode_immediate(int imm) {
	if (imm <= 255) {
		return imm;
	} else {
		if (imm % 2 == 0) {
			int rot = 0;
			int res = 0;
			do {	
				rot += 1;
				res = rotate_left(imm, 2 * rot);
			} while (res  > 255);
				res = res | (rot << 8);
			return res;
		} else {
			yyerror("Imposible representar inmediato");
			return 0;
		}
	}
}

/* Verifica que el valor del inmediato sea del ancho adecuado para las
   instrucciones de memoria */

int encode_imm_mem(int imm) {
	if (imm > 0xFFF) {
		yyerror("Inmediato de tamaño mayor a 12 bits");
		return 0;
	}
	
	if (imm % 0x4 == 0) {
		return imm;
	}
	yyerror("Inmediato debe ser múltiplo de 4");
	return 0;
}

/* Codifica el valor inmediato (imm) de manera que quede listo para realizar
   la operación OR con el valor numérico de la instrucción actual. Se emplea 
   en instrucciones de desplazamiento */

int encode_imm_sh(int imm) {
	if (imm > 31) {
		yyerror("No se puede desplazar a más de 31 espacios");
		return 0;
	}
	return imm << 7;
}

/* Retorna Src2 codificado en modo de direccionamiento de registro. 
   Toma como entradas la representación literal de un registro (reg_str), 
   el código de la función de desplazamiento (sh_code) y la cantidad de 
   espacios para dicho desplazamiento (shamt). */

int encode_register(char *reg_str, int sh_code, int shamt) {
	if (shamt > 31) {
		yyerror("No se puede desplazar a más de 31 espacios");
		return 0;
	}
	int src2_code = data_reg_code(reg_str, 'm');
	src2_code = src2_code | (sh_code << 5);
	src2_code = src2_code | (shamt << 7);
	return src2_code;
}

/* Abre los archivos respectivos y los asigna al puntero
   correspondiente. El argumento es el nombre del archivo
   de entrada. Retorna el nombre del archivo de salida en
   ensamblador */

void init_files_asm(char *inputfn) {
	inputf = fopen(inputfn , "r");
	if (inputf == NULL) {
		printf("Error: Archivo inválido.\n");
		exit(-1);
	}
	err_file = fopen("Módulo de Errores Ensamblado.txt", "w");
	asm_output = fopen("out.txt", "w+");
	return;
}

/* Procesa todas las instrucciones branch en el código fuente. Para ello se 
   obtienen las etiquetas de salto para cada una de esas instrucciones y se 
   comparan con las etiquetas en la tabla de símbolos, para luego codificar
   las etiquetas usando PC_salto - (PC_actual + 8) >> 2 */

void process_branches() {
	int pc_act;
	int pc_dest;
	int imm;
	int br_code;
	struct branches* bt;
	struct std_sym* sp;
	for (bt = branch_table; bt < &branch_table[BRANCH]; bt++) {
		if (bt->label) {
			if (exists_symb(bt->label) == 1) {
			 	sp = search(bt->label);
				br_code = bt->code;
				pc_act = (bt->dir-1)*4;
				pc_dest = (sp->dir-1)*4;		
				imm = (pc_dest-(pc_act+8))>>2;		
				imm = imm & 0xFFFFFF;				
				br_code = br_code | imm;
				if (!fseek(asm_output, (bt->dir-1)*18, SEEK_SET)) { 
					fprintf(asm_output,"%08X\t%08X\n", pc_act, br_code);
				} else {
					printf("Imposible sobreescribir");
				}
			} else {
				yyerror("No existe la etiqueta");
			}
		} else {
			return;
		}
  	}
}

/* Función de rotación a la izquierda, empleada en la codificación de inmediatos. */

int rotate_left(int x, int n) {
	return ((x << n) | (x >> (32 - n)));
}

/* Función de manejo de errores por defecto de yacc. La entrada es el mensaje que se desea dar
   cuando ocurre un error. En este caso se implementó de manera tal que se impriman todos los 
   mensajes en un archivo de texto. */

void yyerror (const char *s) {
	if (err_bool == 0) err_bool = 1;
	fprintf(err_file, "Error en la línea %d. Error: %s.\n", linenumber, s);
} 

/* Función principal. Primero se inicializan todos los archivos, con sus respectivas cabeceras.
   Luego se evalúan el archivo de entrada, token por token, hasta agotarlos. Cada vez que coincide
   una regla se ejecutan las acciones que se encuentran a la derecha. Si se provee el argumento -p
   se imprime la tabla de símbolos al archivo "TablaSímbolos.txt". Si existe algún error, se indica 
   mediante un mensaje en consola y se escribe al archivo "Módulo de Errores.txt" y no se genera 
   la salida en ensamblador. Si no hay errores se indica que se realizó la compilación con éxito. */

int main(int argc, char **argv) {
	init_files_asm(argv[1]);
	yyin = inputf;	
	do {
		yyparse();
		
	} while (!feof(yyin));
	process_branches();
	fclose(yyin);
	if (err_bool == 1) {
		fclose(asm_output);
		remove("out.txt");
		printf("Error durante el ensamblado. Ver Módulo de Errores Ensamblado.txt.\n");
		fclose(err_file);
		return 1;
	}
	fprintf(err_file, "¡No hay errores en el archivo %s!", argv[1]);
	fclose(err_file);
	fclose(asm_output);
	printf("Ensamblado exitosamente.\n");
	return 0;
}
