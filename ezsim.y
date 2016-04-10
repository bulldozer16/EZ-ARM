/* 
 * File:   ezsim.y
 * Author: Sergio Vargas / Héctor Porras
 *
 * Created on 25 de marzo de 2016, 11:10
 */

%{
/* Se incluyen las cabeceras necesarias */

#include "ezarm.c" 
#include "instrucciones.c"

/* Declaración de constantes */

#define SYMB 100	// Máximo número de símbolos en la tabla.
#define BRANCH 200	// Máximo número de instrucciones branch.

/* Variables de soporte (lex-yacc) */

extern int yylex();
extern int yyparse();
extern FILE *yyin;
typedef struct yy_buffer_state *YY_BUFFER_STATE;
extern YY_BUFFER_STATE yy_scan_string(char *str);
extern void yy_delete_buffer(YY_BUFFER_STATE buffer);
extern void yy_switch_to_buffer(YY_BUFFER_STATE new_buffer);

/* Punteros de archivos */

FILE *inputf;		// Puntero al archivo con el programa fuente.
FILE *err_file;		// Puntero al archivo de manejo de errores.
FILE *est_time;		// Puntero al archivo que tendrá el tiempo estimado de ejecución.

/* Variables de control */

extern int linenumber;	/* Se declara una variable par llevar control del número de línea.
			   Se inicializa con valor de 1, pues no puede existir línea 0. */
extern int err_bool;	// Indica si ya apareció algún error
char* br_str;		// Cadena de caracteres para colocación de branch
int branch_cond = 0;
int first_pass = 1;

int cond_code = 14;	// Código de la condición, por defecto está en 14.
int reg_num = 0;

/* Registros Rd, Rn, Rm y RS utilizados en las distintas instrucciones */

int rd = 0;		
int rn = 0;
int rm = 0;
int rs = 0;

int src2 = 0;		// Valor del inmediato en las instrucciones.
int set = 0;		// Bit que indica si la instrucción debe realizar el set de las banderas.

int dir_mode = 0;	// Modo de direccionamiento para las instrucciones de memoria.

int inst_count = 0;	// Cuenta las instrucciones ejecutadas.

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
	| label instruction newline program		
	| label newline	program			
	| NEWLINE program
	|
	;

newline: 
	NEWLINE							{
								 ++linenumber;
								 if (first_pass)
								 {
									
						 		 	if (linenumber > 256) {
										yyerror("se excede el espacio de memoria de instrucciones");
								 	}
						 		 }}
	;

label: 
	ID							{if (first_pass) {
								 	if (exists_symb($<string>1) == 0) {
								 		process_sym($<string>1, linenumber);
									} else {
									  	yyerror("Ya se ha definido la etiqueta");
									}
						 		 }}
	;

cond:	
	COND							{cond_code = get_cond_code($<string>1) >> 28;}
	|							{cond_code = 14;}
	;

set: 
	SET							{set = 1;}
	|							{set = 0;}
	;

Src2:	
	REGISTER						{reg_num = get_reg_num($<string>1);
								 src2 = load_register(reg_num);}
	| REGISTER ',' sh_inst immediate			{reg_num = get_reg_num($<string>1);
						 		 src2 = get_shifted(reg_num, $<ival>3, $<ival>4);}
	| immediate						{src2 = $<ival>1;}
	;

Src2_mem_sign: 
	Src2_mem						{src2 = src2 * 1;}
	| '-' Src2_mem						{src2 = src2 * -1;}
	;

Src2_mem:	
	REGISTER						{reg_num = get_reg_num($<string>1);
							 	 src2 = load_register(reg_num);}
	| REGISTER ',' sh_inst immediate			{reg_num = get_reg_num($<string>1);
							 	 src2 = get_shifted(reg_num, $<ival>3, $<ival>4);}
	| immediate						{src2 = $<ival>1;}
	;

Src2_sh: 
	REGISTER 						{reg_num = get_reg_num($<string>1);
								 src2 = load_register(reg_num);}
	| immediate						{src2 = $<ival>1;}
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
	'[' reg_n ']'						{dir_mode = 0; src2 = 0;}
	| '[' reg_n ',' Src2_mem_sign ']'			{dir_mode = 1;}
	| '[' reg_n ',' Src2_mem_sign ']' '!'			{dir_mode = 2;}
	| '[' reg_n ']' ',' Src2_mem_sign			{dir_mode = 3;}			
	;

reg_d: 
	REGISTER						{rd = get_reg_num($<string>1);}
	;

reg_n: 
	REGISTER						{rn = get_reg_num($<string>1);}
	;

reg_m: 
	REGISTER						{rm = get_reg_num($<string>1);}
	;

reg_s: 
	REGISTER						{rs = get_reg_num($<string>1);}
	;

instruction: 
	instr_and				
	| instr_eor			
	| instr_sub			
	| instr_rsb			
	| instr_add			
	| instr_adc			
	| instr_sbc			
	| instr_rsc			
	| instr_cmp			
	| instr_cmn			
	| instr_orr			
	| instr_mov			
	| instr_lsl			
	| instr_asr			
	| instr_rrx			
	| instr_ror			
	| instr_bic			
	| instr_mvn			
	| instr_mul			
	| instr_mla			
	| instr_str			
	| instr_ldr			
	| instr_strb			
	| instr_ldrb			
	| instr_b			
	;


/*-------------------------------- DATOS ----------------------------------*/

instr_and: 
	INSTR_AND set cond reg_d ',' reg_n ',' Src2		{if (!first_pass) {
									and(set, cond_code, rd, rn, src2);
									inst_count++; 
								 }}
	;

instr_eor: 
	INSTR_EOR set cond reg_d ',' reg_n ',' Src2		{if (!first_pass) {
									eor(set, cond_code, rd, rn, src2);
									inst_count++;
								 }}
	;
	
instr_sub: 
	INSTR_SUB set cond reg_d ',' reg_n ',' Src2		{if (!first_pass) {
									sub(set, cond_code, rd, rn, src2);
									inst_count++;
								 }}
	;
	
instr_rsb: 
	INSTR_RSB set cond reg_d ',' reg_n ',' Src2		{if (!first_pass) {
									rsb(set, cond_code, rd, rn, src2);
									inst_count++;
								 }}
	;

instr_add: 
	INSTR_ADD set cond reg_d ',' reg_n ',' Src2		{if (!first_pass) {
									add(set, cond_code, rd, rn, src2);
									inst_count++;
								 }}
	;

instr_adc: 
	INSTR_ADC set cond reg_d ',' reg_n ',' Src2		{if (!first_pass) {
									adc(set, cond_code, rd, rn, src2);
									inst_count++;
								 }}
	;

instr_sbc: 
	INSTR_SBC set cond reg_d ',' reg_n ',' Src2		{if (!first_pass) {
									sbc(set, cond_code, rd, rn, src2);
									inst_count++;
								 }}
	;

instr_rsc: 
	INSTR_RSC set cond reg_d ',' reg_n ',' Src2		{if (!first_pass) {
									rsc(set, cond_code, rd, rn, src2);
									inst_count++;
								 }}
	;

instr_cmp: 
	INSTR_CMP cond reg_n ',' Src2				{if (!first_pass) {	
									cmp(cond_code, rn, src2);
									inst_count++;
								 }} 
	;

instr_cmn: 
	INSTR_CMN cond reg_n ',' Src2				{if (!first_pass) {
									cmn(cond_code, rn, src2);
									inst_count++;
								 }} 
	;

instr_orr: 
	INSTR_ORR set cond reg_d ',' reg_n ',' Src2		{if (!first_pass) {
									orr(set, cond_code, rd, rn, src2);
									inst_count++;
								 }}
	;

instr_mov: 
	INSTR_MOV set cond reg_d ',' Src2			{if (!first_pass) {
									mov(set, cond_code, rd, src2);
									inst_count++;
								 }}
	;

instr_lsl: 
	INSTR_LSL set cond reg_d ',' reg_m ',' Src2_sh		{if (!first_pass) {
									lsl(set, cond_code, rd, rm, src2);
									inst_count++;
								 }}
	;

instr_asr: 
	INSTR_ASR set cond reg_d ',' reg_m ',' Src2_sh		{if (!first_pass) {
									asr(set, cond_code, rd, rm, src2);
									inst_count++;
								 }}
	;

instr_rrx: 
	INSTR_RRX set cond reg_d ',' reg_m			{if (!first_pass) {
									rrx(set, cond_code, rd, rm);
									inst_count++;
								 }} 
	;

instr_ror: 
	INSTR_ROR set cond reg_d ',' reg_m ',' Src2_sh		{if (!first_pass) {
									ror(set, cond_code, rd, rm, src2);
									inst_count++;
								 }}
	;

instr_bic: 
	INSTR_BIC set cond reg_d ',' reg_n ',' Src2		{if (!first_pass) {
									bic(set, cond_code, rd, rn, src2);
									inst_count++;
								 }}
	;

instr_mvn: 
	INSTR_MVN set cond reg_d ',' Src2			{if (!first_pass) {
									mvn(set, cond_code, rd, src2);
									inst_count++;
								 }}
	;

/*--------------------------- MULTIPLICACIÓN ------------------------------*/

instr_mul: 
	INSTR_MUL set cond reg_n ',' reg_m ',' reg_s		{if (!first_pass) {
									mul(set, cond_code, rn, rm, rs);
									inst_count++;
								 }} 
	;
			
instr_mla: 
	INSTR_MLA set cond reg_n ',' reg_m ',' reg_s ',' reg_d	{if (!first_pass) {
									mla(set, cond_code, rn, rm, rs, rd);
									inst_count++;
								 }} 
	;

/*------------------------------- MEMORIA ---------------------------------*/

instr_str: 
	INSTR_STR cond reg_d ',' mem_mode			{if (!first_pass) {
									int status = str(cond_code, rd, rn, src2, dir_mode);
									if (status != 0)
									{
										yyerror("Dirección inválida");
									}
									inst_count++;
								 }}
	;

instr_ldr: 
	INSTR_LDR cond reg_d ',' mem_mode			{if (!first_pass) {
									int status = ldr(cond_code, rd, rn, src2, dir_mode);
									if (status != 0)
									{
										yyerror("Dirección inválida");
									}
									inst_count++;
								 }}
	;

instr_strb: 
	INSTR_STRB cond reg_d ',' mem_mode			{if (!first_pass) {
									int status = strb(cond_code, rd, rn, src2, dir_mode);
									if (status != 0)
									{
										yyerror("Dirección inválida");
									}
									inst_count++;
								 }}
	;

instr_ldrb: 
	INSTR_LDRB cond reg_d ',' mem_mode			{if (!first_pass) {
									int status = ldrb(cond_code, rd, rn, src2, dir_mode);
									if (status != 0)
									{
										yyerror("Dirección inválida");
									}
									inst_count++;
								 }}
	;

/*-------------------------------- SALTOS ---------------------------------*/

instr_b	: 
	INSTR_B cond ID						{if (!first_pass) {
								 	br_str = $<string>3;
									branch_cond = b(cond_code);
									inst_count++;
								 }}			
	;

%%

/* Abre los archivos respectivos y los asigna al puntero
   correspondiente. El argumento es el nombre del archivo
   de entrada. Retorna el nombre del archivo de salida en
   ensamblador */

void init_files(char *inputfn) {
	inputf = fopen(inputfn , "r");
	if (!inputf) {
		printf("Error: Archivo inválido.\n");
		exit(-1);
	}
	err_file = fopen("Módulo de Errores Simulador.txt", "w");
	return;
}

/* Función de manejo de errores por defecto de yacc. La entrada es el mensaje que se desea dar
   cuando ocurre un error. En este caso se implementó de manera tal que se impriman todos los 
   mensajes en un archivo de texto. */

void yyerror (const char *s) {
	if (err_bool == 0) err_bool = 1;
	fprintf(err_file,"Error en la línea %d. Error: %s.\n", linenumber, s);
} 

/* Obtiene el valor que está almacenado en un registro y le aplica el desplazamiento
   o rotación correspondiente. */

int get_shifted(int reg_num, int sh_inst, int inmediate)
{
	int shifted = 0;
	if (sh_inst == 0) {shifted = shift_left(reg_num, inmediate);}
	else if (sh_inst == 1) {shifted = shift_right(reg_num, inmediate);}
	else if (sh_inst == 2) {shifted = shift_right(reg_num, inmediate);}
	else if (sh_inst == 3) {shifted = rotate_right(reg_num, inmediate);}
	return shifted;
}

void estimate_time(int count)
{
	float time = count * 10;
	est_time = fopen(".exec_time.txt", "w");
	if (time >= 1000000)
	{
		time = time / 1000000;
		printf("Tiempo aproximado de ejecución: %f s\n", time);
		fprintf(est_time, "Tiempo aproximado de ejecución: %f s\n", time);
	}
	else if (time >= 1000)
	{
		time = time / 1000;
		printf("Tiempo aproximado de ejecución: %f ms\n", time);
		fprintf(est_time, "Tiempo aproximado de ejecución: %f ms\n", time);
	}
	else
	{
		printf("Tiempo aproximado de ejecución: %f µs\n", time);
		fprintf(est_time, "Tiempo aproximado de ejecución: %f µs\n", time);
	}
	fclose(est_time);
}

/* Función principal. Primero se inicializan todos los archivos, con sus respectivas cabeceras.
   Luego se evalúan el archivo de entrada, token por token, hasta agotarlos. Cada vez que coincide
   una regla se ejecutan las acciones que se encuentran a la derecha. Si se provee el argumento -p
   se imprime la tabla de símbolos al archivo "TablaSímbolos.txt". Si existe algún error, se indica 
   mediante un mensaje en consola y se escribe al archivo "Módulo de Errores.txt" y no se genera 
   la salida en ensamblador. Si no hay errores se indica que se realizó la compilación con éxito. */

int main(int argc, char **argv) {
	init();
	
	char line[30];
	struct std_sym* sp;
	init_files(argv[1]);
	yyin = inputf;
	YY_BUFFER_STATE buffer;
	do {
    		yyparse();
	} while (!feof(yyin));
	first_pass = 0;
	linenumber = 1;
	fseek(inputf, 0, SEEK_SET);
	fgets(line, 30, inputf);
	buffer = yy_scan_string(line);
	yy_switch_to_buffer(buffer);
	do {
		buffer = yy_scan_string(line);
		yyparse();
		if (branch_cond) {						// Si debe hacer el salto
			if(exists_symb(br_str) == 1) {				// Si existe la etiqueta
				sp = search(br_str);
        			fseek(inputf, 0, SEEK_SET);
        			for (int i = 1; i <=  sp->dir; ++i) {
          				fgets(line, 30, inputf);
        			}
				linenumber = sp->dir;
      			} else {
        			yyerror("No existe la etiqueta");        
        			break;
      			}    
      			branch_cond = 0;
    		}
		fgets(line, 30, inputf);
		buffer = yy_scan_string(line);
	} while (!feof(inputf));
	yy_delete_buffer(buffer);
	if (err_bool == 1) {
		printf("Error durante la simulación. Ver Módulo de Errores Simulador.txt.\n");
		fclose(err_file);
		return 1;
	}
	fprintf(err_file, "¡No hay errores en el archivo %s!", argv[1]);
	fclose(err_file);
	fclose(inputf);
	printf("Simulado exitosamente.\n\n");
	print_regs();
	print_memory();
	print_flags();
	estimate_time(inst_count);
	foo();
	return 0;
}
