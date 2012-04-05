#ifndef _FUNCDEL_H
#define _FUNCDEL_H

#include "JTS_Eng.h"
#include <vector>

typedef void	(CALLBACK *TPProc)(void *pParametr),
				(CALLBACK *TPMsgProc)(void *pParametr, const JTS::TWinMessage &stMsg);

template<class T1, class T2>
class TCFuncDelegate;

#ifndef _MSC_VER
template<typename F>
class CFunctorImpl;

template<typename F>
class CFunctor: public CFunctorImpl<F>
{
	template<class T1, class T2>
	friend class TCFuncDelegate;

	// C++11 inheriting ctor
	//using CFunctorImpl<F>::CFunctorImpl;
	template<class T1, class T2>
	CFunctor(TCFuncDelegate<T1, T2> &parent): CFunctorImpl<F>(parent) {}
};
#endif

template<class T1, class T2>
class TCFuncDelegate
{
	friend class CFunctorImpl<T2>;

	bool _bAllowInvoke;

	struct _TFunc
	{
		void *pParametr;
		T1	pFunc;
	};

	std::vector<_TFunc> _funcList;

public:

	TCFuncDelegate(): _bAllowInvoke(true), Invoke(*this) {}

	void AllowInvoke(bool bAllow)
	{
		_bAllowInvoke = bAllow;
	}

	inline bool IsNull()
	{
		return _funcList.empty();
	}

	void Add(T1 pFunc, void *pParametr)
	{
		_TFunc tmp;
		tmp.pParametr = pParametr;
		tmp.pFunc = pFunc;
		_funcList.push_back(tmp);
	}

	void Remove(T1 pFunc, void *pParametr)
	{
		for (std::size_t i = 0; i < _funcList.size(); i++)
			if (_funcList[i].pParametr == pParametr && _funcList[i].pFunc == pFunc)
			{
				_funcList.erase(_funcList.begin() + i);
				break;
			}
	}

#ifdef _MSC_VER
	T2 Invoke;
#else
	CFunctor<T2> Invoke;
#endif
};

#ifndef _MSC_VER
template<class T1, class T2>
class CFunctorBase
{
protected:
	CFunctorBase(TCFuncDelegate<T1, T2> &parent): _parent(parent) {}
	TCFuncDelegate<T1, T2> &_parent;
};

template<>
class CFunctorImpl<void ()>: CFunctorBase<TPProc, void ()>
{
protected:
	// C++11 inheriting ctor
	//using CFunctorBase::CFunctorBase;
	CFunctorImpl(TCFuncDelegate<TPProc, void ()> &parent): CFunctorBase(parent) {}
public:
	void operator ()()
	{
		if (_parent._bAllowInvoke)
			for (std::size_t i = 0; i < _parent._funcList.size(); i++)
				(*_parent._funcList[i].pFunc)(_parent._funcList[i].pParametr);
	}
};

template<>
class CFunctorImpl<void (const JTS::TWinMessage &)>: CFunctorBase<TPMsgProc, void (const JTS::TWinMessage &)>
{
protected:
	// C++11 inheriting ctor
	//using CFunctorBase::CFunctorBase;
	CFunctorImpl(TCFuncDelegate<TPMsgProc, void (const JTS::TWinMessage &)> &parent): CFunctorBase(parent) {}
public:
	void operator ()(const JTS::TWinMessage &stMsg)
	{
		if (_parent._bAllowInvoke)
			for (std::size_t i = 0; i < _parent._funcList.size(); i++)
				(*_parent._funcList[i].pFunc)(_parent._funcList[i].pParametr, stMsg);
	}
};
#else
inline void TCFuncDelegate<TPProc, void ()>::Invoke()
{
	if (_bAllowInvoke)
		for (std::size_t i = 0; i < _funcList.size(); i++)
			(*_funcList[i].pFunc)(_funcList[i].pParametr);
}

inline void TCFuncDelegate<TPMsgProc, void (const JTS::TWinMessage &)>::Invoke(const JTS::TWinMessage &stMsg)
{
	if (_bAllowInvoke)
		for (std::size_t i = 0; i < _funcList.size(); i++)
			(*_funcList[i].pFunc)(_funcList[i].pParametr, stMsg);
}
#endif

typedef TCFuncDelegate<TPProc, void ()> TProcDelegate;
typedef TCFuncDelegate<TPMsgProc, void (const JTS::TWinMessage &)> TMsgProcDelegate;

#endif //_FUNCDEL_H
