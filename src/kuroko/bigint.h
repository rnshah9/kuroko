#include <kuroko/kuroko.h>
#include <kuroko/value.h>
extern KrkValue krk_makeBigInt_add(krk_integer_type a, krk_integer_type b);
extern KrkValue krk_makeBigInt_sub(krk_integer_type a, krk_integer_type b);
extern KrkValue krk_makeBigInt_mul(krk_integer_type a, krk_integer_type b);
extern void _createAndBind_bigintClass(void);
