#include <stdint.h>
#include <vector>

struct Savable{
	struct Dependency{
		uint32_t data_offset;
		void* 	 data;
		uint32_t data_size;
	};

	virtual uint32_t dependency_amount() {return 0;}
	virtual Dependency* dependencies()   {return 0;}

	virtual uint32_t saved_size() = 0;
	virtual void save(void* data_block) = 0;
	virtual void restore(void* data_block, Savable* instance) = 0;
};


struct Database{
	
};