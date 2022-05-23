SIMPLE(OP_RETURN)
SIMPLE(OP_ADD)
SIMPLE(OP_SUBTRACT)
SIMPLE(OP_MULTIPLY)
SIMPLE(OP_DIVIDE)
SIMPLE(OP_NEGATE)
SIMPLE(OP_MODULO)
SIMPLE(OP_NONE)
SIMPLE(OP_TRUE)
SIMPLE(OP_FALSE)
SIMPLE(OP_NOT)
SIMPLE(OP_EQUAL)
SIMPLE(OP_GREATER)
SIMPLE(OP_LESS)
SIMPLE(OP_POP)
SIMPLE(OP_INHERIT)
SIMPLE(OP_RAISE)
SIMPLE(OP_CLOSE_UPVALUE)
SIMPLE(OP_DOCSTRING)
SIMPLE(OP_BITOR)
SIMPLE(OP_BITXOR)
SIMPLE(OP_BITAND)
SIMPLE(OP_SHIFTLEFT)
SIMPLE(OP_SHIFTRIGHT)
SIMPLE(OP_BITNEGATE)
SIMPLE(OP_INVOKE_GETTER)
SIMPLE(OP_INVOKE_SETTER)
SIMPLE(OP_INVOKE_DELETE)
SIMPLE(OP_INVOKE_ITER)
SIMPLE(OP_INVOKE_CONTAINS)
SIMPLE(OP_INVOKE_AWAIT)
SIMPLE(OP_SWAP)
SIMPLE(OP_FINALIZE)
SIMPLE(OP_IS)
SIMPLE(OP_POW)
SIMPLE(OP_CLEANUP_WITH)
SIMPLE(OP_FILTER_EXCEPT)
SIMPLE(OP_BREAKPOINT)
SIMPLE(OP_YIELD)
SIMPLE(OP_ANNOTATE)
SIMPLE(OP_BEGIN_FINALLY)
SIMPLE(OP_END_FINALLY)
SIMPLE(OP_GREATER_EQUAL)
SIMPLE(OP_LESS_EQUAL)
SIMPLE(OP_FLOORDIV)
SIMPLE(OP_UNSET)
CONSTANT(OP_DEFINE_GLOBAL,(void)0)
CONSTANT(OP_CONSTANT,(void)0)
CONSTANT(OP_GET_GLOBAL,(void)0)
CONSTANT(OP_SET_GLOBAL,(void)0)
CONSTANT(OP_DEL_GLOBAL,(void)0)
CONSTANT(OP_CLASS,(void)0)
CONSTANT(OP_GET_PROPERTY, (void)0)
CONSTANT(OP_SET_PROPERTY, (void)0)
CONSTANT(OP_DEL_PROPERTY,(void)0)
CONSTANT(OP_CLASS_PROPERTY, (void)0)
CONSTANT(OP_CLOSURE, CLOSURE_MORE)
CONSTANT(OP_IMPORT, (void)0)
CONSTANT(OP_IMPORT_FROM, (void)0)
CONSTANT(OP_GET_SUPER, (void)0)
CONSTANT(OP_GET_METHOD, (void)0)
OPERAND(OP_KWARGS, (void)0)
OPERAND(OP_SET_LOCAL, LOCAL_MORE)
OPERAND(OP_GET_LOCAL, LOCAL_MORE)
OPERAND(OP_SET_LOCAL_POP, LOCAL_MORE)
OPERAND(OP_SET_UPVALUE, (void)0)
OPERAND(OP_GET_UPVALUE, (void)0)
OPERAND(OP_CALL, (void)0)
OPERAND(OP_TUPLE, (void)0)
OPERAND(OP_UNPACK, (void)0)
OPERAND(OP_DUP,(void)0)
OPERAND(OP_EXPAND_ARGS,EXPAND_ARGS_MORE)
OPERAND(OP_LIST_APPEND, (void)0)
OPERAND(OP_DICT_SET, (void)0)
OPERAND(OP_SET_ADD, (void)0)
OPERAND(OP_MAKE_LIST, (void)0)
OPERAND(OP_MAKE_DICT, (void)0)
OPERAND(OP_MAKE_SET, (void)0)
OPERAND(OP_REVERSE, (void)0)
OPERAND(OP_SLICE, (void)0)
OPERAND(OP_CALL_METHOD, (void)0)
JUMP(OP_JUMP_IF_FALSE_OR_POP,+)
JUMP(OP_JUMP_IF_TRUE_OR_POP,+)
JUMP(OP_JUMP,+)
JUMP(OP_LOOP,-)
JUMP(OP_PUSH_TRY,+)
JUMP(OP_PUSH_WITH,+)
JUMP(OP_YIELD_FROM,+)
JUMP(OP_CALL_ITER,+)
JUMP(OP_LOOP_ITER,-)
JUMP(OP_POP_JUMP_IF_FALSE,+)
