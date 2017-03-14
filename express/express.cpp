#include "stdafx.h"
#include "../express_core/expresscore.hh"
using namespace std;
typedef double VT;
int main(int argc, char *argv[])
{
	ExpressCore<VT>::ReadValFunc func = [](ExpressCore<VT>& core,int &ptr, string &text)->VT {
		//此函数考虑小数点
		//此时ptr指向第一个数字字符
		string buf;
		for (;;)
		{
			char c = text[ptr++];
			if (isdigit(c) || c == '.'||c=='e')
			{
				buf.push_back(c);
			}
			else break;
		}
		ptr--;
		double ret = atof(buf.c_str());
		return ret;
	};
	ExpressCore<VT> core(func);
start:
	try
	{
		string text;
		cout << "please enter a expression:";
		cin >> text;
		int ptr = 0;//从开头开始
		//这里添加+ - 运算符
		//我只能说……前面的是B 后面的是A
		core.registSymbol("+", 1, [](VT a, VT b)->VT {return a + b; });
		core.registSymbol("-", 1, [](VT a, VT b)->VT {return a - b; });
		core.registSymbol("*", 2, [](VT a, VT b)->VT {return a*b; });
		core.registSymbol("/", 2, [](VT a, VT b)->VT {return a / b; });
		core.registSymbol("^^", 3, [](VT a, VT b)->VT {
			//a^^b =a*(a-1)*...总共循环b次
			VT sum = a--;
			for (int i = 1; i < b; ++i) sum *= a--;
			return sum;
		});
		core.registFunction("test", [](vector<VT> pars) {
			VT sum = 0;
			for (int i = 0; i < pars.size(); ++i) sum += pars[i];
			return sum;
		});
		//以上注册两个运算符
		VT res = core.readExpression(ptr, text, "=");//以等于结束 或者以ptr大于等于字符串长度为结束
		cout << res << endl;
	}
	catch (string msg)
	{
		cout << msg<<endl;
	}
    //逆波兰式处理
    goto start;
    return 0;
}
