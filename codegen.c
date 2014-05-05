/* codegen.c */

/*-----------------------------------------------------------------------------
 * Developed by: #undef TEAMNAME
 * Compiles a Wiz program to Oz
 *---------------------------------------------------------------------------*/

#include <stdio.h>
#include "ast.h"
#include "symbol.h"
#include "analyse.h"
#include "oztree.h"
#include "error_printer.h"
#include "helper.h"

#define PROGENTRY "main"
#define INDENTS "    "
#define INSTRWIDTH (-16)

// /*-----------------------------------------------------------------------------
//  * Function prototypes for internal functions
//  *---------------------------------------------------------------------------*/

void print_lines(FILE *, OzLines *);
void print_op(FILE *, OzOp *);


// /*-----------------------------------------------------------------------------
//  * Functions from header file
//  *---------------------------------------------------------------------------*/

// // Compiles a Wiz program to Oz, outputting to fp. Returns 0 for success.
int 
compile(FILE *fp, Program *prog) {
	void *table = analyse(prog);
	if(table == NULL){
		//Then did not pass semantic analysis. Exit
		report_error_and_exit("Invalid program.");
	}
    OzProgram *ozprog = gen_oz_program(prog, table);
    print_lines(fp, ozprog->start);
    return (int)(!ozprog);
}


/*-----------------------------------------------------------------------------
 * Internal functions
 *---------------------------------------------------------------------------*/

void
print_lines(FILE *fp, OzLines *lines) {
    if (lines == NULL) {
        return; // end of program
    }

    switch(lines->first->kind) {
        case OZ_OP:
            print_op(fp, (OzOp *) lines->first->val);
            break;

        case OZ_BUILTIN:
            fprintf(fp, INDENTS);
            fprintf(fp, "%*s %s\n", INSTRWIDTH, "call_builtin",
                    builtinnames[((OzBuiltin *) lines->first->val)->id]);
            break;

        case OZ_PROC:
            fprintf(fp, "proc_%s:\n", ((OzProc *) lines->first->val)->id);
            break;

        case OZ_LABEL:
            fprintf(fp, "label%d:\n", ((OzLabel *) lines->first->val)->id);
            break;

        case OZ_COMMENT:
            fprintf(fp, "# %s\n",
                    sectionnames[((OzComment *) lines->first->val)->section]);
            break;
    }

    print_lines(fp, lines->rest);
}

