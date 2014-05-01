/* oztree.c */

/*-----------------------------------------------------------------------------
 * Developed by: #undef TEAMNAME
 * Provides full tree implementation for the Oz assembly language.
 *---------------------------------------------------------------------------*/

#include <stdio.h>
#include "ast.h"
#include "symtable.h"
#include "oztree.h"
#include "helper.h"

#define PROGENTRY "main"

const char *sectionnames[] = { COMMENTSTRS };
const char *builtinnames[] = { BUILTINNAMES };

/*-----------------------------------------------------------------------------
 * Function prototypes for internal functions
 *---------------------------------------------------------------------------*/

OzLine *new_line(OzProgram *p);
OzLabel *new_label(OzProgram *p);
OzOp *new_op(OzProgram *p);

void gen_oz_procs(OzProgram *p, Procs *procs, void *tables);
void gen_oz_prologue(OzProgram *p, Params *params, Decls *decls, void *table);
void gen_oz_epilogue(OzProgram *p, void *table);
void gen_oz_params(OzProgram *p, Params *params, void *table);
void gen_oz_decls(OzProgram *p, Decls *decls, void *table);
void gen_oz_stmts(OzProgram *p, Stmts *stmts, void *tables, void *table);
void gen_oz_write(OzProgram *p, Expr *write, void *table);
void gen_oz_expr(OzProgram *p, Expr *expr, void *table, int reg);
void gen_oz_expr_id(OzProgram *p, void *table, int reg, char *id);
void gen_oz_expr_const(OzProgram *p, int reg, Constant *constant);

void gen_comment(OzProgram *p, OzCommentSection section);
void gen_call(OzProgram *p, char *id);
void gen_call_builtin(OzProgram *p, OzBuiltinId id);
void gen_halt(OzProgram *p);
void gen_return(OzProgram *p);
void gen_proc_label(OzProgram *p, char *id);
void gen_int_const(OzProgram *p, int reg, int val);
void gen_real_const(OzProgram *p, int reg, float val);
void gen_string_const(OzProgram *p, int reg, char *val);
void gen_triop(OzProgram *p, OpCode code, int arg1, int arg2, int arg3);
void gen_binop(OzProgram *p, OpCode code, int arg1, int arg2);
void gen_unop(OzProgram *p, OpCode code, int arg1);


/*-----------------------------------------------------------------------------
 * Functions from header file
 *---------------------------------------------------------------------------*/

OzProgram *
gen_oz_program(Program *p, void *tables) {
    OzProgram *ozprog = checked_malloc(sizeof(OzProgram));
    ozprog->start = NULL;
    ozprog->end   = NULL;

    gen_call(ozprog, PROGENTRY);
    gen_halt(ozprog);

    gen_oz_procs(ozprog, p->procedures, tables);

    return ozprog;
}


/*-----------------------------------------------------------------------------
 * Create new Oz structures to represent code
 *---------------------------------------------------------------------------*/

// Add a new line to the end of the program, and return it
OzLine *
new_line(OzProgram *p) {
    OzLine *new_line = checked_malloc(sizeof(OzLine));
    OzLines *lines = checked_malloc(sizeof(OzLines));
    lines->first = new_line;
    lines->rest = NULL;

    if (p->start == NULL) {
        p->start = lines;
        p->end = lines;
    }
    else {
        p->end->rest = lines;
        p->end = lines;
    }

    return new_line;
}

OzOp *
new_op(OzProgram *p) {
    OzOp *new_op = checked_malloc(sizeof(OzOp));
    new_op->arg1 = NULL;
    new_op->arg2 = NULL;
    new_op->arg3 = NULL;

    OzLine *line = new_line(p);
    line->kind = OZ_OP;
    line->val = (void *) new_op;

    return new_op;
}


/*-----------------------------------------------------------------------------
 * Convert Wiz structures into Oz structures
 *---------------------------------------------------------------------------*/

void
gen_oz_procs(OzProgram *p, Procs *procs, void *tables) {
    if (procs == NULL) {
        return; // no more procs
    }

    Proc *proc = procs->first;
    void *table = find_symtable(tables, proc->header->id);

    gen_proc_label(p, proc->header->id);
    gen_oz_prologue(p, proc->header->params, proc->body->decls, table);

    gen_oz_epilogue(p, table);

    gen_oz_procs(p, procs->rest, tables);
}

