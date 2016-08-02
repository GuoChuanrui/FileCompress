#pragma once 
#include"Heap.h"
#include<assert.h>


template<class T>
struct HuffmanTreeNode
{
	HuffmanTreeNode<T>* _left;
	HuffmanTreeNode<T>* _right;
	HuffmanTreeNode<T>* _parent;
	T _weight;  //权值

	//构造函数
	HuffmanTreeNode(const T& x)
		:_weight(x)
		, _left(NULL)
		, _right(NULL)
		, _parent(NULL)
	{}
};

template<class T>
class HuffmanTree
{
	typedef HuffmanTreeNode<T> Node;

	template<class T>
	struct NodeCompare
	{
		bool operator() (Node* l, Node* r)
		{
			return l->_weight < r->_weight;
		}
	};

public:
	HuffmanTree()
		:_root(NULL)
	{}

	~HuffmanTree()
	{}

	void CreateTree(const T* a, size_t size, const T& invalid)
	{
		assert(a);
		Heap<Node*, NodeCompare<T>> minHeap;
		for (size_t i = 0; i < size; i++)
		{
			if (a[i] != invalid)
			{
				Node* node = new Node(a[i]);
				minHeap.Push(node);
			}
		}

		while (minHeap.Size()>1)
		{
			Node* left = minHeap.Top();
			minHeap.Pop();
			Node* right = minHeap.Top();
			minHeap.Pop();

			Node* parent = new Node(left->_weight + right->_weight);
			parent->_left = left;
			parent->_right = right;
			left->_parent = parent;
			right->_parent = parent;

			minHeap.Push(parent);
		}
		_root = minHeap.Top();
	}

	Node* GetRootNode()
	{
		return _root;
	}


protected:
	Node* _root;

};

void TestHuffmanTree()
{
	HuffmanTree<int> ht;
	int a[] = {1,2,3,4,5,6,7,8,9,10};
	ht.CreateTree(a, 10, 0);
}