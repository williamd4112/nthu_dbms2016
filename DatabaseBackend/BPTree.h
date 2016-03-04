#pragma once

#include <map>
#include <queue>

template <typename K, typename V>
class BPTreeNode;

template <typename K, typename V>
class BPTreeLeafNode;

template <typename K, typename V>
class BPTree
{
public:
	BPTree(int order)
		: mOrder(order)
	{
		mRootNode = new BPTreeLeafNode();
	}

	BPTree(const char *filepath)
	{

	}

	~BPTree()
	{
		delete mRootNode;
	}

	void insert(K key, V value)
	{

	}

	V find(K key);
	BPTreeLeafNode<K, V> *findLeafNode(K key);
	void remove(K key);
	BPTreeNode<K, V> *getTreeNode(int nodeId);
private:
	BPTreeNode<K, V> *loadTreeNode(int nodeId);

	int mOrder;
	BPTreeNode<K, V> *mRootNode;
	std::map<int, BPTreeNode<K, V>* > mNodeCache; // After tree node loaded, node will store in this map.
	std::priority_queue<int> mFreeIDHeap;
};

template <typename K, typename V>
class BPTreeNode
{
public:
	BPTreeNode(BPTree<K, V> *tree, int id, int order) 
		: mTree(tree), mId(id), mOrder(order)
	{

	}

	BPTreeNode *parent() const;
	void insertKey(K key);
	void deleteKey(K key);
	void dealOverflow();
	virtual BPTreeNode<K, V>* split() = 0;
	
protected:
	void setParent(BPTreeNode<K, V> *newParent);
	
	BPTree<K, V> *mTree;
	BPTreeNode<K, V> *mParentNode;
	int mId;
	int mOrder;
	int mKeyCount;
	K *mKeys;
};

template <typename K, typename V>
class BPTreeInnerNode : public BPTreeNode<K, V>
{
public:
	BPTreeInnerNode(BPTree<K, V> *tree, int id, int order);

	void insertChild(BPTreeNode<K, V> *child);
	void deleteChild(K key);
	BPTreeNode *split();
private:
	BPTreeNode<K, V> *mChildren[];
	int mChildCount;
};

template<typename K, typename V>
class BPTreeLeafNode : public BPTreeNode<K, V>
{
public:
	BPTreeLeafNode(BPTree<K, V> *tree, int id, int order);

	void setValue(K key, V value);
	void deleteValue(K key, V value);
	BPTreeNode *split();
private:
	V *mValues;
	int mValueCount;
};

