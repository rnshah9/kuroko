#include <stdarg.h>
#include <string.h>
#include "vm.h"
#include "debug.h"
#include "memory.h"
#include "compiler.h"
#include "object.h"
#include "table.h"

/* Why is this static... why do we do this to ourselves... */
KrkVM vm;

static void resetStack() {
	vm.stackTop = vm.stack;
}

static void runtimeError(const char * fmt, ...) {
	va_list args;
	va_start(args, fmt);
	vfprintf(stderr, fmt, args);
	va_end(args);
	fprintf(stderr, "\n");
	size_t instruction = vm.ip - vm.chunk->code - 1;
	size_t line = vm.chunk->lines[instruction];
	fprintf(stderr, "[line %d] in script\n", (int)line);
	resetStack();
}

void krk_push(KrkValue value) {
	if ((size_t)(vm.stackTop - vm.stack) + 1 > vm.stackSize) {
		size_t old = vm.stackSize;
		size_t old_offset = vm.stackTop - vm.stack;
		vm.stackSize = GROW_CAPACITY(old);
		vm.stack = GROW_ARRAY(KrkValue, vm.stack, old, vm.stackSize);
		vm.stackTop = vm.stack + old_offset;
	}
	*vm.stackTop = value;
	vm.stackTop++;
}

KrkValue krk_pop() {
	vm.stackTop--;
	if (vm.stackTop < vm.stack) {
		fprintf(stderr, "XXX: Stack overflow?");
	}
	return *vm.stackTop;
}

KrkValue krk_peep(int distance) {
	return vm.stackTop[-1 - distance];
}

void krk_initVM() {
	resetStack();
	vm.objects = NULL;
	krk_initTable(&vm.globals);
	krk_initTable(&vm.strings);
}

void krk_freeVM() {
	krk_freeTable(&vm.globals);
	krk_freeTable(&vm.strings);
	krk_freeObjects();
	FREE_ARRAY(size_t, vm.stack, vm.stackSize);
}

static int isFalsey(KrkValue value) {
	return IS_NONE(value) || (IS_BOOLEAN(value) && !AS_BOOLEAN(value)) ||
	       (IS_INTEGER(value) && !AS_INTEGER(value));
	/* Objects in the future: */
	/* IS_STRING && length == 0; IS_ARRAY && length == 0; IS_INSTANCE && __bool__ returns 0... */
}

const char * typeName(KrkValue value) {
	if (value.type == VAL_BOOLEAN) return "Boolean";
	if (value.type == VAL_NONE) return "None";
	if (value.type == VAL_INTEGER) return "Integer";
	if (value.type == VAL_FLOATING) return "Floating";
	if (value.type == VAL_OBJECT) {
		if (IS_STRING(value)) return "String";
		return "(Unspecified Object)";
	}
	return "???";
}

