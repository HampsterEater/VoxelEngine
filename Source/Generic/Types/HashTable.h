// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GENERIC_HASHTABLE_
#define _GENERIC_HASHTABLE_

#include <string.h>

#include "Generic\Threads\Mutex.h"
#include "Generic\Threads\MutexLock.h"
#include "Generic\Types\CachedMemoryPool.h"

#define HASH_TABLE_INITIAL_BUCKETS	32
#define HASH_TABLE_BUCKET_INCREMENT	2
#define HASH_TABLE_MAX_LOAD			0.75f

template <typename ValueType, typename KeyType>
class HashTable
{
private:
	
	// Value stored in hash table.
	struct HashTableValue
	{
		ValueType		Value;
		KeyType			Hash;
		HashTableValue*	Next;
	};

	HashTableValue**					m_data;
	int									m_size;
	int									m_allocated;

	CachedMemoryPool<HashTableValue>	m_memory_pool;

	Mutex*								m_mutex;

protected:
	
	// Gets the bucket a hash should be in.
	int Get_Bucket(KeyType hash) const
	{
		return hash % m_allocated;
	}

	// Gets a value from a hash.
	HashTableValue* Get_Value(KeyType hash) const
	{
		// Figure out bucket index.
		int bucket_index = Get_Bucket(hash);
		
		// Find end of current bucket chain.
		// TODO: Double linked list, faster.
		HashTableValue* bucket = m_data[bucket_index];
		while (bucket != NULL)
		{
			if (bucket->Hash == hash)
			{
				return bucket;
			}

			bucket = bucket->Next;
		}

		return NULL;
	}

	// Allocates our buckets, and keeps old data if neccessary.
	void Allocate(int size, bool keep_old)
	{
        HashTableValue**	 old_data		= m_data;
		int					 old_alloc_size = m_allocated;
        int					 allocate_size	=  size;

        if (allocate_size > m_allocated || m_data == NULL)
		{
            m_data = new HashTableValue*[allocate_size];
            m_allocated = allocate_size;
            memset(m_data, 0, allocate_size * sizeof(HashTableValue*));
				
            if (keep_old == true && old_data != NULL)
            {
				m_size = 0;

				// Reinsert the data into the new memory block.
				for (int i = 0; i < old_alloc_size; i++)
                {
					HashTableValue* val = old_data[i];
					while (val != NULL)
					{
						Set(val->Hash, val->Value);
						val = val->Next;
					}						
                }
			}
			else
			{
				m_size = 0;
            }

			// Dispose of any old data.
            if (old_data != NULL)
            {
				for (int i = 0; i < old_alloc_size; i++)
                {
					HashTableValue* val = old_data[i];
					while (val != NULL)
					{
						HashTableValue* next = val->Next;
						
						//delete val;
						m_memory_pool.Release(val);

						val = next;
					}						
                }

				SAFE_DELETE_ARRAY(old_data);
            }
        }
	}

	// Checks if buckets are overloaded, if they are it increases
	// the amounts of buckets and redistributes nodes.
	void Check_Load()
	{
		float current_load = (float)m_size / (float)m_allocated;
		if (current_load > HASH_TABLE_MAX_LOAD)
		{
			float new_load_size = (float)m_allocated * HASH_TABLE_BUCKET_INCREMENT;

			// TODO: Is this neccessary?
			while (true)
			{
				float new_load = float(m_size) / float(new_load_size);
				if (new_load < HASH_TABLE_MAX_LOAD)
				{
					break;
				}
				new_load_size *= HASH_TABLE_BUCKET_INCREMENT;
			}

			Allocate((int)new_load_size, true);
		}
	}

public:

	// Constructors.
	HashTable()
		: m_data(NULL)
		, m_allocated(0)
		, m_size(0)
		//, m_memory_pool(sizeof(HashTableValue))
	{
		Allocate(HASH_TABLE_INITIAL_BUCKETS, false);

		m_mutex = Mutex::Create();
		DBG_ASSERT(m_mutex != NULL);
	}