void
gen_oz_prologue(OzProgram *p, Params *params, Decls *decls, void *table) {
    gen_comment(p, SECTION_PROLOGUE);
    gen_unop(p, OP_PUSH_STACK_FRAME, slots_needed_for_table(table));
    gen_oz_params(p, params, table);
    gen_oz_decls(p, decls, table);
}

void
gen_oz_epilogue(OzProgram *p, void *table) {
    gen_comment(p, SECTION_EPILOGUE);
    gen_unop(p, OP_POP_STACK_FRAME, slots_needed_for_table(table));
    gen_return(p);
}

void
gen_oz_params(OzProgram *p, Params *params, void *table) {
    Param *param;
    Symbol *sym;
    int count = 0;

    while (params != NULL) {
        param = params->first;
        sym = find_symbol_by_id(table, param->id);

        gen_binop(p, OP_STORE, sym->slot, count);

        count++;
        params = params->rest;
    }
}

void
gen_oz_decls(OzProgram *p, Decls *decls, void *table) {
    Decls *ds;
    Decl *decl;
    Symbol *sym;
    int count = 0;

    BOOL ints, reals;
    ints = reals = FALSE;
    int int_reg, real_reg;

    // figure out which registers we need to initialise
    ds = decls;
    while (ds != NULL) {
        decl = ds->first;
        sym = find_symbol_by_id(table, decl->id);

        if (!reals && sym->type == SYM_REAL) {
            reals = TRUE;
            real_reg = count++;
        }

        else if (!ints) {
            ints = TRUE;
            int_reg = count++;
        }

        ds = ds->rest;
    }

    // initialise the required registers
    if (ints) {
        gen_int_const(p, int_reg, 0);
    }

    if (reals) {
        gen_real_const(p, real_reg, 0.0f);
    }

    // initialise the variables in the stack slots
    ds = decls;
    while (ds != NULL) {
        decl = ds->first;
        sym = find_symbol_by_id(table, decl->id);

        if (sym->type == SYM_REAL) {
            gen_binop(p, OP_STORE, sym->slot, real_reg);
        } else {
            gen_binop(p, OP_STORE, sym->slot, int_reg);
        }

        ds = ds->rest;
    }
}

void
gen_oz_stmts(OzProgram *p, Stmts *stmts, void *tables, void *table) {
    if (stmts == NULL) {
        return; //no more statements
    }

    Stmt *stmt = stmts->first;

    switch(stmt->kind) {
        case STMT_WRITE:
            gen_oz_write(p, stmt->info.write, table);
            break;

        default:
            report_error_and_exit("cannot generate for statement!");
    }

    gen_oz_stmts(p, stmts->rest, tables, table);
}

void
gen_oz_write(OzProgram *p, Expr *write, void *table) {
    gen_oz_expr(p, write, table, 0);

    switch(write->inferred_type) {
        case BOOL_TYPE:
            gen_call_builtin(p, BUILTIN_PRINT_BOOL);
            break;

        case INT_TYPE:
            gen_call_builtin(p, BUILTIN_PRINT_INT);
            break;

        case FLOAT_TYPE:
            gen_call_builtin(p, BUILTIN_PRINT_REAL);
            break;

        case STRING_CONST:
            gen_call_builtin(p, BUILTIN_PRINT_STRING);
            break;

        default:
            report_error_and_exit("cannot infer type for write!");
    }
}

void
gen_oz_expr(OzProgram *p, Expr *expr, void *table, int reg) {
    switch(expr->kind) {
        case EXPR_ID:
            gen_oz_expr_id(p, table, reg, expr->id);
            break;

        case EXPR_CONST:
            gen_oz_expr_const(p, reg, &(expr->constant));
            break;

        case EXPR_BINOP:

            break;

        case EXPR_UNOP:

            break;

        case EXPR_ARRAY:
            
            break;
    }
}

void
gen_oz_expr_id(OzProgram *p, void *table, int reg, char *id) {
    Symbol *sym = find_symbol_by_id(table, id); 

    if (sym->kind == SYM_PARAM_REF) {
        gen_binop(p, OP_LOAD_ADDRESS, reg, sym->slot);
    } else {
        gen_binop(p, OP_LOAD, reg, sym->slot);
    }
}

