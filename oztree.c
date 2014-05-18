/* oztree.c */

/*-----------------------------------------------------------------------------
 * Developed by: #undef TEAMNAME
 * Provides full tree implementation for the Oz assembly language.
 *---------------------------------------------------------------------------*/

#include <stdio.h>
#include <string.h>
#include "ast.h"
#include "symbol.h"
#include "oztree.h"
#include "helper.h"
#include "error_printer.h"
#include "pretty.h"
#include "array_access.h"

#define PROGENTRY "main"

const char *sectionnames[] = { COMMENTSTRS };
const char *builtinnames[] = { BUILTINNAMES };

#define BOUNDS_ERROR "[FATAL]: array element out of bounds!\\n"
#define DIV_ERROR "[FATAL]: division by zero!\\n"

typedef enum {
    OUT_OF_BOUNDS_LABEL, DIV_BY_ZERO_LABEL, FIRST_AVAILABLE_LABEL
} ReservedLabel;

int next_label = FIRST_AVAILABLE_LABEL;


/*-----------------------------------------------------------------------------
 * Function prototypes for internal functions
 *---------------------------------------------------------------------------*/

void gen_oz_procs(OzProgram *p, Procs *procs, void *tables);
void gen_oz_prologue(OzProgram *p, Params *params, Decls *decls, void *table);
void gen_oz_epilogue(OzProgram *p, void *table);
void gen_oz_params(OzProgram *p, Params *params, void *table);
void gen_oz_decls(OzProgram *p, Decls *decls, void *table);
void gen_oz_init_array(OzProgram *p, int slot, int reg, Bounds *bounds);
void gen_oz_out_of_bounds(OzProgram *p);
void gen_oz_div_by_zero(OzProgram *p);

void gen_oz_stmts(OzProgram *p, Stmts *stmts, void *tables, void *table);
void gen_oz_write(OzProgram *p, Expr *write, void *table);
void gen_oz_read(OzProgram *p, Expr *read, void *table);
void gen_oz_assign(OzProgram *p, Assign *assign, void *table);
void gen_oz_call(OzProgram *p, Function *call, void *tables, void *table);
void gen_oz_cond(OzProgram *p, Cond *cond, void *tables, void *table);
void gen_oz_while(OzProgram *p, While *loop, void *tables, void *table);

void gen_oz_expr(OzProgram *p, int reg, Expr *expr, void *table);
void gen_oz_expr_id(OzProgram *p, int reg, char *id, void *table);
void gen_oz_expr_const(OzProgram *p, int reg, Constant *constant);
void gen_oz_expr_array_val(OzProgram *p, int reg, Expr *a, void *table);
void gen_oz_expr_array_addr(OzProgram *p, int reg, Expr *a, void *table);
void gen_oz_expr_binop(OzProgram *p, int reg, Expr *expr, void *table);
void gen_oz_expr_binop_bool(OzProgram *p, int r1, int r2, int r3, Expr *expr);
void gen_oz_expr_binop_int(OzProgram *p, int r1, int r2, int r3, Expr *expr);
void gen_oz_expr_binop_float(OzProgram *p, int r1, int r2, int r3, Expr *expr);
void gen_oz_expr_unop(OzProgram *p, int reg, Expr *expr, void *table);

OzLine *new_line(OzProgram *p);
OzOp *new_op(OzProgram *p);

void gen_comment(OzProgram *p, OzCommentSection section);
void gen_call(OzProgram *p, char *id);
void gen_call_builtin(OzProgram *p, OzBuiltinId id);
void gen_halt(OzProgram *p);
void gen_return(OzProgram *p);
void gen_proc_label(OzProgram *p, char *id);
void gen_label(OzProgram *p, int id);
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
    gen_oz_out_of_bounds(ozprog);
    gen_oz_div_by_zero(ozprog);

    gen_oz_procs(ozprog, p->procedures, tables);

    return ozprog;
}


/*-----------------------------------------------------------------------------
 * Convert high level Wiz stuff into Oz structures
 *---------------------------------------------------------------------------*/

