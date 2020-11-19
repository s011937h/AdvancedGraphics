#pragma once

// Smart pointer class for managing reference counts of IUnknown derived COM interfances
template <class T>
class ComPtr
{
private:
	T * m_Pointer;

	typedef ComPtr<T> this_type;

	void InternalReleaseRef(T * rc)
	{
		rc->Release();
	}

	void InternalAddRef(T * rc)
	{
		rc->AddRef();
	}

	void AttachWithoutAddRef(T * newValue)
	{
		if (m_Pointer)
		{
			InternalReleaseRef(m_Pointer);
		}

		m_Pointer = newValue;
	}

	void Assign(T * newValue)
	{
		if (newValue == m_Pointer)
		{
			return;
		}

		if (newValue)
		{
			InternalAddRef(newValue);
		}

		if (m_Pointer)
		{
			InternalReleaseRef(m_Pointer);
		}


		m_Pointer = newValue;
	}

public:
	// default constructor - assigns to nullptr
	ComPtr()
		: m_Pointer(nullptr)
	{

	}

	// construct from a nullptr, equivalent to default constructor
	ComPtr(nullptr_t)
		: m_Pointer(nullptr)
	{

	}

	// Move constructor. 
	// Acquires ownership of reference count from another ComPtr
	ComPtr(ComPtr && other)
		: m_Pointer(other.Detach())
	{

	}


	// Move constructor. 
	// Acquires ownership of reference count from another ComPtr of a different type, assignable to this type
	template <typename Y>
	ComPtr(ComPtr<Y> && other)
		: m_Pointer(other.Detach())
	{

	}

	// Assigns smartpointer to an object without incrementing the reference count	
	static [[nodiscard]] ComPtr AttachNew(T * t)
	{
		ComPtr ptr;

		ptr.AttachWithoutAddRef(t);

		return ptr;
	}

	// Removes ownership of the assigned reference counted pointer and returns it
	// without invoking Release() on the reference counted object.
	// Used for removing onwership of a reference count from this smart pointer
	[[nodiscard]] T * Detach()
	{
		T * value = m_Pointer;
		m_Pointer = nullptr;
		return value;
	}

	// Assigns smartpointer to an object and increments reference count.
	// Used for transfering ownership of a reference count to this smart pointer.
	static ComPtr AssignFromPtrAndAddRef(T * t)
	{
		ComPtr ptr;

		ptr.Assign(t);

		return ptr;
	}

	// Copy constructor.
	// Increments reference count on the object.
	template <class Y>
	ComPtr(const ComPtr<Y> & other)
		: m_Pointer(0)
	{
		Assign(other.Get());
	}

	// Copy constructor.
	// Increments reference count on the object.
	ComPtr(const ComPtr & other)
		: m_Pointer(nullptr)
	{
		Assign(other.Get());
	}

	// Copy assignment operator
	// Increments reference count on the object.
	ComPtr & operator = (const ComPtr & other)
	{
		Assign(other.Get());

		return *this;
	}

	// Copy assignment operator
	// Increments reference count on the object.
	template<class Y>
	ComPtr & operator = (const ComPtr<Y> & other)
	{
		Assign(other.Get());

		return *this;
	}

	// Move assignment operator
	// Acquires ownership of the reference count from the other smart pointer
	ComPtr & operator = (ComPtr && other)
	{
		AttachWithoutAddRef(other.Detach());

		return *this;
	}

	// Move assignment operator
	// Acquires ownership of the reference count from the other smart pointer
	template<class Y>
	ComPtr & operator = (ComPtr<Y> && other)
	{
		AttachWithoutAddRef(other.Detach());

		return *this;
	}

	// Destructor
	// Releases the reference count owned by the smart pointer
	~ComPtr()
	{
		Assign(nullptr);
	}

	// Pointer dereferencing operator
	T & operator * () const
	{
		assert(m_Pointer != nullptr);

		return *m_Pointer;
	}

	// Pointer indirection operator
	T * operator -> () const
	{
		assert(m_Pointer != nullptr);

		return m_Pointer;
	}

	// ! operator, returns true if pointer is nullptr
	bool operator! () const 
	{
		return m_Pointer == nullptr;
	}

	// Cast-to-unspecified bool type. 
	// Helper used for testing in if (ptr) conditions
	typedef T * (this_type::*unspecified_bool_type)() const;	
	operator unspecified_bool_type() const 
	{
		return m_Pointer == nullptr ? 0 : &this_type::Get;
	}

	// Accessor
	T * Get() const
	{
		return m_Pointer;
	}

	// Releases the current reference count if held
	// and returns pointer to the wrapped pointer.
	// Used for reference count ownership transfering assignment.
	T ** ReleaseAndGetAddressOf()
	{
		Assign(nullptr);

		return &m_Pointer;
	}

	// Returns pointer to the wrapped pointer, without discarding the current state
	// Use with caution, ensure that ownership lifetime is managed.
	T ** GetAddressOf()
	{
		return &m_Pointer;
	}

	// Returns pointer to the wrapped pointer, without discarding the current state
	// Use with caution, ensure that ownership lifetime is managed.
	T *const * GetAddressOf() const
	{
		return &m_Pointer;
	}


	// QueryInterface helper cast
	template <class Y>
	ComPtr<Y> QueryInterfaceCast() const
	{		
		if (m_Pointer)
		{
			ComPtr<Y> result;

			m_Pointer->QueryInterface(__uuidof(Y),
									  (void**)(result.ReleaseAndGetAddressOf()));
			
			return result;
		}

		return nullptr;
	}

	// Static cast helper cast
	// Used when casting between ComPtrs where the types are different but static_cast-able
	template <class Y>
	ComPtr<Y> StaticCast() const
	{
		return ComPtr<Y>::AssignFromPtrAndAddRef(static_cast<Y *>(m_Pointer));
	}

	// Releases the owned reference count and sets the ComPtr to nullptr
	void Reset()
	{
		Assign(nullptr);
	}

	bool operator == (nullptr_t) const
	{
		return m_Pointer == nullptr;
	}

	bool operator != (nullptr_t) const
	{
		return m_Pointer != nullptr;
	}

	template <class Y> bool operator == (const ComPtr<Y> & other) const
	{
		return m_Pointer == other.Get();
	}

	template <class Y> bool operator != (const ComPtr<Y> & other) const
	{
		return m_Pointer != other.Get();
	}

	template <class Y> bool operator > (const ComPtr<Y> & other) const
	{
		return m_Pointer > other.Get();
	}

	template <class Y> bool operator < (const ComPtr<Y> & other) const
	{
		return m_Pointer < other.Get();
	}

	template <class Y> bool operator >= (const ComPtr<Y> & other) const
	{
		return m_Pointer >= other.Get();
	}

	template <class Y> bool operator <= (const ComPtr<Y> & other) const
	{
		return m_Pointer > other.Get();
	}
};

template <class Y, class U> bool operator != (const U * lhs, const ComPtr<Y> & rhs)
{
	return lhs != rhs.Get();
}

template <class Y, class U> bool operator != (const ComPtr<Y> & lhs, const U * rhs)
{
	return lhs.Get() != rhs;
}

template <class Y, class U> bool operator == (const U * lhs, const ComPtr<Y> & rhs)
{
	return lhs == rhs.Get();
}

template <class Y, class U> bool operator == (const ComPtr<Y> & lhs, const U * rhs)
{
	return lhs.Get() == rhs;
}