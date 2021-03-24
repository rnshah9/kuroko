#include <string.h>
#include <kuroko/vm.h>
#include <kuroko/object.h>
#include <kuroko/memory.h>
#include <kuroko/util.h>
#include <kuroko/bigint.h>

static KrkClass * bigint;

struct bigint {
	KrkInstance inst;
	int     sign;
	size_t  size;
	uint32_t *digits;
};

static void bigint_ongcsweep(KrkInstance * _self) {
	struct bigint * self = (struct bigint*)_self;
	if (self->digits) free(self->digits);
}

#define AS_bigint(o) ((struct bigint*)AS_OBJECT(o))
#define IS_bigint(o) (krk_isInstanceOf(o,bigint))
#define CURRENT_CTYPE struct bigint*
#define CURRENT_NAME  self

static void bigint_init_small(struct bigint * self, long long val) {
	if (val > 0) {
		self->sign = 1;
	} else if (val < 0) {
		self->sign = -1;
		val = -val;
	}

	self->size   = 0;
	self->digits = malloc(sizeof(uint32_t) * 3);

	while (val) {
		self->digits[self->size] = val & 0xFFFFFF;
		self->size++;
		val >>= 24;
	}
}

static KrkValue bigint_from_long_long(long long val) {
	struct bigint * newInt = (struct bigint *)krk_newInstance(bigint);
	bigint_init_small(newInt, val);
	return OBJECT_VAL(newInt);
}

KrkValue krk_makeBigInt_add(krk_integer_type a, krk_integer_type b) {
	return bigint_from_long_long((long long)a + (long long)b);
}

KrkValue krk_makeBigInt_sub(krk_integer_type a, krk_integer_type b) {
	return bigint_from_long_long((long long)a - (long long)b);
}

KrkValue krk_makeBigInt_mul(krk_integer_type a, krk_integer_type b) {
	return bigint_from_long_long((long long)a * (long long)b);
}

KRK_METHOD(bigint,__add__,{
	METHOD_TAKES_EXACTLY(1);
	if (!IS_bigint(argv[1])) return krk_runtimeError(vm.exceptions->notImplementedError, "not implemented");
	struct bigint * them = AS_bigint(argv[1]);

	if (self->sign != them->sign) {
		return krk_runtimeError(vm.exceptions->notImplementedError, "todo subtraction");
	}

	struct bigint * out = (struct bigint *)krk_newInstance(bigint);
	krk_push(OBJECT_VAL(out));

	out->sign = self->sign;

	size_t digits = self->size > them->size ? self->size : them->size;

	out->digits = malloc(sizeof(uint32_t) * (digits + 1));

	uint32_t carry = 0;
	for (size_t i = 0; i < digits; ++i) {
		uint32_t r = carry + ((self->size >= i) ? self->digits[i] : 0) + ((them->size >= i) ? them->digits[i] : 0);
		carry = r >> 24;
		out->digits[i] = r & 0xFFFFFF;
		out->size++;
	}

	if (carry) {
		out->digits[out->size++] = carry;
	}

	return krk_pop();
})

KRK_METHOD(bigint,negate,{
	METHOD_TAKES_NONE();
	struct bigint * out = (struct bigint *)krk_newInstance(bigint);
	krk_push(OBJECT_VAL(out));

	out->digits = malloc(sizeof(uint32_t) * self->size);
	out->size = self->size;
	memcpy(out->digits, self->digits, self->size * sizeof(uint32_t));

	out->sign = -(self->sign);
	return krk_pop();
})

KRK_METHOD(bigint,copy,{
	METHOD_TAKES_NONE();
	struct bigint * out = (struct bigint *)krk_newInstance(bigint);
	krk_push(OBJECT_VAL(out));

	out->digits = malloc(sizeof(uint32_t) * self->size);
	out->size = self->size;
	memcpy(out->digits, self->digits, self->size * sizeof(uint32_t));
	out->sign = self->sign;

	return krk_pop();
})

KRK_METHOD(bigint,__sub__,{
	METHOD_TAKES_EXACTLY(1);
	if (!IS_bigint(argv[1])) return krk_runtimeError(vm.exceptions->notImplementedError, "not implemented");
	struct bigint * them = AS_bigint(argv[1]);

	if (self->sign != them->sign) {
		krk_push(FUNC_NAME(bigint,negate)(1,&argv[1],0));
		krk_push(FUNC_NAME(bigint,__add__)(2,(KrkValue[]){argv[0],krk_peek(0)},0));
		krk_swap(1);
		krk_pop();
		return krk_pop();
	}

	krk_push(FUNC_NAME(bigint,copy)(1,&argv[0],0));
	struct bigint * out = (struct bigint*)AS_INSTANCE(krk_peek(0));

	if (out->size < them->size) {
		return krk_runtimeError(vm.exceptions->notImplementedError, "Sign flip because right side is definitely bigger");
	}

	int32_t carry = 0;
	for (size_t i = 0; i < out->size; ++i) {
		int32_t r = carry + out->digits[i] - ((them->size >= i) ? them->digits[i] : 0);
		out->digits[i] = r & 0xFFFFFF;
		carry = r >> 24;
	}

	for (size_t i = out->size; i > 0; i--) {
		if (out->digits[i-1]) break;
		out->size--;
	}

	if (carry) {
		return krk_runtimeError(vm.exceptions->notImplementedError, "Sign flip: %u\n", carry);
	}

	out->sign = self->sign;

	return krk_pop();
})

KRK_METHOD(bigint,__repr__,{
	struct StringBuilder sb = {0};
	pushStringBuilderStr(&sb, "<bigint ", 8);

	if (self->sign == 0)       pushStringBuilder(&sb, ' ');
	else if (self->sign == 1)  pushStringBuilder(&sb, '+');
	else if (self->sign == -1) pushStringBuilder(&sb, '-');

	pushStringBuilderStr(&sb, "0x", 2);

	for (size_t i = self->size; i > 0; i--) {
		char tmp[100];
		size_t len = snprintf(tmp,100,(i == self->size) ? "%x" : "%06x",self->digits[i-1]);
		pushStringBuilderStr(&sb, tmp, len);
	}

	pushStringBuilder(&sb, '>');

	return finishStringBuilder(&sb);
})

KRK_METHOD(bigint,__init__,{
	if (argc == 0) return argv[0];
	if (IS_int(argv[1])) {
		long long val = AS_int(argv[1]);
		bigint_init_small(self, val);
		return argv[0];
	}
	return TYPE_ERROR(int,argv[1]);
})

_noexport
void _createAndBind_bigintClass(void) {
	krk_makeClass(vm.builtins, &bigint, "bigint", vm.baseClasses->objectClass);
	bigint->allocSize = sizeof(struct bigint);
	bigint->_ongcsweep = bigint_ongcsweep;

	BIND_METHOD(bigint,__init__);
	BIND_METHOD(bigint,__repr__);
	BIND_METHOD(bigint,__add__);
	BIND_METHOD(bigint,__sub__);
	BIND_METHOD(bigint,negate);

	krk_finalizeClass(bigint);
}
