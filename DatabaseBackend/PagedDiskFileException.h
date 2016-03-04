#pragma once

struct PagedDiskFileException
{
	enum ExceptionType
	{
		FILE_NOSUCHFILE,
		OUT_OF_PAGE,
		OUT_OF_MEMORY,
		LOAD_OUT_OF_BOUND,
		LOAD_READBYTE_ERROR,
		CREATE_ERROR,
		WRITE_OUT_OF_SPACE,
		ALLOCATE_NOPAGE,
		INDEX_OUT_OF_BOUND,
		PAGE_NOT_USING
	};

	ExceptionType type;

	PagedDiskFileException(ExceptionType t) : 
		type(t)
	{

	}
};