void
gen_oz_procs(OzProgram *p, Procs *procs, void *tables) {
    if (procs == NULL) {
        return; // no more procs
    }

    Proc *proc = procs->first;
    void *table = find_scope(proc->header->id, tables);

    gen_proc_label(p, proc->header->id);
    gen_oz_prologue(p, proc->header->params, proc->body->decls, table);
    gen_oz_stmts(p, proc->body->statements, tables, table);
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
    symbol *sym;
    int count = 0;
    while (params != NULL) {
        param = params->first;
        sym = retrieve_symbol_in_scope(param->id,(scope *)table);

        gen_binop(p, OP_STORE, sym->slot, count);

        count++;
        params = params->rest;
    }
}

void
gen_oz_decls(OzProgram *p, Decls *decls, void *table) {
    Decls *ds;
    Decl *decl;
    symbol *sym;
    int count = 0;
    int reg;

    BOOL ints, reals;
    ints = reals = FALSE;
    int int_reg, real_reg;

    // figure out which registers we need to initialise
    ds = decls;
    while (ds != NULL) {
        decl = ds->first;
          
        sym = retrieve_symbol_in_scope(decl->id, (scope *)table);
        
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
        sym = retrieve_symbol_in_scope(decl->id, (scope *)table);

        if (sym->type == SYM_REAL) {
            reg = real_reg;
        } else {
            reg = int_reg;
        }

        // if not array, just do one, otherwise initalise all stack vars
        if (sym->bounds == NULL) {
             gen_binop(p, OP_STORE, sym->slot, reg);
        } else {
            gen_oz_init_array(p, sym->slot, reg, sym->bounds);
        }

        ds = ds->rest;
    }
}

void
gen_oz_init_array(OzProgram *p, int slot, int reg, Bounds *bounds) {
    int size = bounds->first->upper - bounds->first->lower + 1;

    // deepest down, initialse all the frames
    if (bounds->rest == NULL) {
        for (int i = 0; i < size; i++) {
            gen_binop(p, OP_STORE, slot + i, reg);
        }
    }
    // recursively initialise all the subarrays
    else {
        int offset = bounds->first->offset_size;
        int next_slot;
        for (int i = 0; i < size; i++) {
            next_slot = slot + i * offset;
            gen_oz_init_array(p, next_slot, reg, bounds->rest);
        }
    }
}

// Label to halt the program because someone attempted to access elements
// outside the bounds of an array!
void gen_oz_out_of_bounds(OzProgram *p) {
    int size = strlen(BOUNDS_ERROR) + 1;
    char *msg = checked_malloc(sizeof(char) * size);
    strcpy(msg, BOUNDS_ERROR);

    gen_label(p, OUT_OF_BOUNDS_LABEL);
    gen_string_const(p, 0, msg);
    gen_call_builtin(p, BUILTIN_PRINT_STRING);
    gen_halt(p);
}

// Label to halt the program because someone attempted to divide by zero
void gen_oz_div_by_zero(OzProgram *p) {
    int size = strlen(DIV_ERROR) + 1;
    char *msg = checked_malloc(sizeof(char) * size);
    strcpy(msg, DIV_ERROR);

    gen_label(p, DIV_BY_ZERO_LABEL);
    gen_string_const(p, 0, msg);
    gen_call_builtin(p, BUILTIN_PRINT_STRING);
    gen_halt(p);
}


/*-----------------------------------------------------------------------------
 * Convert Wiz statements into Oz
 *---------------------------------------------------------------------------*/

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

        case STMT_READ:
            gen_oz_read(p, stmt->info.read, table);
            break;

        case STMT_ASSIGN:
            gen_oz_assign(p, &(stmt->info.assign), table);
            break;

        case STMT_FUNC:
            gen_oz_call(p, stmt->info.func, tables, table);
            break;

        case STMT_COND:
            gen_oz_cond(p, &(stmt->info.cond), tables, table);
            break;

        case STMT_WHILE:
            gen_oz_while(p, &(stmt->info.loop), tables, table);
            break;

        default:
            report_error_and_exit("cannot generate for statement!");
    }

    gen_oz_stmts(p, stmts->rest, tables, table);
}

