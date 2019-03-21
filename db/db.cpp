#include "db.h"

uint32_t 	Savable::dependency_amount(){return 0;}
Dependency* Savable::dependencies()		{return 0;}

typedef char template_placeholder;
Savable::Dependency TableDepp = {off_of(Table<template_placeholder>, storage), 0};

uint32_t 	Table::dependency_amount()							{return 1;}
Dependency* Table::dependencies()								{return &TableDepp;}
uint32_t Table::saved_size()									{return sizeof(Table);}							
void     Table::save(void* data_block)							{memcpy(data_block, this, sizeof(Table));}
void	 Table::restore(void* data_block, Savable* instance)	{memcpy(instance, data_block, sizeof(Table));}