#pragma once

#include "kuroko.h"
#include "value.h"

/**
 * Opcodes
 *
 * These are pretty much entirely based on the clox opcodes from the book.
 * There's not really much else to add here, since the VM is sufficient for
 * our needs. Most of the interesting changes happen in the compiler.
 */
typedef enum {
	OP_CONSTANT,
	OP_CONSTANT_LONG,
	OP_NEGATE,
	OP_RETURN,
	OP_ADD,
	OP_SUBTRACT,
	OP_MULTIPLY,
	OP_DIVIDE,
	OP_NONE,
	OP_TRUE,
	OP_FALSE,
	OP_NOT,
	OP_POP,
	OP_EQUAL,
	OP_GREATER,
	OP_LESS,
	OP_PRINT,
	OP_DEFINE_GLOBAL,
	OP_DEFINE_GLOBAL_LONG,
	OP_GET_GLOBAL,
	OP_GET_GLOBAL_LONG,
	OP_SET_GLOBAL,
	OP_SET_GLOBAL_LONG,
	OP_SET_LOCAL,
	OP_SET_LOCAL_LONG,
	OP_GET_LOCAL,
	OP_GET_LOCAL_LONG,
	OP_JUMP_IF_FALSE,
	OP_JUMP_IF_TRUE,
	OP_JUMP,
	OP_LOOP,
} KrkOpCode;

/**
 * Bytecode chunks
 */
typedef struct {
	size_t  count;
	size_t  capacity;
	uint8_t * code;
	size_t * lines;
	KrkValueArray constants;
} KrkChunk;

extern void krk_initChunk(KrkChunk * chunk);
extern void krk_writeChunk(KrkChunk * chunk, uint8_t byte, size_t line);
extern void krk_freeChunk(KrkChunk * chunk);
extern size_t krk_addConstant(KrkChunk * chunk, KrkValue value);
extern void krk_emitConstant(KrkChunk * chunk, size_t ind);
extern size_t krk_writeConstant(KrkChunk * chunk, KrkValue value, size_t line);