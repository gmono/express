#ifndef EXPRESSCORE_HH
#define EXPRESSCORE_HH
#include "stdafx.h"
using namespace std;
#define umap unordered_map
template<typename ValueType = int>
class ExpressCore;
///这里的ValueType其实是通用值类型
///传参模式为传值
///因此 如果要实现字符串之类的 请在ValueType中动手脚 然后提供一个通用的外部值读取器
///目前正在添加声明式的处理
///注意以下说得所有的“数字" "value" 值 都指的是ValueType
template<class ValueType>
ValueType genReadValue(ExpressCore<ValueType>&,int &ptr,string &text);
template<class ValueType>
ValueType genGetValue(string &name);
//表达式运算类
template<typename ValueType=int>
class ExpressCore
{
public:
    typedef function<ValueType(ExpressCore<ValueType>&,int &,string&)> ReadValFunc;
	//符号查询函数定义
	//此函数由一个纯符号名字 得到一个ValueType 变量 通常用于变量查询
	typedef function<ValueType(string &)> GetValFunc;
    //默认构造器只有特定的特化版本 目前只有对整数 （int longlong等)
    ExpressCore();
    ExpressCore(ReadValFunc);//正常情况下需要这个函数 除了一些特殊的外
	ExpressCore(ReadValFunc rv, GetValFunc gv);//当程序支持变量储存时 需要此构造函数

	//这里发现至少DEBUG模式下 function模板的调用速度远低于函数指针 考虑替换此声明
	//说明一下区别 标准函数只处理值 符号函数只处理两个值 关键字函数直接处理表达式本身，其中参数1为表达式当前读取指针
	//参数2为表达式存储string
    typedef function<ValueType(const vector<ValueType> &)> ExpFunc;//标准函数定义
    typedef function<ValueType(const ValueType,const ValueType)> SymbolFunc;//符号函数定义
	typedef function<ValueType(ExpressCore<ValueType>&,int &,string&)> KeyFunc;//关键字处理函数定义
	//关于这里有两种方案 方案1为公开所有的表达式相关函数 这样KeyFunc就可以自如的使用本类处理表达式 包括获取名字等
	//方案二为将本类的表达式处理函数作为function放入一个结构体中，传引用给KeyFunc 这样可以保证完整的封装性
	//但是这样效率比较低下
	//目前采用方案1

public:
	//这里本来应该是private的
	//注意 由于方案1需求，KeyFunc可以访问本类的所有内容 但是其他函数不应该这么做

    enum class WordType{Value,Sym,None};

    struct Symbol
    {
        //    string symsign="";//这是调试用的方便识别符号
        Symbol(){}
        Symbol(int level,const SymbolFunc &func);
        int level=0;//优先级0为最低级
        SymbolFunc func;
    };
    struct Word
    {
        WordType type=WordType::None;
        struct
        {
            ValueType value;
            Symbol sym;
        }data;
    };
    //这里建立一个容器可访问的stack适配器
    template<typename _Tp, typename _Sequence = deque<_Tp> >
    struct istack:public stack<_Tp,_Sequence>
    {
        _Sequence *contptr;
        istack(){this->contptr=&this->c;}
    };
    umap<string,Symbol *> symmap; //符号函数映射表 符号->func(a,b) 用于查询运算符的存在性和取得运算符函数 运算符不能为逗号和括号
    umap<string,ExpFunc> exmap;//表达式函数表
	umap<string, KeyFunc> keymap;//关键字函数表

    //原则 任何一个读取函数读取后 一切属于读取范围内的内容都被跳过
	ReadValFunc readValuefun;
	GetValFunc getValuefun;//从符号到值的查询函数
	//读取一个值
    Word readValue(int &ptr,string &text);
	//读取一个二元操作符
    Word readSymbol(int &ptr,string &text);
    //下面为字面读取函数 碰到指定标记结束 返回读取到的值 不包括标记
	//此为全局字面量读取的基础 为了灵活性 使用一个判断函数确定是否可以继续读取 如果此函数返回true代表继续 否则代表
	//中断 返回目前读取到的字面量 结束位置的char不放入字面量中 ptr指向中断位置
	//judge函数参数分别为 当前读取指针 文本整体 当前指针指向的char值
	//getExp函数可以跳过前后空格
	string getExp(int &ptr, string &text, function<bool(int&,string&text,char v)> judgefun, bool isneedtrim = false);
	//通用取名字函数
	string getName(int &ptr, string &text);
	//忽略空白的跳位函数
	void JumpSpace(int &ptr, string &text);
	//下列为有名式读取函数

