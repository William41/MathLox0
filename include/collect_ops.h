/* THIS HEADER IS PART OF THE CLOX RUNTIME.
*IT INCLUDES FUNCTION SIGNATURES FOR ALL THE
*NATIVELY SUPPORTED OPERATIONS WHICH ARE
*PERFORMED ON list object TYPE.
*
*/

#ifndef clox_collect_ops_h
#define clox_collect_ops_h

#include "object.h"
#include "value.h"

Value appendNative(int,Value*);
Value deleteNative(int,Value*);
Value getListLength(int,Value*);
Value isRealNative(int,Value*);
Value getMaxNative(int,Value*);
Value getMinNative(int,Value*);
Value getRangeNative(int,Value*);
//gets the AM,GM,HM of a real valued list
Value getAMNative(int,Value*);
Value getGMNative(int,Value*);
Value getHMNative(int,Value*);
Value getMedianNative(int,Value*);
Value getModeNative(int,Value*);
Value getQ1Native(int,Value*);
Value getQ3Native(int,Value*);
Value getQDcoeffNative(int,Value*);
Value getSDNative(int,Value*);
Value getSDPopNative(int,Value*);
//view the list as a set
Value isEmptySetNative(int,Value*);
Value unionNative(int,Value*);
Value intersectNative(int,Value*);
Value complementNative(int,Value*);
Value setDiffNative(int,Value*);
Value symDiffNative(int,Value*);
Value isMultiSetNative(int,Value*);

#endif
