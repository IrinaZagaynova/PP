#pragma once
#include <stdexcept>
#include <memory>
#include <optional>

template <typename T>
class CLinkedList
{
	struct Node
	{
		Node(const std::optional<T>& data, Node* prev, std::unique_ptr<Node>&& next)
			:data(data),
			prev(prev),
			next(std::move(next))
		{
		}
		std::optional<T> data;
		Node* prev;
		std::unique_ptr<Node> next;
	};

	class CIterator
	{
		friend class CLinkedList;
		CIterator(Node* node, bool isRevers = false)
			:m_node(node),
			m_isRevers(isRevers)
		{
		}
	public:
		CIterator()
		{
			m_node = nullptr;
			m_isRevers = false;
		}

		bool operator != (const CIterator& it) const
		{
			return m_node != it.m_node;
		}

		bool operator == (const CIterator& it) const
		{
			return (m_node == it.m_node);
		}

		CIterator& operator++()
		{
			if (m_isRevers)
			{
				if (!m_node->prev)
				{
					throw std::out_of_range("you are trying to go beyond the list");
				}
				m_node = m_node->prev;
			}
			else
			{
				if (!m_node->next)
				{
					throw std::out_of_range("you are trying to go beyond the list");
				}
				m_node = m_node->next.get();
			}
			return *this;
		}
		CIterator& operator--()
		{
			if (m_isRevers)
			{
				if (!m_node->next->next)
				{
					throw std::out_of_range("you are trying to go beyond the list");
				}
				m_node = m_node->next.get();
			}
			else
			{
				if (!m_node->prev->prev)
				{
					throw std::out_of_range("you are trying to go beyond the list");
				}
				m_node = m_node->prev;
			}
			return *this;
		}

		CIterator& operator++(int)
		{
			++* this;
			return *this;
		}

		CIterator& operator--(int)
		{
			--* this;
			return *this;
		}

		T& operator*() const
		{
			if (!m_node->next || !m_node->prev)
			{
				throw std::out_of_range("you are trying to go beyond the list");
			}
			return (m_node->data).value();
		}

	private:
		Node* m_node = nullptr;
		bool m_isRevers = false;
	};
public:

	CLinkedList()
	{
		std::unique_ptr<Node> lastNode = std::make_unique<Node>(std::nullopt, nullptr, nullptr);
		Node* pointerLastNode = lastNode.get();
		std::unique_ptr<Node> firstNode = std::make_unique<Node>(std::nullopt, nullptr, std::move(lastNode));

		m_firstNode = std::move(firstNode);
		m_lastNode = pointerLastNode;
		m_lastNode->prev = m_firstNode.get();
		m_size = 0;
	}

	CLinkedList(const CLinkedList& other)
		:CLinkedList()
	{
		CLinkedList tmp;
		for (const auto& elem : other)
		{
			tmp.Insert(elem, tmp.end());
		}
		std::swap(*this, tmp);
	}

	CLinkedList(CLinkedList&& other) noexcept
	{
		Clear();
		m_firstNode = std::move(other.m_firstNode);
		m_lastNode = other.m_lastNode;
		m_size = other.m_size;

		CLinkedList emptyStringList;
		other.m_firstNode = std::move(emptyStringList.m_firstNode);
		other.m_lastNode = emptyStringList.m_lastNode;
		other.m_size = emptyStringList.m_size;
	}

	~CLinkedList()
	{
		Clear();
		m_firstNode = nullptr;
		m_lastNode = nullptr;
	}

	void Insert(const T& elem, const CIterator& it)
	{
		if (!it.m_node)
		{
			throw std::runtime_error("this iterator does not exist");
		}
		else if (!IsIteratorCorrect(it))
		{
			throw std::runtime_error("such an iterator does not exist in the object");
		}
		auto newPointNode = std::make_unique<Node>(elem, it.m_node->prev, std::move(it.m_node->prev->next));
		it.m_node->prev = newPointNode.get();
		newPointNode->prev->next = std::move(newPointNode);
		m_size++;
	}

	void PushBack(const T& elem)
	{
		Insert(elem, end());
	}

	void PushFront(const T& elem)
	{
		Insert(elem, begin());
	}

	void Clear()
	{
		while (!IsEmpty())
		{
			Erase(begin());
		}
	}

	void Erase(const CIterator& it)
	{
		if (IsEmpty())
		{
			throw std::runtime_error("cannot delete node in empty list");
		}
		else if (!IsIteratorCorrect(it))
		{
			throw std::runtime_error("such an iterator does not exist in the object");
		}
		else if (it == end())
		{
			throw std::out_of_range("unable to remove item from end position ()");
		}

		it.m_node->next->prev = it.m_node->prev;
		it.m_node->prev->next = std::move(it.m_node->next);
		--m_size;
	}

	CIterator begin()
	{
		return CIterator(m_firstNode->next.get());
	}

	CIterator end()
	{
		return CIterator(m_lastNode);
	}

	const CIterator begin() const
	{
		return CIterator(m_firstNode->next.get());
	}
	const CIterator end() const
	{
		return CIterator(m_lastNode);
	}

	const CIterator cbegin()const
	{
		return CIterator(m_firstNode->next.get());
	}
	const CIterator cend() const
	{
		return CIterator(m_lastNode);
	}

	CIterator rbegin()
	{
		return CIterator(m_lastNode->prev, true);
	}

	CIterator rend()
	{
		return CIterator(m_firstNode.get(), true);
	}

	const CIterator crbegin() const
	{
		return CIterator(m_lastNode->prev, true);
	}

	const CIterator crend() const
	{
		return CIterator(m_firstNode.get(), true);
	}

	size_t GetSize() const
	{
		return m_size;
	}

	bool IsEmpty() const
	{
		return m_size == 0;
	}

	CLinkedList& operator=(CLinkedList& other)
	{
		if (&other != this)
		{
			CLinkedList tmp(other);
			std::swap(*this, tmp);
		}
		return *this;
	}

	CLinkedList& operator=(CLinkedList&& other) noexcept
	{
		if (&other != this)
		{
			Clear();
			m_firstNode = std::move(other.m_firstNode);
			m_lastNode = other.m_lastNode;
			m_size = other.m_size;

			CLinkedList emptyStringList;
			other.m_firstNode = std::move(emptyStringList.m_firstNode);
			other.m_lastNode = emptyStringList.m_lastNode;
			other.m_size = emptyStringList.m_size;
		}
		return *this;
	}

private:
	bool IsIteratorCorrect(const CIterator& it) const
	{
		if (it == end())
		{
			return true;
		}
		auto objectIterator = this->begin();
		for (; objectIterator != end(); objectIterator++)
		{
			if (it == objectIterator)
			{
				return true;
			}
		}
		return false;
	}

	size_t m_size;
	std::unique_ptr<Node> m_firstNode;
	Node* m_lastNode;
};