#define MAKE_BIN_OP(name,operator) \
	static KrkValue name (KrkValue a, KrkValue b) { \
		if (IS_INTEGER(a) && IS_INTEGER(b)) return INTEGER_VAL(AS_INTEGER(a) operator AS_INTEGER(b)); \
		if (IS_FLOATING(a)) { \
			if (IS_INTEGER(b)) return FLOATING_VAL(AS_FLOATING(a) operator (double)AS_INTEGER(b)); \
			else if (IS_FLOATING(b)) return FLOATING_VAL(AS_FLOATING(a) operator AS_FLOATING(b)); \
		} else if (IS_FLOATING(b)) { \
			if (IS_INTEGER(a)) return FLOATING_VAL((double)AS_INTEGER(a) operator AS_FLOATING(b)); \
		} \
		runtimeError("Incompatible types for binary operand %s: %s and %s", #operator, typeName(a), typeName(b)); \
		return NONE_VAL(); \
	}

MAKE_BIN_OP(add,+)
MAKE_BIN_OP(subtract,-)
MAKE_BIN_OP(multiply,*)
MAKE_BIN_OP(divide,/)

#define MAKE_COMPARATOR(name, operator) \
	static KrkValue name (KrkValue a, KrkValue b) { \
		if (IS_INTEGER(a) && IS_INTEGER(b)) return BOOLEAN_VAL(AS_INTEGER(a) operator AS_INTEGER(b)); \
		if (IS_FLOATING(a)) { \
			if (IS_INTEGER(b)) return BOOLEAN_VAL(AS_FLOATING(a) operator AS_INTEGER(b)); \
			else if (IS_FLOATING(b)) return BOOLEAN_VAL(AS_FLOATING(a) operator AS_FLOATING(b)); \
		} else if (IS_FLOATING(b)) { \
			if (IS_INTEGER(a)) return BOOLEAN_VAL(AS_INTEGER(a) operator AS_INTEGER(b)); \
		} \
		runtimeError("Can not compare types %s and %s", typeName(a), typeName(b)); \
		return NONE_VAL(); \
	}

MAKE_COMPARATOR(less, <)
MAKE_COMPARATOR(greater, >)

static void concatenate(const char * a, const char * b, size_t al, size_t bl) {
	size_t length = al + bl;
	char * chars = ALLOCATE(char, length + 1);
	memcpy(chars, a, al);
	memcpy(chars + al, b, bl);
	chars[length] = '\0';

	KrkString * result = takeString(chars, length);
	krk_push(OBJECT_VAL(result));
}

static void addObjects() {
	KrkValue _b = krk_pop();
	KrkValue _a = krk_pop();

	if (IS_STRING(_a)) {
		KrkString * a = AS_STRING(_a);
		if (IS_STRING(_b)) {
			KrkString * b = AS_STRING(_b);
			concatenate(a->chars,b->chars,a->length,b->length);
			return;
		}
		char tmp[256];
		if (IS_INTEGER(_b)) {
			sprintf(tmp, "%d", AS_INTEGER(_b));
		} else if (IS_FLOATING(_b)) {
			sprintf(tmp, "%g", AS_FLOATING(_b));
		} else if (IS_BOOLEAN(_b)) {
			sprintf(tmp, "%s", AS_BOOLEAN(_b) ? "True" : "False");
		} else if (IS_NONE(_b)) {
			sprintf(tmp, "None");
		} else {
			sprintf(tmp, "<Object>");
		}
		concatenate(a->chars,tmp,a->length,strlen(tmp));
	} else {
		runtimeError("Can not concatenate types %s and %s", typeName(_a), typeName(_b)); \
		krk_push(NONE_VAL());
	}
}

#define DEBUG

#define READ_BYTE() (*vm.ip++)
#define BINARY_OP(op) { KrkValue b = krk_pop(); KrkValue a = krk_pop(); krk_push(op(a,b)); break; }
#define READ_CONSTANT(s) (vm.chunk->constants.values[readBytes(s)])
#define READ_STRING(s) AS_STRING(READ_CONSTANT(s))

static size_t readBytes(int num) {
	if (num == 1) return READ_BYTE();
	else if (num == 2) {
		unsigned int top = READ_BYTE();
		unsigned int bot = READ_BYTE();
		return (top << 8) | (bot);
	} else if (num == 3) {
		unsigned int top = READ_BYTE();
		unsigned int mid = READ_BYTE();
		unsigned int bot = READ_BYTE();
		return (top << 16) | (mid << 8) | (bot);
	}
	return 0;
}

static KrkValue run() {

	for (;;) {
#ifdef DEBUG
		fprintf(stderr, "        | ");
		for (KrkValue * slot = vm.stack; slot < vm.stackTop; slot++) {
			fprintf(stderr, "[ ");
			krk_printValue(stderr, *slot);
			fprintf(stderr, " ]");
		}
		fprintf(stderr, "\n");
		krk_disassembleInstruction(vm.chunk, (size_t)(vm.ip - vm.chunk->code));
#endif
		uint8_t opcode;
		switch ((opcode = READ_BYTE())) {
			case OP_PRINT: {
				krk_printValue(stdout, krk_pop());
				fprintf(stdout, "\n");
				break;
			}
			case OP_RETURN: {
				krk_printValue(stdout, krk_pop());
				fprintf(stdout, "\n");
				return INTEGER_VAL(0);
			}
			case OP_EQUAL: {
				KrkValue b = krk_pop();
				KrkValue a = krk_pop();
				krk_push(BOOLEAN_VAL(krk_valuesEqual(a,b)));
				break;
			}
			case OP_GREATER: BINARY_OP(greater)
			case OP_ADD:
				if (IS_OBJECT(krk_peep(0)) || IS_OBJECT(krk_peep(1))) addObjects();
				else BINARY_OP(add)
				break;
			case OP_SUBTRACT: BINARY_OP(subtract)
			case OP_MULTIPLY: BINARY_OP(multiply)
			case OP_DIVIDE: BINARY_OP(divide)
			case OP_NEGATE: {
				KrkValue value = krk_pop();
				if (IS_INTEGER(value)) krk_push(INTEGER_VAL(-AS_INTEGER(value)));
				else if (IS_FLOATING(value)) krk_push(FLOATING_VAL(-AS_FLOATING(value)));
				else { runtimeError("Incompatible operand type for prefix negation."); return NONE_VAL(); }
				break;
			}
			case OP_CONSTANT_LONG:
			case OP_CONSTANT: {
				size_t index = readBytes(opcode == OP_CONSTANT ? 1 : 3);
				KrkValue constant = vm.chunk->constants.values[index];
				krk_push(constant);
				break;
			}
			case OP_NONE:  krk_push(NONE_VAL()); break;
			case OP_TRUE:  krk_push(BOOLEAN_VAL(1)); break;
			case OP_FALSE: krk_push(BOOLEAN_VAL(0)); break;
			case OP_NOT:   krk_push(BOOLEAN_VAL(isFalsey(krk_pop()))); break;
			case OP_POP:   krk_pop(); break;
			case OP_DEFINE_GLOBAL_LONG:
			case OP_DEFINE_GLOBAL: {
				KrkString * name = READ_STRING((opcode == OP_DEFINE_GLOBAL ? 1 : 3));
				krk_tableSet(&vm.globals, OBJECT_VAL(name), krk_peep(0));
				krk_pop();
				break;
			}
			case OP_GET_GLOBAL_LONG:
			case OP_GET_GLOBAL: {
				KrkString * name = READ_STRING((opcode == OP_GET_GLOBAL ? 1 : 3));
				KrkValue value;
				if (!krk_tableGet(&vm.globals, OBJECT_VAL(name), &value)) {
					runtimeError("Undefined variable '%s'.", name->chars);
					return NONE_VAL();
				}
				krk_push(value);
				break;
			}
			case OP_SET_GLOBAL_LONG:
			case OP_SET_GLOBAL: {
				KrkString * name = READ_STRING((opcode == OP_SET_GLOBAL ? 1 : 3));
				if (krk_tableSet(&vm.globals, OBJECT_VAL(name), krk_peep(0))) {
					krk_tableDelete(&vm.globals, OBJECT_VAL(name));
					/* TODO: This should probably just work as an assignment? */
					runtimeError("Undefined variable '%s'.", name->chars);
					return NONE_VAL();
				}
				break;
			}
			case OP_GET_LOCAL_LONG:
			case OP_GET_LOCAL: {
				uint32_t slot = readBytes((opcode == OP_GET_LOCAL ? 1 : 3));
				krk_push(vm.stack[slot]);
				break;
			}
			case OP_SET_LOCAL_LONG:
			case OP_SET_LOCAL: {
				uint32_t slot = readBytes((opcode == OP_SET_LOCAL ? 1 : 3));
				vm.stack[slot] = krk_peep(0);
				break;
			}
			case OP_JUMP_IF_FALSE: {
				uint16_t offset = readBytes(2);
				if (isFalsey(krk_peep(0))) vm.ip += offset;
				break;
			}
			case OP_JUMP_IF_TRUE: {
				uint16_t offset = readBytes(2);
				if (!isFalsey(krk_peep(0))) vm.ip += offset;
				break;
			}
			case OP_JUMP: {
				vm.ip += readBytes(2);
				break;
			}
			case OP_LOOP: {
				uint16_t offset = readBytes(2);
				vm.ip -= offset;
				break;
			}
		}
	}

#undef BINARY_OP
#undef READ_BYTE
}

int krk_interpret(const char * src) {
	KrkChunk chunk;
	krk_initChunk(&chunk);
	if (!krk_compile(src, &chunk)) {
		krk_freeChunk(&chunk);
		return 1;
	}
	vm.chunk = &chunk;
	vm.ip = vm.chunk->code;

	KrkValue result = run();
	krk_freeChunk(&chunk);
	return IS_NONE(result);
}