	~HashTable()
	{
		for (int i = 0; i < m_allocated; i++)
		{	
			HashTableValue* bucket = m_data[i];
			while (bucket != NULL)
			{
				HashTableValue* next = bucket->Next;
				
				//delete next;
				m_memory_pool.Release(next);

				bucket = next;
			}
		}

		SAFE_DELETE_ARRAY(m_data);
	}

	// Returns true if we contain the given hash.
	bool Contains(KeyType hash) const
	{
		MutexLock lock(m_mutex);

		return Get_Value(hash) != NULL;
	}
	
	// Returns the size of the hash table.
	int Size() const
	{
		MutexLock lock(m_mutex);

		return m_size;
	}

	// Clears the hash table of values.
	void Clear()
	{
		MutexLock lock(m_mutex);

		for (int i = 0; i < m_allocated; i++)
		{	
			HashTableValue* bucket = m_data[i];
			while (bucket != NULL)
			{
				HashTableValue* next = bucket->Next;
			
				//delete next;
				m_memory_pool.Release(next);

				bucket = next;
			}

			m_data[i] = NULL;
		}

		m_size = 0;
	}

	// Removes a hash value.
	void Remove(KeyType hash)
	{
		MutexLock lock(m_mutex);

		// Figure out bucket index.
		int bucket_index = Get_Bucket(hash);
		
		// Find end of current bucket chain.
		// TODO: Double linked list, faster.
		HashTableValue* bucket = m_data[bucket_index];
		HashTableValue* prev   = NULL;
		while (bucket != NULL)
		{
			if (bucket->Hash == hash)
			{
				if (prev != NULL)
				{
					prev->Next = bucket->Next;					
				}
				else
				{
					m_data[bucket_index] = bucket->Next;
				}
				
				//delete bucket;
				m_memory_pool.Release(bucket);

				m_size--;

				return;
			}

			prev = bucket;
			bucket = bucket->Next;
		}

		DBG_ASSERT(false);
	}

	// Gets an "index" of the hash table. This is used for interating, and is slow
	// as shit, you should only use this when doing things like deleting eveyrthing in the hash table.
	void Get_Index(int index, KeyType& hash, ValueType& value)
	{
		MutexLock lock(m_mutex);

		int at_index = 0;

		for (int i = 0; i < m_allocated; i++)
		{	
			HashTableValue* bucket = m_data[i];
			while (bucket != NULL)
			{
				if (at_index == index)
				{
					hash = bucket->Hash;
					value = bucket->Value;
					return;
				}

				at_index++;
				bucket = bucket->Next;
			}
		}
	}

	// Gets the value for the given hash.
	ValueType Get(KeyType hash) const
	{
		MutexLock lock(m_mutex);

		HashTableValue* val = Get_Value(hash);
		return val == NULL ? NULL : val->Value;
	}

	// Sets the value for the given hash.
	void Set(KeyType hash, ValueType value)
	{
		MutexLock lock(m_mutex);

		// Figure out bucket index.
		int bucket_index = Get_Bucket(hash);
		
		// Find end of current bucket chain.
		// TODO: Double linked list, faster.
		HashTableValue* bucket = m_data[bucket_index];
		while (bucket != NULL)
		{
			if (value == NULL)
			{
				if (bucket->Hash == hash)
				{
					bucket->Value = NULL;
					return;
				}
			}
			else
			{
				if (bucket->Value == NULL)
				{
					bucket->Value = value;
					return;
				}
				else
				{
					DBG_ASSERT(bucket->Hash != hash);
				}
			}

			HashTableValue* next = bucket->Next;
				
			if (next == NULL)
			{
				break;					
			}

			bucket = next;
		}

		// Insert the new item.
		HashTableValue* val = new(m_memory_pool.Allocate()) HashTableValue();
		val->Hash	= hash;
		val->Value	= value;
		val->Next	= NULL;

		if (bucket == NULL)
		{
			m_data[bucket_index] = val;
		}
		else
		{
			bucket->Next = val;			
		}

		m_size++;

		// Expand if we are over the load limit.
		Check_Load();
	}

};

#endif