#ifndef _FUNCDEL_H
#define _FUNCDEL_H

#include "JTS_Eng.h"
#include <vector>

typedef void	(CALLBACK *TPProc)(void *pParametr),
				(CALLBACK *TPMsgProc)(void *pParametr, const TWinMessage &stMsg);

template<class T1, class T2>
class TCFuncDelegate
{
	bool _bAllowInvoke;

	struct _TFunc
	{
		void *pParametr;
		T1	pFunc;
	};

	std::vector<_TFunc> _funcList;

public:

	TCFuncDelegate(): _bAllowInvoke(true){}

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
				_funcList.erase(_funcList.begin() + i);
	}

	T2 Invoke;
};

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

typedef TCFuncDelegate<TPProc, void ()> TProcDelegate;
typedef TCFuncDelegate<TPMsgProc, void (const JTS::TWinMessage &)> TMsgProcDelegate;

#endif //_FUNCDEL_H