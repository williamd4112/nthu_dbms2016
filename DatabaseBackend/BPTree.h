#pragma once

#include "DatabaseDataType.h"

enum TreeNodeType {INNER, LEAF};
enum SearchType {INSERT, MATCH};

typedef PageID NodeID;
typedef int(*CompareFunc) (const void *a, const void *b);
typedef void(*PrintFunc) (const void *src);

static int CompareInt(const void *a, const void *b)
{
	return (*(int*)a == *(int*)b) ? 0 : (*(int*)a < *(int*)b) ? -1 : 1;
}

static void PrintInt(const void *src)
{
	std::cout << *((int*)src) << std::endl;
}

template <typename K, size_t PAGE_SIZE>
class BPTree;

template <typename K,
		size_t PAGE_SIZE,
		size_t HEADER_SIZE = sizeof(TreeNodeType) + sizeof(unsigned int),
		size_t FREESPACE_SIZE = PAGE_SIZE - HEADER_SIZE>
class BPTreeNode
{
public:
	typedef BPTreeNode<K, PAGE_SIZE> Node;
	struct alignas(PAGE_SIZE) NodeData
	{
		TreeNodeType nodeType;
		unsigned int keyCount;
		byte freeSpace[FREESPACE_SIZE];

		NodeData(TreeNodeType _type) : keyCount(0)
		{

		}

		~NodeData(){}
	};

	BPTreeNode(BPTree<K, PAGE_SIZE> &parentTree, TreeNodeType type, DomainType keyDomain, size_t keySize)
		: mParentTree(parentTree), 
		mNodeData(type),
		mKeyDomain(keyDomain),
		keySize(keySize),
		maxKeyCount(FREESPACE_SIZE / (keySize + sizeof(NodeID)))
	{
		valueSegmentOffset = maxKeyCount * keySize;
		if (keyDomain == INTEGER_DOMAIN) 
		{
			mCmpFunc = CompareInt;
			mPrintFunc = PrintInt;
		}
	}

	~BPTreeNode()
	{

	}

	inline TreeNodeType Type() { return mNodeType; }
	inline void Type(TreeNodeType type) { mDiskPart.nodeType = type; }
	inline int InsertKey(const void *src)
	{
		assert(mNodeData.keyCount <= maxKeyCount);

		int left = binarySearch(src, INSERT);

		memmove(mNodeData.freeSpace + (left + 1) * keySize,
			mNodeData.freeSpace + (left) * keySize, (mNodeData.keyCount - left) * keySize);
		memcpy(mNodeData.freeSpace + left * keySize, src, keySize);
		mNodeData.keyCount++;
		
		return (mNodeData.keyCount == maxKeyCount) ? 1 : 0; // Check overflow
	}

	inline void DumpAllKeys()
	{
		std::cout << "KeySize: " << keySize << " KeyCount: " << mNodeData.keyCount
			<< " MaxKeyCount: " << maxKeyCount << " KeySegment: " << valueSegmentOffset
			<< std::endl;
		for (int i = 0; i < mNodeData.keyCount; i++)
		{
			PrintInt(mNodeData.freeSpace + i * keySize);
		}
	}
protected:
	BPTree<K, PAGE_SIZE> &mParentTree;
	DomainType mKeyDomain;
	size_t keySize;
	size_t valueSegmentOffset;
	unsigned int maxKeyCount;
	CompareFunc mCmpFunc;
	PrintFunc mPrintFunc;

	// Disk part
	NodeData mNodeData;

	inline int binarySearch(const void *key, SearchType type)
	{
		int left = 0;
		int right = mNodeData.keyCount - 1;
		int result;
		while (left <= right)
		{
			int mid = (left + right) >> 1;
			result = mCmpFunc(key, mNodeData.freeSpace + mid * keySize);
			if (result < 0) right = mid - 1;
			else if(result > 0) left = mid + 1;
			else if(type == INSERT) throw DUPLICATE_RECORD;
			else return mid;
		}
		
		return (type == INSERT) ? left : -1;
	}
};

template <typename K,
	size_t PAGE_SIZE,
	size_t HEADER_SIZE = sizeof(TreeNodeType) + sizeof(unsigned int),
	size_t FREESPACE_SIZE = PAGE_SIZE - HEADER_SIZE>
class BPTreeLeafNode
	: public BPTreeNode<K, PAGE_SIZE>
{
public:
	BPTreeLeafNode(BPTree<K, PAGE_SIZE> &parentTree, DomainType keyDomain, size_t keySize)
		: BPTreeNode(parentTree, LEAF, keyDomain, keySize)
	{

	}

	~BPTreeLeafNode()
	{

	}

	inline Node *Split()
	{
		BPTreeLeafNode<K, PAGE_SIZE> *sibling =
			new BPTreeLeafNode<K, PAGE_SIZE>(mParentTree, mKeyDomain, keySize);
		
		int halfCount = mNodeData.keyCount >> 1;

		size_t remainSize = (halfCount) * keySize;
		byte *freeSpaceHalfAddr = mNodeData.freeSpace + remainSize;
		size_t moveSize = mNodeData.keyCount * keySize - remainSize;

		memcpy(sibling->mNodeData.freeSpace, 
			freeSpaceHalfAddr,
			moveSize);
		memset(freeSpaceHalfAddr,
			0,
			moveSize);
		sibling->mNodeData.keyCount = mNodeData.keyCount - halfCount;
		mNodeData.keyCount = halfCount;

		return sibling;
	}

	inline void SetValue(const void *key, NodeID value)
	{
		int pos = binarySearch(key, MATCH);
		if (pos >= 0)
		{
			std::cout << "Found ";
			PrintInt(key);
			std::cout << "\n";
		}
		else
		{
			std::cout << "Not found" << std::endl;
		}
		
	}
private:
};

template <typename K, size_t PAGE_SIZE>
class BPTree
{
public:
	BPTree(unsigned int order)
	{

	}

	~BPTree()
	{

	}

private:
	BPTreeNode<K, PAGE_SIZE> *mRootNode;
};