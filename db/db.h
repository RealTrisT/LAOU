#include <stdint.h>
#include <vector>

#define off_of(struct_, member_) (char*)&(((struct_*)0)->member_) 

struct Savable{
	struct Dependency{
		uint32_t data_offset;	//this is the structure offset to the pointer which holds the data. Will be fixed when saved, and fixed back when restored
		uint32_t data_size;		//this is the size of the data chunk
	};

	virtual uint32_t dependency_amount();			//this will of course return the amount of save dependencies for the derived struct
	virtual Dependency* dependencies();				//this will return an array of size dependency_amount() with the dependencies for the struct

	virtual uint32_t saved_size() = 0;								//this is the size of the derived struct when saved
	virtual void save(void* data_block) = 0;						//this is what'll save the structure into that block of memory (to then maybe be saved to file?)
	virtual void restore(void* data_block, Savable* instance) = 0;	//this restores it from the data block into a created instance
};


//TODO: fucking don't make table the vector, make it a vector, aka, create a vector class and then 
//what if I don't want a savable vector?


template<class T> struct Table : public Savable{													//this will basically be a vector

	struct InsiderDependency;
	struct ForeignDependency;

	struct ForeignDependency{			
		InsiderDependency* agressive_dep;		//pointer to a InsiderDependency for the insiderDeps of another table
		T* internal_entry_loc;					//the location of the entry the agressive table points to, for trackability
	};									

	struct InsiderDependency{			
		void** internal_ptr_loc;				//pointer in our table
		ForeignDependency* passive_dep;			//foreignDeps entry of another table
	};									


	//Table();																						//justaconstructor
	
	std::vector<T> 				   data;															//will hold the data of this table
	std::vector<ForeignDependency> passiveDeps;														//will hold the dependencies that other instances have to our own 	(will be fixed when our table is changed)
	std::vector<InsiderDependency> agressiveDeps;													//will hold the dependencies our instance has to others				(will be fixed when an external table is changed)

/**//**//**//**//**//**//**//**//**//**//**//**//**//**//**//**//**//**//**//**//**//**//**//**//**//**/
/**/T* push(const T&);																				/**/
/**//**//**//**//**//**//**//**//**//**//**//**//**//**//**//**//**//**//**//**//**//**//**//**//**//**/	

/**//**//**//**//**//**//**//**//**//**//**//**//**//**//**//**//**//**//**//**//**//**//**//**//**//**/
/**/void fix_agressive_dependency(InsiderDependency* depp, T* old_ptr, T* new_ptr);					/**/
/**/void fix_passive_dependency(ForeignDependency* depp, T* old_ptr, T* new_ptr);					/**/
/**/																								/**/
/**/void reloc(T* old_ptr, T* new_ptr);							//function to be called when the data is reallocated (as happens with a move)  \  so we fix it in
/**/void repos(T* old_ptr, T* new_ptr, uint32_t amount);		//function to be called when data is inserted or removed                       /  the dep tables
/**//**//**//**//**//**//**//**//**//**//**//**//**//**//**//**//**//**//**//**//**//**//**//**//**//**/	

/**//**//**//**//**//**//**//**//**//**//**//**//**//**//**//**//**//**//**//**//**//**//**//**//**//**/
/**/void fix_foregin_passive_tables();							//this will be called when our agressive table is written to (dependency registered or unregisterd)
/**/void fix_foregin_agressive_tables();						//this will only be called by other tables, which agressively put things on our own
/**/																								/**/
/**/template<class U> void register_dependency(void* from_ptr, Table<U>* foregin, void* to_entry);	/**/
/**//**//**//**//**//**//**//**//**//**//**//**//**//**//**//**//**//**//**//**//**//**//**//**//**//**/
	uint32_t saved_size();															
	void     save(void* data_block);
	void	 restore(void* data_block, Savable* instance);
};

struct Database{
	
};

#include "db.tcpp"