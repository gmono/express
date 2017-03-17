#pragma once
#include "stdafx.h"
#include "../express_core/expresscore.hh"

//此类专为脚本语言设计 提供语句块支持 
//变量存储支持
//控制结构支持
//作用域支持
//单目运算符支持
//程序块存储支持（函数）
template<typename ValueType>
class LanguageCore :public ExpressCore<ValueType>
{
public:
	enum class SideSymbolType{Left,Right};
	LanguageCore();
	//运行一段完整的程序代码 返回值为最后一条语句的返回值
	ValueType RunCodes(const string &code);
	//注册单目运算符
	ValueType registSideSymbol(SideSymbolType stype, const string &sym);
};

template<typename ValueType>
inline LanguageCore<ValueType>::LanguageCore()
{
}
