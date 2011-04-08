#ifndef __TASK_H__
#define __TASK_H__

#include "config.h"
#include <boost/array.hpp>
#include <boost/foreach.hpp>
#include <string>
#include "Collections.h"
#include "Hash.h"
#include "FileEntry.h"

class HashTask{
private:
	class HashTaskImpl;
	boost::shared_ptr<HashTaskImpl> impl;
public:
	HashTask();
	void add(uint32 id);
	void remove(uint32 id);
	bool isSet(uint32 id);
	size_t size();
	ArrayList<Hash> createHashList(uint64 totalLength);
};

class OutputTask{
private:
	class OutputTaskImpl;
	boost::shared_ptr<OutputTaskImpl> impl;
public:
	OutputTask();
	void remove(uint32 id);
	bool isSet(uint32 id);
	size_t size();
	std::string getFileName(uint32 id);
	void add(uint32 id, std::string fileName);
	void perform(HashTask task, ArrayList<FileEntry> data);
};

#endif