	//读取声明式 key a,b,c
	//这里存在这样的逻辑 即如果key并不存在于关键字函数表里 则考虑其为一个”名字引用“
	Word readKeyExp(int &ptr, string &text,string &name); //作为关键字函数的读取程序
	Word readFunction(int &ptr, string &text,string &name);//此函数目前做为普通函数的读取函数
	//读取有名操作 记录其名字 判断后分别调用上方不同的读取函数处理 返回值为上面函数的返回值
	Word readNameOpera(int &ptr, string & text);
public:
	//这才是真正公开的部分
    //这个函数会自动跳过空格 但是如果设置成空格结束 也可以
    //这个函数是计算函数 读取一个表达式返回一个值
	//此函数也可以跳过前后空格 这里以后要用getExp替代 统一处理结构
    ValueType RunExpression(int &ptr,string &text,const string &signs);
	//块执行函数
	//此函数使用RunExpression函数执行多行语句 以expsigns中的字符为语句结束符
	//以endsigns中的字符为总结束符
	//结束后ptr指向总结束符 如果为text末尾则指向text->length() 即字符串中的结束符0
	ValueType RunBlock(int &ptr, string &text, const string &expsigns, const string &endsigns);
    //下面是注册函数
	//注册函数的返回值都是 true代表为替换操作  false表示注册新的
    //注册一个运算符
    //返回值表示是否已经存在（即这次操作是否属于替换操作）
    bool registSymbol(const string &name,int level,const SymbolFunc &func);
    //注册一个函数
    bool registFunction(const string &name,ExpFunc func);
	//注册一个关键字
	bool registKey(const string &name, KeyFunc func);
};
template<>
ExpressCore<int>::ExpressCore() :ExpressCore(&genReadValue<int>) {}
template<>
ExpressCore<long long>::ExpressCore():ExpressCore(&genReadValue<long long>) {}
template<>
ExpressCore<long>::ExpressCore() :ExpressCore(&genReadValue<long>) {}
template<>
ExpressCore<short>::ExpressCore() :ExpressCore(&genReadValue<short>) {}
template<>
ExpressCore<double>::ExpressCore() :ExpressCore(&genReadValue<double>) {}
//unsigned 特化构造器
template<>
ExpressCore<unsigned int>::ExpressCore():ExpressCore(&genReadValue<unsigned int>) {}
template<>
ExpressCore<unsigned long long>::ExpressCore():ExpressCore(&genReadValue<unsigned long long>) {}
template<>
ExpressCore<unsigned long>::ExpressCore():ExpressCore(&genReadValue<unsigned long>) {}
template<>
ExpressCore<unsigned short>::ExpressCore() :ExpressCore(&genReadValue<unsigned short>) {}
//以上为特化构造器

//只提供读取函数的
template<class T>
ExpressCore<T>::ExpressCore(ReadValFunc rnfunc) : ExpressCore(rnfunc, &genGetValue<T>) {}
//原始构造函数
template<typename ValueType>
inline ExpressCore<ValueType>::ExpressCore(ReadValFunc rv, GetValFunc gv)
{
	//原始构造函数
	this->readValuefun = rv;
	this->getValuefun = gv;
}

//这个函数用于整数
template<class T>
auto genReadValue(ExpressCore<T>& core,int &ptr,string &text)->T
{
    //这里有个基本认识即string的end就是\0
    T sum=0;
    for(char c=text[ptr++];c<='9'&&c>='0';c=text[ptr++])
    {
        sum=sum*10+(c-'0');
    }
    ptr--;//调整 由于循环完时会让ptr++ 因此会在结束符后面一位 调整到前一位
    return sum;
}

