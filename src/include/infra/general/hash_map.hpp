#ifdef USE_GCC3
#include <ext/hash_map>
namespace std {
using __gnu_cxx::hash;
using __gnu_cxx::hashtable;
using __gnu_cxx::hash_map;
using __gnu_cxx::hash_multimap;
}
#endif
