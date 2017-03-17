#pragma once
#include "stdafx.h"
#include "../express_core/expresscore.hh"

//����רΪ�ű�������� �ṩ����֧�� 
//�����洢֧��
//���ƽṹ֧��
//������֧��
//��Ŀ�����֧��
//�����洢֧�֣�������
template<typename ValueType>
class LanguageCore :public ExpressCore<ValueType>
{
public:
	enum class SideSymbolType{Left,Right};
	LanguageCore();
	//����һ�������ĳ������ ����ֵΪ���һ�����ķ���ֵ
	ValueType RunCodes(const string &code);
	//ע�ᵥĿ�����
	ValueType registSideSymbol(SideSymbolType stype, const string &sym);
};

template<typename ValueType>
inline LanguageCore<ValueType>::LanguageCore()
{
}