template<class ValueType>
inline ValueType genGetValue(string & name)
{
	//默认取值函数抛出错误
	throw string("错误，不支持值存储");
}

////////////////////////////

template<class T>
auto ExpressCore<T>::readValue(int &ptr, string &text)->Word
{
    Word ret;
    ret.type=WordType::Value;
    ret.data.value=this->readValuefun(*this,ptr,text); //从自定义数字解析器解析数字 得到valueType型
    return ret;
}

template<class T>
auto ExpressCore<T>::readSymbol(int &ptr, string &text)->Word
{
    //运算符 遇到任何字母或数字时停止
	auto jfun = [](int &p,string &t,char v)->bool {
		if (!isalnum(v) || v == '_') return true;
		return false;
	};
	string symbuf = getExp(ptr, text, jfun, true);
    auto sptr=symmap.find(symbuf);
    if(sptr!=symmap.end())
    {
        Word ret;
        ret.type=WordType::Sym;
        ret.data.sym=*(sptr->second);
        return ret;
    }
    throw string("运算符错误");
}
template<typename ValueType>
inline string ExpressCore<ValueType>::getExp(int & ptr, string & text, function<bool(int&,string&text,char v)> judgefun,bool isneedtrim)
{
	if (ptr >= text.length()) return "";//直接遇到末尾情况 直接结束
	string namebuf;//字面量存储
	//这里说明一下 string 取序列后面的值 会返回\0 字符 如同C字符串的末尾
	for (char c = text[ptr++];ptr<=text.length()&&c!='\0'&&judgefun(ptr,text,c); c = text[ptr++])
	{
		namebuf.push_back(c);
	}
	//此时字符串指向停止的字符的后后面
	ptr--;//调整 让其指向结束字符本身 莫名其妙 本来应该指向结束字符后面的莫名就指向了其本身
	if (!isneedtrim) return namebuf;//如不需要去空白就直接返回 提升性能
	//去前后空白
	auto start = namebuf.begin();
	for (; start != namebuf.end() && isspace(*start); ++start);
	auto end = namebuf.rbegin();
	for (; end != namebuf.rend() && isspace(*end); ++end);
	string ret;//这里本来想用start和end直接构造string的不过报错
	for (auto i = start; i != end.base(); ++i) ret.push_back(*i);//低效的方法 不知有没有高效的 
	return ret;
}
template<typename ValueType>
inline string ExpressCore<ValueType>::getName(int & ptr, string & text)
{
	//此时ptr指向此字符
	bool isending = false;//记录是否在末尾读取中 所谓末尾读取是指在遇到第一个空白字符时进入的模式
						  //此模式下遇到第一个非空白字符便终止
	auto judgefun = [&isending](int &p, string &t, char c)->bool {
		//这里本来判断的是是否空白 现在用(isalnum(c)||c=='_')替代 “非空白”，用其取反代替空白
		//目前这样固定符号必须为字母数字和下划线组成 以后可以考虑用更灵活的方法实现
		if (isending)
		{
			if (!isspace(c))
				return false;
			else return true;//ending下忽略空白遇到非空白结束
		}
		else
		{
			//这里说明一下  只有遇到空白才进入ending模式 非空白字符就要看是否为下划线和字母数字 如果不是就结束
			if (isspace(c))
			{
				isending = true;
				return true;
			}
			else if ((isalnum(c) || c == '_'))
				return true;
			else return false;
			//如果是不是ending遇到空白则进入ending状态
		}

		//读取名字过程中 字母和数字都是允许的 注意 数字开头的在RunExpression中会识别为数字
		//不会调用此函数
	};
	string name = this->getExp(ptr, text, judgefun, true); //去前后空白 得到名字
	return name;
}
template<typename ValueType>
inline void ExpressCore<ValueType>::JumpSpace(int & ptr, string & text)
{
	for (char c = text[ptr++]; ptr < text.length()&&c==' '; c=text[ptr++])
	{

	}
	ptr--;
}
template<typename ValueType>
inline auto ExpressCore<ValueType>::readKeyExp(int & ptr, string & text, string & name)->Word
{
	//策略：查询名字在不在关键字表中 如果在则当作关键字 不在则当作符号 调用符号查询函数处理
	auto mp = keymap.find(name);
	ValueType data;
	if (mp != keymap.end())
	{
		//关键字的情况
		KeyFunc &fun = mp->second;
		data=fun(*this, ptr, text);
	}
	else
	{
		//符号的情况
		data = this->getValuefun(name);
	}
	//包装成Word 注意这里假定任何一个关键词函数都会返回一个值 值不定 意义不明。。 请充分发挥创造力..
	Word ret;
	ret.type = WordType::Value;
	ret.data.value = data;
	return ret;
}
template<class T>
auto ExpressCore<T>::readFunction(int &ptr, string &text,string &name)->Word
{
    //此时ptr指向'第一个不是数字和字母的字符‘ 后面的那个 正常情况下那个字符应该是'('
	//这里有了一些变化 因为添加声明式 声明式形如 key name;因此这里的函数名还可能是Key
    auto sptr=exmap.find(name);
    if(sptr==exmap.end()) throw string("没有这个函数");
    ExpFunc fun=sptr->second;
    //读取参数表 参数表以逗号分割 每一个参数都是一个表达式
    vector<T> pars;//参数表
    while(ptr<text.length())//这里保证其不会越界
    {
        T p=this->RunExpression(ptr,text,",)");//以逗号和反括号结尾 结尾时ptr指向逗号
        pars.push_back(p);
        if(text[ptr]==',') {ptr++;continue;}
        if(text[ptr]==')') {ptr++;break;}//遇到反括号就跳出
        else throw string("内部错误！");//这里理应是上面两个字符 如果不是就是内部错误了
    }
    //调用函数得到结果
    T num=fun(pars);
    Word ret;
    ret.type=WordType::Value;
    ret.data.value=num;
    return ret;
}
//有名操作读取函数 这是关键位置
template<typename ValueType>
inline auto ExpressCore<ValueType>::readNameOpera(int & ptr, string & text)->Word
{
	//此函数为遇到非空白非数字字符时调用
	//此时ptr指向此字符
	//读取名字过程中 字母和数字都是允许的 注意 数字开头的在RunExpression中会识别为数字
	//不会调用此函数
	//判断处理
	//
	string name = getName(ptr, text);
	char c = text[ptr];//取得中断值
	switch (c)
	{
	case '(':
		//这是函数调用的情况 名字后面跟括号(忽略空白)
		return readFunction(ptr, text, name);
		break;
	default:
		//这是关键字表达式的情况 这种情况有可能是一个声明式或者一个符号式（单纯的符号引用)
		return readKeyExp(ptr, text, name);
		break;
	}
}
template<class T>
auto ExpressCore<T>::RunExpression(int &ptr, string &text,const string &signs)->T
{
    //signs中的任意字符为结束标志 同时末尾自动结束
    stack<Word> stack1; //符号栈
    istack<Word> stack2; //操作数栈
    Word ls;//最低优先级运算符
    ls.type=WordType::Sym;
    ls.data.sym=Symbol(); //默认新的symbol 初始优先级为最低0
    stack1.push(ls);
    //构造查询set
    set<char> query;
    for(int i=0;i<signs.length();++i) query.insert(signs[i]);
    //初始化完成
    for(;;)
    {
        if(ptr>=text.length()) break;//保证不越界
        char c=text[ptr];
        if(query.find(c)!=query.end()) break;//遇到结束标记 结束
		if (c == ' ') { ptr++; continue; }//跳过空格
        if(c=='(')
        {
            int p=RunExpression(ptr,text,")");
            Word temp;
            temp.type=WordType::Value;
            temp.data.value=p;
            stack2.push(temp);
            //此时ptr指向反括号
            ptr++;//跳过反括号
        }
        else if(isdigit(c))
        {
            //数字
            Word temp=readValue(ptr,text);
            stack2.push(temp);
        }
        else if(isalpha(c))
        {
            //字母
            //当函数处理
			Word temp = readNameOpera(ptr, text);
            stack2.push(temp);
        }
        else
        {
            //当操作符处理
            Word temp=readSymbol(ptr,text);
            int nlevel=temp.data.sym.level;
            Word top=stack1.top();
            int olevel=top.data.sym.level;
            for(;;)
            {
                if(nlevel>olevel)
                {
                    stack1.push(temp);
                    break;
                }
                else
                {
                    //将s1的顶运算符弹出送入s2栈
                    stack2.push(stack1.top());
                    stack1.pop();
                    //更新olevel
                    top=stack1.top();
                    olevel=top.data.sym.level;
                }
            }
        }

    }
    //处理完成，最后一步把stack1中的运算符出到stack2中
    for(Word t=stack1.top();t.data.sym.level!=0;t=stack1.top())
    {
        stack2.push(t);
        stack1.pop();
    }
    //构造完成
    deque<Word> &cont=*(stack2.contptr);//stack从低向高生长 因此直接zheng正序遍历
    //计算开始
    stack<T> nums;//数字计算栈
    for(auto i=cont.begin();i!=cont.end();++i)
    {
        Word w=*i;
        if(w.type==WordType::Value) nums.push(w.data.value);
        else if(w.type==WordType::Sym)
        {
            //运算符 取出两个计算后再压入
            //例如 1+2+这样的表达式就会出现 12++这样的逆波兰式 这样就会出现取数错误
            try
            {
				/*
				T a=nums.top();
				nums.pop();
				T b=nums.top();
				nums.pop();

				*/
                //a为后入栈 所以是后面的数
				T a;
				if (nums.size() > 0)
				{
					a = nums.top();
					nums.pop();
				}
				else throw "";
				T b;
				if (nums.size() > 0)
				{
					b = nums.top();
					nums.pop();
				}
				else throw "";

                T sum=w.data.sym.func(b,a); //1-2则是 func(1,2) 栈中从栈顶开始为2 1
                nums.push(sum);
            }
            catch(...)
            {
                throw string("取值错误！值的个数不正确");
            }
        }
        else throw string("内部错误");//无故出现None型对象 内部错误
    }
    //得到结果
    T ret=nums.top();
    return ret;
}
template<typename ValueType>
inline ValueType ExpressCore<ValueType>::RunBlock(int & ptr, string & text, const string & expsigns, const string & endsigns)
{
	VT ret;
	//确保没有读取完 且 没有读到最终结束符
	while (ptr < text.length()&&endsigns.find(text[ptr])==-1)
	{
		//不断执行语句
		ret = this->RunExpression(ptr, text, expsigns);
		ptr++;//跳过语句结束符 如果是读到末尾 没有结束符 则从0结束处跳到之后
	}
	
	////结束后ptr指向最终结束符之后 因此要调整
	//ptr--;
	//目前考虑不用调整
	return ret;
}
template<class T>
bool ExpressCore<T>::registSymbol(const string &name, int level, const SymbolFunc &func)
{
    bool isexist=symmap.find(name)!=symmap.end()? true:false;//如果存在就返回true
    if(isexist)
    {
        Symbol *old=symmap[name];
        symmap.erase(name);
        delete old;
        //释放原来的
    }
    Symbol *symbol=new Symbol(level,func);
    this->symmap[name]=symbol;
    return isexist;//这里是返回true代表替换 相反的话可以加个取反
}
template<class T>
bool ExpressCore<T>::registFunction(const string &name, ExpFunc func)
{
    bool isexist=exmap.find(name)!=exmap.end()? true:false;//如果存在就返回true
    //由于无需释放，所以直接替换 应该没事
    this->exmap[name]=func;//允许替换
	return isexist;
}
template<typename ValueType>
inline bool ExpressCore<ValueType>::registKey(const string & name, KeyFunc func)
{
	auto ptr=this->keymap.find(name);
	bool isexist = ptr != this->keymap.end() ? true : false;
	keymap[name] = func;
	return isexist;
}
template<class T>
ExpressCore<T>::Symbol::Symbol(int level, const SymbolFunc &func)
{
    this->level=level;
    this->func=func;
}


#endif // EXPRESSCORE_HH
