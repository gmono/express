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
	//添加符号存储函数
	umap<string, VT> valmap;
	auto vfunc=[&valmap](string &name)->VT{
		auto ptr = valmap.find(name);
		if (ptr == valmap.end()) throw string("错误 不存在的符号引用");
		return ptr->second;//返回值
	};
	valmap["hello"] = 128.6;
	//由于目前是直接使用 double做值变量因此所有的操作都是传值  因此不可能基于普通Symbol函数实现
	//形如赋值= 的表达式 如果要实现 需要修改VT为一个代表引用的类
	//目前采用set关键字来实现 set name val 其中name为字符串 要求与不能为关键字 val为值目前为double
	
	ExpressCore<VT> core(func,vfunc);
	//注册set关键字
	core.registKey("set", [&valmap](ExpressCore<VT>&fthis,int &ptr,string& text)->VT {
		//向后读取名字
		string name = fthis.getName(ptr, text);
		//空格分隔 因此后面这时ptr应指向后面量的第一个字符
		//这里使用了readValue 这意味着其只能读取字面值 不能做出把a赋值给b的操作
		VT val = fthis.RunExpression(ptr, text, ";,");
		valmap[name] = val;
		return val;//目前就返回val
	});
	core.registKey("vallist", [&valmap](ExpressCore<VT>&fthis, int &ptr, string& text)->VT {
		for (auto i = valmap.begin(); i != valmap.end(); i++)
		{
			cout << i->first << ":" << i->second << endl;
		}
		return 0;
	});
	core.registKey("for", [&valmap](ExpressCore<VT>&fthis, int &ptr, string& text)->VT {
		VT f = fthis.RunExpression(ptr, text, ":");
		ptr++;
		VT t = fthis.RunExpression(ptr, text, "{");
		ptr++;
		int start = ptr;//记录代码开始位置
		VT ret;
		for (VT i = f; i <= t; ++i)
		{
			ptr = start;
			//记录最后一条语句返回值
			ret = fthis.RunBlock(ptr, text, ";", "}");

			
		}
		ptr++;
		return ret;
	});
	core.registKey("if", [&valmap](ExpressCore<VT>&fthis, int &ptr, string& text)->VT {
		VT val = fthis.RunExpression(ptr, text, "{");
		ptr++;
		int start = ptr;//记录代码开始位置
		VT ret;
		if (val)
		{
			ret = fthis.RunBlock(ptr, text, ";", "}");
			ptr++;
		}
		else
		{
			//跳过 这个功能以后会放到核心功能里
			int csum = 1;
			for (int i = ptr;i<text.length(); ++i)
			{
				if (text[i] == '{') csum++;
				else if(text[i]=='}') csum--;
				if (csum == 0)
				{
					ptr = i;
					break;
				}
			}
			//此时ptr指向}结束符
			ptr++;
			//读取else
			int tptr = ptr;
			string elname = fthis.getName(ptr, text);
			if (elname == "else"&&text[ptr] == '{')
			{
				//执行else语句块
				ptr++;
				ret = fthis.RunBlock(ptr, text, ";", "}");
				ptr++;//跳过尾部}
			}
			else
			{
				//不执行 还原指针
				ptr = tptr;
				ret = 0;
			}
		
		}
		return ret;
	});
	//
	//这里添加+ - 运算符
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
	core.registSymbol(">", 4, [](VT a, VT b)->VT {return a > b; });
	core.registSymbol("<", 4, [](VT a, VT b)->VT {return a < b; });
	core.registSymbol(">=", 4, [](VT a, VT b)->VT {return a >= b; });
	core.registSymbol("<=", 4, [](VT a, VT b)->VT {return a <= b; });
	core.registSymbol("==", 4, [](VT a, VT b)->VT {return a == b; });
	core.registSymbol("!=", 4, [](VT a, VT b)->VT {return a != b; });
	//logic
	core.registSymbol("&&", 5, [](VT a, VT b)->VT {return a && b; });
	core.registSymbol("||", 5, [](VT a, VT b)->VT {return a && b; });
	core.registFunction("test", [](vector<VT> pars) {
		VT sum = 0;
		for (int i = 0; i < pars.size(); ++i) sum += pars[i];
		return sum;
	});
	//以上注册两个运算符
	char *expbuf = new char[4096];//允许4096字符的一行表达式
start:
	try
	{
		
		string text;
		cout << "请输入表达式:";
		getline(cin, text);
		if (text.empty()) goto start;
		int ptr = 0;//从开头开始
		for (; ptr < text.length(); ptr++)
		{
			VT res = core.RunExpression(ptr, text, ";,");//以等于结束 或者以ptr大于等于字符串长度为结束
			cout << res << endl;
		}
		
	}
	catch (string msg)
	{
		cout << msg<<endl;
	}
    goto start;
    return 0;
}
