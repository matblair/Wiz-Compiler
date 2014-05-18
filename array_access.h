/* array_access.h */

/*-----------------------------------------------------------------------
    Developed by: #undef TEAMNAME

    provides data structure for mediating efficient array access,
    and function for constructing this structure from an array
    expression
-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------
    data structure that contains all required information for run-time
    array access
    - flat array index can be computed by adding static_offset to
      each of the expressions in dynamic_offsets
    - dynamic_bounds gives the access bounds for each of the dynamic
      indices, and is_in_static_bounds indicates whether the access
      from statically determined indices is in bounds or not
-----------------------------------------------------------------------*/
typedef struct {
    int         static_offset;
    BOOL        is_in_static_bounds;
    Exprs       *dynamic_offsets;
    Intervals   *dynamic_bounds;
} ArrayAccess;


/*-----------------------------------------------------------------------
    function for obtaining the array access structure for a given
    array expression
-----------------------------------------------------------------------*/
ArrayAccess *get_array_access(Expr *expr, Bounds *bounds);
