/* codegen.c */

/*-----------------------------------------------------------------------------
 * Developed by: #undef TEAMNAME
 * Compiles a Wiz program to Oz
 *---------------------------------------------------------------------------*/

#include <stdio.h>
#include "ast.h"
#include "symtable.h"
#include "oztree.h"
#include "helper.h"

#define PROGENTRY "main"
#define INDENTS "    "

/*-----------------------------------------------------------------------------
 * Function prototypes for internal functions
 *---------------------------------------------------------------------------*/

void print_lines(FILE *, OzLines *);
void print_op(FILE *, OzOp *);


/*-----------------------------------------------------------------------------
 * Functions from header file
 *---------------------------------------------------------------------------*/

// Compiles a Wiz program to Oz, outputting to fp. Returns 0 for success.
int
compile(FILE *fp, Program *prog) {
    void *tables      = gen_symtables(prog);
    OzProgram *ozprog = gen_oz_program(prog, tables);

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
            fprintf(fp, "call_builtin %s\n",
                    builtinnames[((OzBuiltin *) lines->first->val)->id]);
            break;

        case OZ_PROC:
            fprintf(fp, "proc_%s\n", ((OzProc *) lines->first->val)->id);
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
            fprintf(fp, "call proc_%s\n", (char *)op->arg1);
            break;

        case OP_HALT:
            fprintf(fp, "halt\n");
            break;

        case OP_PUSH_STACK_FRAME:
            fprintf(fp, "push_stack_frame %d\n", * (int *)op->arg1);
            break;

        case OP_POP_STACK_FRAME:
            fprintf(fp, "pop_stack_frame %d\n", * (int *)op->arg1);
            break;

        case OP_RETURN:
            fprintf(fp, "return\n");
            break;

        case OP_LOAD:
            fprintf(fp, "load r%d, %d\n",
                    * (int *)op->arg1, * (int *)op->arg2);
            break;

        case OP_STORE:
            fprintf(fp, "store %d, r%d\n",
                    * (int *)op->arg1, * (int *)op->arg2);
            break;

        case OP_LOAD_ADDRESS:
            fprintf(fp, "load_address %d, r%d\n",
                    * (int *)op->arg1, * (int *)op->arg2);
            break;

        case OP_LOAD_INDIRECT:
            fprintf(fp, "load_indirect %d, %d\n",
                    * (int *)op->arg1, * (int *)op->arg2);
            break;

        case OP_STORE_INDIRECT:
            fprintf(fp, "store_indirect %d, %d\n",
                    * (int *)op->arg1, * (int *)op->arg2);
            break;

        case OP_MOVE:
            fprintf(fp, "move r%d, r%d\n",
                    * (int *)op->arg1, * (int *)op->arg2);

        case OP_INT_CONST:
            fprintf(fp, "int_const r%d, %d\n",
                    * (int *)op->arg1, * (int *)op->arg2);
            break;

        case OP_REAL_CONST:
            fprintf(fp, "real_const r%d, %f\n",
                    * (int *)op->arg1, * (float *)op->arg2);
            break;

        case OP_STRING_CONST:
            fprintf(fp, "string_const r%d, %s\n",
                    * (int *)op->arg1, (char *)op->arg2);
            break;

        case OP_ADD_INT:
            fprintf(fp, "add_int r%d, r%d, r%d\n",
                    * (int *)op->arg1, * (int *)op->arg2, * (int *)op->arg3);

        case OP_ADD_REAL:
            fprintf(fp, "add_real r%d, r%d, r%d\n",
                    * (int *)op->arg1, * (int *)op->arg2, * (int *)op->arg3);
            
        case OP_ADD_OFFSET:
            fprintf(fp, "add_offset r%d, r%d, r%d\n",
                    * (int *)op->arg1, * (int *)op->arg2, * (int *)op->arg3);

        case OP_SUB_INT:
            fprintf(fp, "sub_int r%d, r%d, r%d\n",
                    * (int *)op->arg1, * (int *)op->arg2, * (int *)op->arg3);

        case OP_SUB_REAL:
            fprintf(fp, "sub_real r%d, r%d, r%d\n",
                    * (int *)op->arg1, * (int *)op->arg2, * (int *)op->arg3);
            
        case OP_SUB_OFFSET:
            fprintf(fp, "sub_offset r%d, r%d, r%d\n",
                    * (int *)op->arg1, * (int *)op->arg2, * (int *)op->arg3);

        case OP_MUL_INT:
            fprintf(fp, "mul_int r%d, r%d, r%d\n",
                    * (int *)op->arg1, * (int *)op->arg2, * (int *)op->arg3);

        case OP_MUL_REAL:
            fprintf(fp, "mul_real r%d, r%d, r%d\n",
                    * (int *)op->arg1, * (int *)op->arg2, * (int *)op->arg3);

        case OP_DIV_INT:
            fprintf(fp, "div_int r%d, r%d, r%d\n",
                    * (int *)op->arg1, * (int *)op->arg2, * (int *)op->arg3);

        case OP_DIV_REAL:
            fprintf(fp, "div_real r%d, r%d, r%d\n",
                    * (int *)op->arg1, * (int *)op->arg2, * (int *)op->arg3);
            
        case OP_CMP_EQ_INT:
            fprintf(fp, "cmp_eq_int r%d, r%d, r%d\n",
                    * (int *)op->arg1, * (int *)op->arg2, * (int *)op->arg3);
            
        case OP_CMP_NE_INT:
            fprintf(fp, "cmp_ne_int r%d, r%d, r%d\n",
                    * (int *)op->arg1, * (int *)op->arg2, * (int *)op->arg3);
            
        case OP_CMP_GT_INT:
            fprintf(fp, "cmp_gt_int r%d, r%d, r%d\n",
                    * (int *)op->arg1, * (int *)op->arg2, * (int *)op->arg3);
            
        case OP_CMP_GE_INT:
            fprintf(fp, "cmp_ge_int r%d, r%d, r%d\n",
                    * (int *)op->arg1, * (int *)op->arg2, * (int *)op->arg3);
            
        case OP_CMP_LT_INT:
            fprintf(fp, "cmp_lt_int r%d, r%d, r%d\n",
                    * (int *)op->arg1, * (int *)op->arg2, * (int *)op->arg3);
            
        case OP_CMP_LE_INT:
            fprintf(fp, "cmp_le_int r%d, r%d, r%d\n",
                    * (int *)op->arg1, * (int *)op->arg2, * (int *)op->arg3);
            
        case OP_CMP_EQ_REAL:
            fprintf(fp, "cmp_eq_real r%d, r%d, r%d\n",
                    * (int *)op->arg1, * (int *)op->arg2, * (int *)op->arg3);
            
        case OP_CMP_NE_REAL:
            fprintf(fp, "cmp_ne_real r%d, r%d, r%d\n",
                    * (int *)op->arg1, * (int *)op->arg2, * (int *)op->arg3);
            
        case OP_CMP_GT_REAL:
            fprintf(fp, "cmp_gt_real r%d, r%d, r%d\n",
                    * (int *)op->arg1, * (int *)op->arg2, * (int *)op->arg3);
            
        case OP_CMP_GE_REAL:
            fprintf(fp, "cmp_ge_real r%d, r%d, r%d\n",
                    * (int *)op->arg1, * (int *)op->arg2, * (int *)op->arg3);
            
        case OP_CMP_LT_REAL:
            fprintf(fp, "cmp_lt_real r%d, r%d, r%d\n",
                    * (int *)op->arg1, * (int *)op->arg2, * (int *)op->arg3);
            
        case OP_CMP_LE_REAL:
            fprintf(fp, "cmp_le_real r%d, r%d, r%d\n",
                    * (int *)op->arg1, * (int *)op->arg2, * (int *)op->arg3);
            
        case OP_AND:
            fprintf(fp, "and r%d, r%d, r%d\n",
                    * (int *)op->arg1, * (int *)op->arg2, * (int *)op->arg3);
            
        case OP_OR:
            fprintf(fp, "or r%d, r%d, r%d\n",
                    * (int *)op->arg1, * (int *)op->arg2, * (int *)op->arg3);

        case OP_NOT:
            fprintf(fp, "not r%d, r%d\n",
                    * (int *)op->arg1, * (int *)op->arg2);


        case OP_INT_TO_REAL:
            fprintf(fp, "int_to_real r%d, r%d\n",
                    * (int *)op->arg1, * (int *)op->arg2);

        default:
            report_error_and_exit("operation not yet implemented!");
    }
}
