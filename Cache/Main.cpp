/*Wielow¹tkowy Cache w c++ trzymaj¹cy elementy <klucz,wartoœæ> z góry ograniczon¹ pojemnoœci¹:

Za³o¿enia:
- Maksymalna pojemnoœæ - po jej przekroczeniu usuwamy najdawniej u¿ywan¹ wartoœæ 
- Mechanizm przeterminowania (TTL - time to live): ile czasu maksymalnie mo¿e byæ przechowywany (osobno dla ka¿dej wartoœci)
- wielow¹tkowoœæ - synchronizacja np. mutexy


template < typename Key, typename Value>
class Cache

metody:
put()
get()
contains()
cleanup() [robiony poza u¿ytkownikiem]*/
#include "Cache.h"

int main() 
{
	Cache<std::string, std::string> cache(3);
	cache.Put("1", "one", 5000);
	cache.Put("2", "two", 10000);
	return 0;
}