void
gen_oz_write(OzProgram *p, Expr *write, void *table) {
    gen_comment(p, SECTION_WRITE);

    gen_oz_expr(p, 0, write, table);
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
gen_oz_read(OzProgram *p, Expr *read, void *table) {
    gen_comment(p, SECTION_READ);

    symbol *sym = retrieve_symbol_in_scope(read->id, table);

    switch(sym->type) {
        case SYM_BOOL:
            gen_call_builtin(p, BUILTIN_READ_BOOL);
            break;

        case SYM_INT:
            gen_call_builtin(p, BUILTIN_READ_INT);
            break;

        case SYM_REAL:
            gen_call_builtin(p, BUILTIN_READ_REAL);
            break;

        default:
            report_error_and_exit("invalid type to read!");
    }

    if (read->kind == EXPR_ARRAY) {
        gen_oz_expr_array_addr(p, 1, read, table);
        gen_binop(p, OP_STORE_INDIRECT, 1, 0);
    }
    else if (sym->kind == SYM_PARAM_REF) {
        gen_binop(p, OP_LOAD, 1, sym->slot);
        gen_binop(p, OP_STORE_INDIRECT, 1, 0);
    }
    else {
        gen_binop(p, OP_STORE, sym->slot, 0);
    }
}

void
gen_oz_assign(OzProgram *p, Assign *assign, void *table) {
    gen_comment(p, SECTION_ASSIGN);

    symbol *sym = retrieve_symbol_in_scope(assign->asg_ident->id, table);
    Type etype = assign->asg_expr->inferred_type;

    // Evaluate the expression
    gen_oz_expr(p, 0, assign->asg_expr, table);

    // convert to float if needed
    if (sym->type == SYM_REAL && etype == INT_TYPE) {
        gen_binop(p, OP_INT_TO_REAL, 0, 0);
    }

    // Store the value
    if (assign->asg_ident->kind == EXPR_ARRAY){
        gen_oz_expr_array_addr(p, 1, assign->asg_ident, table);
        gen_binop(p, OP_STORE_INDIRECT, 1, 0);
    }
    else if (sym->kind == SYM_PARAM_REF) {
        gen_binop(p, OP_LOAD, 1, sym->slot);
        gen_binop(p, OP_STORE_INDIRECT, 1, 0);
    }
    else {
        gen_binop(p, OP_STORE, sym->slot, 0);
    }
}

void
gen_oz_call(OzProgram *p, Function *call, void *tables, void *table) {
    gen_comment(p, SECTION_CALL);

    scope *call_table = find_scope(call->id, tables);
    Params *params = call_table->params;
    Param *param;
    symbol *arg_sym;
    int reg = 0;
    Exprs *args = call->args;
    Expr *arg;

    // Store all the args in registers
    while (args != NULL) {
        arg = args->first;
        param = params->first;

        // see if we're passing by ref or val
        if (param->ind == REF_IND) {
            arg_sym = retrieve_symbol_in_scope(arg->id, table);

            if (arg_sym->kind == SYM_PARAM_REF) {
                gen_binop(p, OP_LOAD, reg, arg_sym->slot);
            }
            else if (arg->kind == EXPR_ARRAY) {
                gen_oz_expr_array_addr(p, reg, arg, table);
            }
            else {
                gen_binop(p, OP_LOAD_ADDRESS, reg, arg_sym->slot);
            }    
        }
        else {
            gen_oz_expr(p, reg, arg, table);
            // are we passing an int value to a float param?
            if (arg->inferred_type == INT_TYPE && param->type == FLOAT_TYPE) {
                gen_binop(p, OP_INT_TO_REAL, reg, reg);
            }
        }
        
        // other args
        reg++;
        params = params->rest;
        args = args->rest;
    }

    // call the proc
    gen_call(p, call->id);
}

void
gen_oz_cond(OzProgram *p, Cond *cond, void *tables, void *table) {
    gen_comment(p, SECTION_IF);

    int else_label, after_label;
    BOOL else_branch = (cond->else_branch != NULL);

    // set up the labels we need
    if (else_branch) {
        else_label = next_label++;
    }
    after_label = next_label++;

    gen_oz_expr(p, 0, cond->cond, table);
    gen_binop(p, OP_BRANCH_ON_FALSE, 0, else_branch ? else_label : after_label);

    gen_oz_stmts(p, cond->then_branch, tables, table); // then body
    
    if (else_branch) {
        gen_unop(p, OP_BRANCH_UNCOND, after_label);
        gen_label(p, else_label);
        gen_oz_stmts(p, cond->else_branch, tables, table);
    }

    gen_label(p, after_label);                  // exit jump point
}

void
gen_oz_while(OzProgram *p, While *loop, void *tables, void *table) {
    gen_comment(p, SECTION_WHILE);

    int begin_label = next_label++;
    int after_label = next_label++;

    gen_label(p, begin_label);                  // Where the loop begins
    gen_oz_expr(p, 0, loop->cond, table);       // the condition to match
    gen_binop(p, OP_BRANCH_ON_FALSE, 0, after_label); // exit loop if false
    gen_oz_stmts(p, loop->body, tables, table); // the loop body
    gen_unop(p, OP_BRANCH_UNCOND, begin_label); // restart loop
    gen_label(p, after_label);                  // exit jump point
}


/*-----------------------------------------------------------------------------
 * Convert Wiz expressions into Oz structures
 *---------------------------------------------------------------------------*/

void
gen_oz_expr(OzProgram *p, int reg, Expr *expr, void *table) {
    switch(expr->kind) {
        case EXPR_ID:
            gen_oz_expr_id(p, reg, expr->id, table);
            break;

        case EXPR_CONST:
            gen_oz_expr_const(p, reg, &(expr->constant));
            break;

        case EXPR_BINOP:
            gen_oz_expr_binop(p, reg, expr, table);
            break;

        case EXPR_UNOP:
            gen_oz_expr_unop(p, reg, expr, table);
            break;

        case EXPR_ARRAY:
            gen_oz_expr_array_val(p, reg, expr, table);
            break;

        default:
            report_error_and_exit("unknown expr type!");
    }
}

void
gen_oz_expr_id(OzProgram *p, int reg, char *id, void *table) {
    symbol *sym = retrieve_symbol_in_scope(id, table);

    if (sym->kind == SYM_PARAM_REF) {
        gen_binop(p, OP_LOAD_INDIRECT, reg, sym->slot);
    }
    else {
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

// evaluate an array expr, storing value in reg
void
gen_oz_expr_array_val(OzProgram *p, int reg, Expr *a, void *table) {
    gen_oz_expr_array_addr(p, reg, a, table);
    gen_binop(p, OP_LOAD_INDIRECT, reg, reg);
}

// store the address of an array value in a register
void
gen_oz_expr_array_addr(OzProgram *p, int reg, Expr *a, void *table) {
    symbol *sym = retrieve_symbol_in_scope(a->id, table);
    Bounds *bounds = sym->bounds;
    ArrayAccess *array_access = get_array_access(a, bounds);
    Exprs *dynamic_offsets = array_access->dynamic_offsets;
    Intervals *dynamic_bounds = array_access->dynamic_bounds;

    // default to static offset
    gen_int_const(p, reg, array_access->static_offset); 

    // check if we are in static bounds, if not create jump to out of bounds
    // and do no more compilation for this access
    if (!array_access->is_in_static_bounds) {
        gen_unop(p, OP_BRANCH_UNCOND, OUT_OF_BOUNDS_LABEL);
        return;
    }

    // calculate the dynamic offsets we want to apply, iteratively adding
    // them to the total offset, and doing dynamic bounds checking for
    // each dynamic offset
    while(dynamic_offsets != NULL) {
        Expr *dynamic_offset = dynamic_offsets->first;
        Interval *bounds = dynamic_bounds->first;
        // calculate the dynamic offset:
        gen_oz_expr(p, reg + 1, dynamic_offset, table);

        // check that it is in bounds
        // offset < min_offset
        gen_int_const(p, reg + 2, bounds->lower);
        gen_triop(p, OP_CMP_LT_INT, reg + 2, reg + 1, reg + 2);
        gen_binop(p, OP_BRANCH_ON_TRUE, reg + 2, OUT_OF_BOUNDS_LABEL);
        // offset > max_offset
        gen_int_const(p, reg + 2, bounds->upper);
        gen_triop(p, OP_CMP_GT_INT, reg + 2, reg + 1, reg + 2);
        gen_binop(p, OP_BRANCH_ON_TRUE, reg + 2, OUT_OF_BOUNDS_LABEL);

        // add to the total offset so far
        gen_triop(p, OP_ADD_INT, reg, reg, reg + 1);

        //advance the lists we are iterating through
        dynamic_offsets = dynamic_offsets->rest;
        dynamic_bounds = dynamic_bounds->rest;
    }

    // access the array element
    gen_binop(p, OP_LOAD_ADDRESS, reg + 1, sym->slot);
    gen_triop(p, OP_SUB_OFFSET, reg, reg + 1, reg);
}


/*-----------------------------------------------------------------------------
 * Convert Wiz binary/unary operations into Oz structures
 *---------------------------------------------------------------------------*/

void
gen_oz_expr_binop(OzProgram *p, int reg, Expr *expr, void *table) {
    int e1type = expr->e1->inferred_type;
    int e2type = expr->e2->inferred_type;

    // Eval sub expressions
    gen_oz_expr(p, reg, expr->e1, table);
    gen_oz_expr(p, reg + 1, expr->e2, table);

    // check for div by 0
    if (expr->binop == BINOP_DIV) {
        if (e2type == FLOAT_TYPE) {
            gen_real_const(p, reg + 2, 0.0f);
            gen_triop(p, OP_CMP_EQ_REAL, reg + 2, reg + 2, reg + 1);
        } else {
            gen_int_const(p, reg + 2, 0);
            gen_triop(p, OP_CMP_EQ_INT, reg + 2, reg + 2, reg + 1);
        }
        gen_binop(p, OP_BRANCH_ON_TRUE, reg + 2, DIV_BY_ZERO_LABEL);
    }

    // deal with operations with both int and float
    if (e1type == INT_TYPE && e2type == FLOAT_TYPE) {
        gen_binop(p, OP_INT_TO_REAL, reg, reg);
    }
    else if (e1type == FLOAT_TYPE && e2type == INT_TYPE) {
        gen_binop(p, OP_INT_TO_REAL, reg + 1, reg + 1);
    }

    // generate the code
    if (e1type == INT_TYPE && e2type == INT_TYPE) {
        gen_oz_expr_binop_int(p, reg, reg, reg + 1, expr);
    }
    else if (e1type == FLOAT_TYPE || e2type == FLOAT_TYPE) {
        gen_oz_expr_binop_float(p, reg, reg, reg + 1, expr);

    } else {
        gen_oz_expr_binop_bool(p, reg, reg, reg + 1, expr);

    }
}

void
gen_oz_expr_binop_bool(OzProgram *p, int r1, int r2, int r3, Expr *expr) {
    switch(expr->binop) {
        case BINOP_OR:
            gen_triop(p, OP_OR, r1, r2, r3);
            break;

        case BINOP_AND:
            gen_triop(p, OP_AND, r1, r2, r3);
            break;

        default:
            print_expression(stderr, expr,0);

            report_error_and_exit("invalid op for bool binop expr!");
    }
}

void
gen_oz_expr_binop_int(OzProgram *p, int r1, int r2, int r3, Expr *expr) {
    switch(expr->binop) {
        case BINOP_ADD:
            gen_triop(p, OP_ADD_INT, r1, r2, r3);
            break;

        case BINOP_SUB:
            gen_triop(p, OP_SUB_INT, r1, r2, r3);
            break;

        case BINOP_MUL:
            gen_triop(p, OP_MUL_INT, r1, r2, r3);
            break;

        case BINOP_DIV:
            gen_triop(p, OP_DIV_INT, r1, r2, r3);
            break;

        case BINOP_EQ:
            gen_triop(p, OP_CMP_EQ_INT, r1, r2, r3);
            break;

        case BINOP_NTEQ:
            gen_triop(p, OP_CMP_NE_INT, r1, r2, r3);
            break;

        case BINOP_LT:
            gen_triop(p, OP_CMP_LT_INT, r1, r2, r3);
            break;

        case BINOP_LTEQ:
            gen_triop(p, OP_CMP_LE_INT, r1, r2, r3);
            break;

        case BINOP_GT:
            gen_triop(p, OP_CMP_GT_INT, r1, r2, r3);
            break;

        case BINOP_GTEQ:
            gen_triop(p, OP_CMP_GE_INT, r1, r2, r3);
            break;

        default:
            report_error_and_exit("invalid op for int binop expr!");
    }
}

void
gen_oz_expr_binop_float(OzProgram *p, int r1, int r2, int r3, Expr *expr) {
    switch(expr->binop) {
        case BINOP_ADD:
            gen_triop(p, OP_ADD_REAL, r1, r2, r3);
            break;

        case BINOP_SUB:
            gen_triop(p, OP_SUB_REAL, r1, r2, r3);
            break;

        case BINOP_MUL:
            gen_triop(p, OP_MUL_REAL, r1, r2, r3);
            break;

        case BINOP_DIV:
            gen_triop(p, OP_DIV_REAL, r1, r2, r3);
            break;

        case BINOP_EQ:
            gen_triop(p, OP_CMP_EQ_REAL, r1, r2, r3);
            break;

        case BINOP_NTEQ:
            gen_triop(p, OP_CMP_NE_REAL, r1, r2, r3);
            break;

        case BINOP_LT:
            gen_triop(p, OP_CMP_LT_REAL, r1, r2, r3);
            break;

        case BINOP_LTEQ:
            gen_triop(p, OP_CMP_LE_REAL, r1, r2, r3);
            break;

        case BINOP_GT:
            gen_triop(p, OP_CMP_GT_REAL, r1, r2, r3);
            break;

        case BINOP_GTEQ:
            gen_triop(p, OP_CMP_GE_REAL, r1, r2, r3);
            break;

        default:
            report_error_and_exit("invalid op for float binop expr!");
    }
}

void
gen_oz_expr_unop(OzProgram *p, int reg, Expr *expr, void *table) {
    Type t = expr->inferred_type;

    // Eval sub expression
    gen_oz_expr(p,reg, expr->e1, table);

    // Do we need to worry about converting float to int?
    if (t == FLOAT_TYPE && expr->e1->inferred_type == INT_TYPE) {
        gen_binop(p, OP_INT_TO_REAL, reg, reg);
    }

    // generate the op of this expr
    if (t == BOOL_TYPE && expr->unop == UNOP_NOT) {
        gen_binop(p, OP_NOT, reg, reg);
    }

    else if (t == INT_TYPE && expr->unop == UNOP_MINUS) {
        gen_int_const(p, reg + 1, 0);
        gen_triop(p, OP_SUB_INT, reg, reg + 1, reg);
    }

    else if (t == FLOAT_TYPE && expr->unop == UNOP_MINUS) {
        gen_real_const(p, reg + 1, 0.0f);
        gen_triop(p, OP_SUB_REAL, reg, reg + 1, reg);
    }

    else {
        fprintf(stderr,"Type is %s for %s\n", typenames[t], expr->id);
        report_error_and_exit("invalid op for unop expr!");
    }
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

