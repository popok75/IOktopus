
#include <map>
#include <iostream>
#include <vector>

#define x86BUILD
#define RF(x) x

#include "infrastructure/x86/monoTimer.h"

#include "tests/test-datastruct.h"
//#include "tests/test-clock.h"

#include "IOktopus.ino"

/*

template <class T>
class my_allocator
{
public:
  typedef size_t    size_type;
  typedef ptrdiff_t difference_type;
  typedef T*        pointer;
  typedef const T*  const_pointer;
  typedef T&        reference;
  typedef const T&  const_reference;
  typedef T         value_type;

  my_allocator() {}
  my_allocator(const my_allocator&) {}



  pointer   allocate(size_type n, const void * = 0) {
	  	  	  int size=n * sizeof(T);
              T* t = (T*) malloc(size);
              std::cout
              << "  used my_allocator to allocate "<< size<<"  at address "
              << t << " (+)" << std::endl;
              return t;
            }

  void      deallocate(void* p, size_type) {
              if (p) {
                free(p);
                std::cout
                << "  used my_allocator to deallocate at address "
                << p << " (-)" <<
                std::endl;
              }
            }

  pointer           address(reference x) const { return &x; }
  const_pointer     address(const_reference x) const { return &x; }
  my_allocator<T>&  operator=(const my_allocator&) { return *this; }
  void              construct(pointer p, const T& val)
                    { new ((T*) p) T(val); }
  void              destroy(pointer p) { p->~T(); }

  size_type         max_size() const { return size_t(-1); }

  template <class U>
  struct rebind { typedef my_allocator<U> other; };

  template <class U>
  my_allocator(const my_allocator<U>&) {}

  template <class U>
  my_allocator& operator=(const my_allocator<U>&) { return *this; }
};
*/

int main(int argc, char* argv[])
{
	std::cout << "IOktopusx86 v0.03 "<<std::endl;
/*
	const int numItems = 100;
	  std::cout << "\nCreating a RWTValDlist with a default allocator"
	            << std::endl;

	  std::vector<GenString> regular;


	  std::cout << "\nInserting " << numItems
	            << " items" << std::endl;

	  for (int i = 0; i < numItems; ++i) {
	    regular.push_back(GenString('a' + i, i));
	  }


	  std::cout << "\n\nCreating a RWTValDlist with my_allocator type"
	            << std::endl;

	  std::vector<GenString, my_allocator<GenString> > custom;

	  std::cout << "\nInserting " << numItems
	            << " items\n" << std::endl;

	  for (int i = 0; i < numItems; ++i) {
		if(!(custom.size()%5)) custom.reserve(custom.size()+5);
	    custom.push_back(GenString('a' + i, i));
	  }
*/

	testChunkedVector();


/*
	bool b=testObjMap();
	if(!b) {std::cout << "testObjMap failed "<<std::endl;return -1;}


	b=testMap();
	if(!b) {std::cout << "testMap failed "<<std::endl;return -1;}
	b=testTreeMap();
	if(!b) {std::cout << "testTreeMap failed "<<std::endl;return -1;}
*/
//	b=testClock();
//	if(!b) {std::cout << "testClock failed "<<std::endl;return -1;}


/*
	b=testCurFS();
	if(!b) {std::cout << "testCurFS failed "<<std::endl;return -1;}
*/

//	testSSMap();

	 setup();

	// x86 timer and server maintaining

	bool first=true;
	while(1){
			    if(first) {std::cout << "loop started " << monomillis() << std::endl;first=false;}
							//or IOFactory::yield();
			 	loop();
			//	 std::cout << ". " << monomillis() << std::endl;
				delay(100);
	//		*	std::cout << "millis 2 " << monomillis() << std::endl;
			//	std::cout << std::endl;
			}

	std::cout << "IOktopusx86 v0.01 ended"<<std::endl;

	//end();	//cleanup

	return 0;
}