void
print_op(FILE *fp, OzOp *op) {
    fprintf(fp, INDENTS); // indent all ops

    switch(op->code) {
        case OP_CALL:
            fprintf(fp, "%*s proc_%s\n", INSTRWIDTH, "call", (char *)op->arg1);
            break;

        case OP_HALT:
            fprintf(fp, "halt\n");
            break;

        case OP_PUSH_STACK_FRAME:
            fprintf(fp, "%*s %d\n", INSTRWIDTH, "push_stack_frame",
                    * (int *)op->arg1);
            break;

        case OP_POP_STACK_FRAME:
            fprintf(fp, "%*s %d\n", INSTRWIDTH, "pop_stack_frame",
                    * (int *)op->arg1);
            break;

        case OP_RETURN:
            fprintf(fp, "%*s\n", INSTRWIDTH, "return");
            break;

        case OP_BRANCH_ON_TRUE:
            fprintf(fp, "%*s r%d, label%d\n", INSTRWIDTH, "branch_on_true",
                    * (int *)op->arg1, * (int *)op->arg2);
            break;

        case OP_BRANCH_ON_FALSE:
            fprintf(fp, "%*s r%d, label%d\n", INSTRWIDTH, "branch_on_false",
                    * (int *)op->arg1, * (int *)op->arg2);
            break;

        case OP_BRANCH_UNCOND:
            fprintf(fp, "%*s label%d\n", INSTRWIDTH, "branch_uncond",
                    * (int *)op->arg1);
            break;

        case OP_LOAD:
            fprintf(fp, "%*s r%d, %d\n", INSTRWIDTH, "load",
                    * (int *)op->arg1, * (int *)op->arg2);
            break;

        case OP_STORE:
            fprintf(fp, "%*s %d, r%d\n", INSTRWIDTH, "store",
                    * (int *)op->arg1, * (int *)op->arg2);
            break;

        case OP_LOAD_ADDRESS:
            fprintf(fp, "%*s r%d, %d\n", INSTRWIDTH, "load_address",
                    * (int *)op->arg1, * (int *)op->arg2);
            break;

        case OP_LOAD_INDIRECT:
            fprintf(fp, "%*s r%d, r%d\n", INSTRWIDTH, "load_indirect",
                    * (int *)op->arg1, * (int *)op->arg2);
            break;

        case OP_STORE_INDIRECT:
            fprintf(fp, "%*s r%d, r%d\n", INSTRWIDTH, "store_indirect",
                    * (int *)op->arg1, * (int *)op->arg2);
            break;

        case OP_MOVE:
            fprintf(fp, "%*s r%d, r%d\n", INSTRWIDTH, "move",
                    * (int *)op->arg1, * (int *)op->arg2);

        case OP_INT_CONST:
            fprintf(fp, "%*s r%d, %d\n", INSTRWIDTH, "int_const",
                    * (int *)op->arg1, * (int *)op->arg2);
            break;

        case OP_REAL_CONST:
            fprintf(fp, "%*s r%d, %f\n", INSTRWIDTH, "real_const",
                    * (int *)op->arg1, * (float *)op->arg2);
            break;

        case OP_STRING_CONST:
            fprintf(fp, "%*s r%d, \"%s\"\n", INSTRWIDTH, "string_const",
                    * (int *)op->arg1, (char *)op->arg2);
            break;

        case OP_ADD_INT:
            fprintf(fp, "%*s r%d, r%d, r%d\n", INSTRWIDTH, "add_int",
                    * (int *)op->arg1, * (int *)op->arg2, * (int *)op->arg3);
            break;

        case OP_ADD_REAL:
            fprintf(fp, "%*s r%d, r%d, r%d\n", INSTRWIDTH, "add_real",
                    * (int *)op->arg1, * (int *)op->arg2, * (int *)op->arg3);
            break;
            
        case OP_ADD_OFFSET:
            fprintf(fp, "%*s r%d, r%d, r%d\n", INSTRWIDTH, "add_offset",
                    * (int *)op->arg1, * (int *)op->arg2, * (int *)op->arg3);
            break;

        case OP_SUB_INT:
            fprintf(fp, "%*s r%d, r%d, r%d\n", INSTRWIDTH, "sub_int",
                    * (int *)op->arg1, * (int *)op->arg2, * (int *)op->arg3);
            break;

        case OP_SUB_REAL:
            fprintf(fp, "%*s r%d, r%d, r%d\n", INSTRWIDTH, "sub_real",
                    * (int *)op->arg1, * (int *)op->arg2, * (int *)op->arg3);
            break;
            
        case OP_SUB_OFFSET:
            fprintf(fp, "%*s r%d, r%d, r%d\n", INSTRWIDTH, "sub_offset",
                    * (int *)op->arg1, * (int *)op->arg2, * (int *)op->arg3);
            break;

        case OP_MUL_INT:
            fprintf(fp, "%*s r%d, r%d, r%d\n", INSTRWIDTH, "mul_int",
                    * (int *)op->arg1, * (int *)op->arg2, * (int *)op->arg3);
            break;

        case OP_MUL_REAL:
            fprintf(fp, "%*s r%d, r%d, r%d\n", INSTRWIDTH, "mul_real",
                    * (int *)op->arg1, * (int *)op->arg2, * (int *)op->arg3);
            break;

        case OP_DIV_INT:
            fprintf(fp, "%*s r%d, r%d, r%d\n", INSTRWIDTH, "div_int",
                    * (int *)op->arg1, * (int *)op->arg2, * (int *)op->arg3);
            break;

        case OP_DIV_REAL:
            fprintf(fp, "%*s r%d, r%d, r%d\n", INSTRWIDTH, "div_real",
                    * (int *)op->arg1, * (int *)op->arg2, * (int *)op->arg3);
            break;
            
        case OP_CMP_EQ_INT:
            fprintf(fp, "%*s r%d, r%d, r%d\n", INSTRWIDTH, "cmp_eq_int",
                    * (int *)op->arg1, * (int *)op->arg2, * (int *)op->arg3);
            break;
            
        case OP_CMP_NE_INT:
            fprintf(fp, "%*s r%d, r%d, r%d\n", INSTRWIDTH, "cmp_ne_int",
                    * (int *)op->arg1, * (int *)op->arg2, * (int *)op->arg3);
            break;
            
        case OP_CMP_GT_INT:
            fprintf(fp, "%*s r%d, r%d, r%d\n", INSTRWIDTH, "cmp_gt_int",
                    * (int *)op->arg1, * (int *)op->arg2, * (int *)op->arg3);
            break;
            
        case OP_CMP_GE_INT:
            fprintf(fp, "%*s r%d, r%d, r%d\n", INSTRWIDTH, "cmp_ge_int",
                    * (int *)op->arg1, * (int *)op->arg2, * (int *)op->arg3);
            break;
            
        case OP_CMP_LT_INT:
            fprintf(fp, "%*s r%d, r%d, r%d\n", INSTRWIDTH, "cmp_lt_int",
                    * (int *)op->arg1, * (int *)op->arg2, * (int *)op->arg3);
            break;
            
        case OP_CMP_LE_INT:
            fprintf(fp, "%*s r%d, r%d, r%d\n", INSTRWIDTH, "cmp_le_int",
                    * (int *)op->arg1, * (int *)op->arg2, * (int *)op->arg3);
            break;
            
        case OP_CMP_EQ_REAL:
            fprintf(fp, "%*s r%d, r%d, r%d\n", INSTRWIDTH, "cmp_eq_real",
                    * (int *)op->arg1, * (int *)op->arg2, * (int *)op->arg3);
            break;
            
        case OP_CMP_NE_REAL:
            fprintf(fp, "%*s r%d, r%d, r%d\n", INSTRWIDTH, "cmp_ne_real",
                    * (int *)op->arg1, * (int *)op->arg2, * (int *)op->arg3);
            break;
            
        case OP_CMP_GT_REAL:
            fprintf(fp, "%*s r%d, r%d, r%d\n", INSTRWIDTH, "cmp_gt_real",
                    * (int *)op->arg1, * (int *)op->arg2, * (int *)op->arg3);
            break;
            
        case OP_CMP_GE_REAL:
            fprintf(fp, "%*s r%d, r%d, r%d\n", INSTRWIDTH, "cmp_ge_real",
                    * (int *)op->arg1, * (int *)op->arg2, * (int *)op->arg3);
            break;
            
        case OP_CMP_LT_REAL:
            fprintf(fp, "%*s r%d, r%d, r%d\n", INSTRWIDTH, "cmp_lt_real",
                    * (int *)op->arg1, * (int *)op->arg2, * (int *)op->arg3);
            break;
            
        case OP_CMP_LE_REAL:
            fprintf(fp, "%*s r%d, r%d, r%d\n", INSTRWIDTH, "cmp_le_real",
                    * (int *)op->arg1, * (int *)op->arg2, * (int *)op->arg3);
            break;
            
        case OP_AND:
            fprintf(fp, "%*s r%d, r%d, r%d\n", INSTRWIDTH, "and",
                    * (int *)op->arg1, * (int *)op->arg2, * (int *)op->arg3);
            break;
            
        case OP_OR:
            fprintf(fp, "%*s r%d, r%d, r%d\n", INSTRWIDTH, "or",
                    * (int *)op->arg1, * (int *)op->arg2, * (int *)op->arg3);
            break;

        case OP_NOT:
            fprintf(fp, "%*s r%d, r%d\n", INSTRWIDTH, "not",
                    * (int *)op->arg1, * (int *)op->arg2);
            break;


        case OP_INT_TO_REAL:
           fprintf(fp, "%*s r%d, r%d\n", INSTRWIDTH, "int_to_real",
                    * (int *)op->arg1, * (int *)op->arg2);
            break;

        default:
            report_error_and_exit("operation not yet implemented!");
    }
}