void
gen_oz_expr_const(OzProgram *p, int reg, Constant *constant) {
    Value *val = &(constant->val);

    switch(constant->type) {
        case BOOL_TYPE:
            gen_int_const(p, reg, val->bool_val);
            break;

        case INT_TYPE:
            gen_int_const(p, reg, val->int_val);
            break;

        case FLOAT_TYPE:
            gen_real_const(p, reg, val->float_val);
            break;

        case STRING_CONST:
            gen_string_const(p, reg, val->string);
            break;

        default:
            report_error_and_exit("invalid expr const type!");
    }
}


/*-----------------------------------------------------------------------------
 * Create Oz command for a particular operation
 *---------------------------------------------------------------------------*/

void
gen_comment(OzProgram *p, OzCommentSection section) {
    OzComment *new_comment = checked_malloc(sizeof(OzComment));
    new_comment->section = section;

    OzLine *line = new_line(p);
    line->kind = OZ_COMMENT;
    line->val = (void *) new_comment;
}

void
gen_call(OzProgram *p, char *id) {
    OzOp *op = new_op(p);
    op->code = OP_CALL;
    op->arg1 = id;
}

void
gen_call_builtin(OzProgram *p, OzBuiltinId id) {
    OzBuiltin *b = checked_malloc(sizeof(OzBuiltin));
    b->id = id;

    OzLine *line = new_line(p);
    line->kind = OZ_BUILTIN;
    line->val = (void *) b;
}

void
gen_halt(OzProgram *p) {
    OzOp *op = new_op(p);
    op->code = OP_HALT;
}

void
gen_return(OzProgram *p) {
    OzOp *op = new_op(p);
    op->code = OP_RETURN;
}

void
gen_proc_label(OzProgram *p, char *id) {
    OzProc *proc = checked_malloc(sizeof(OzProc));
    proc->id = id;

    OzLine *line = new_line(p);
    line->kind = OZ_PROC;
    line->val = (void *) proc;
}

void
gen_label(OzProgram *p, int id) {
    OzLabel *label = checked_malloc(sizeof(OzLabel));
    label->id = id;

    OzLine *line = new_line(p);
    line->kind = OZ_LABEL;
    line->val = (void *) label;
}

void
gen_int_const(OzProgram *p, int reg, int val) {
    int *preg = checked_malloc(sizeof(int));
    *preg = reg;

    int *pval = checked_malloc(sizeof(int));
    *pval = val;

    OzOp *op = new_op(p);
    op->code = OP_INT_CONST;
    op->arg1 = (void *) preg;
    op->arg2 = (void *) pval;
}

void
gen_real_const(OzProgram *p, int reg, float val) {
    int *preg = checked_malloc(sizeof(int));
    *preg = reg;

    int *pval = checked_malloc(sizeof(float));
    *pval = val;

    OzOp *op = new_op(p);
    op->code = OP_REAL_CONST;
    op->arg1 = (void *) preg;
    op->arg2 = (void *) pval;
}

void
gen_string_const(OzProgram *p, int reg, char *val) {
    int *preg = checked_malloc(sizeof(int));
    *preg = reg;

    OzOp *op = new_op(p);
    op->code = OP_STRING_CONST;
    op->arg1 = (void *) preg;
    op->arg2 = (void *) val;
}

void
gen_triop(OzProgram *p, OpCode code, int arg1, int arg2, int arg3) {
    int *p1 = checked_malloc(sizeof(int));
    *p1 = arg1;

    int *p2 = checked_malloc(sizeof(int));
    *p2 = arg2;

    int *p3 = checked_malloc(sizeof(int));
    *p3 = arg3;

    OzOp *op = new_op(p);
    op->code = code;
    op->arg1 = (void *) p1;
    op->arg2 = (void *) p2;
    op->arg3 = (void *) p3;
}

void
gen_binop(OzProgram *p, OpCode code, int arg1, int arg2) {
    int *p1 = checked_malloc(sizeof(int));
    *p1 = arg1;

    int *p2 = checked_malloc(sizeof(int));
    *p2 = arg2;

    OzOp *op = new_op(p);
    op->code = code;
    op->arg1 = (void *) p1;
    op->arg2 = (void *) p2;
}

void
gen_unop(OzProgram *p, OpCode code, int arg1) {
    int *p1 = checked_malloc(sizeof(int));
    *p1 = arg1;

    OzOp *op = new_op(p);
    op->code = code;
    op->arg1 = (void *) p1;
}

