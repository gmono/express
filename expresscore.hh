#ifndef EXPRESSCORE_HH
#define EXPRESSCORE_HH
#include <stack>
#include <vector>
#include  <functional>
#include <unordered_map>
#include <deque>
#include <set>
using namespace std;
#define umap unordered_map
template<class ValueType>
ValueType genReadNumber(int &ptr,string &text);
//表达式运算类
template<typename ValueType=int>
class ExpressCore
{
public:
    typedef function<ValueType(int &,string&)> ReadNumFunc;
    //默认构造器只有特定的特化版本 目前只有对整数 （int longlong等)
    ExpressCore();
    ExpressCore(ReadNumFunc);//正常情况下需要这个函数 除了一些特殊的外

    typedef function<ValueType(const vector<ValueType> &)> ExpFunc;//标准函数定义
    typedef function<ValueType(const ValueType,const ValueType)> SymbolFunc;//符号函数定义

private:

    enum class WordType{Number,Sym,None};

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
            ValueType number;
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

    //原则 任何一个读取函数读取后 一切属于读取范围内的内容都被跳过

//    friend class GenFuns<ValueType>;
    Word readNumber(int &ptr,string &text);
    ReadNumFunc readNumberfun;
    Word readSymbol(int &ptr,string &text);
    Word readFunction(int &ptr,string &text);
public:
    //这个函数会自动跳过空格 但是如果设置成空格结束 也可以
    //这个函数是计算函数 读取一个表达式返回一个值
    ValueType readExpression(int &ptr,string &text,string signs);
    //下面是注册函数

    //注册一个运算符
    //返回值表示是否已经存在（即这次操作是否属于替换操作）
    bool registSymbol(const string &name,int level,const SymbolFunc &func);
    //注册一个函数
    void registFunction(const string &name,ExpFunc func);
};
template<>
ExpressCore<int>::ExpressCore()
{
    this->readNumberfun=&genReadNumber<int>;
}
template<>
ExpressCore<long long>::ExpressCore()
{
    this->readNumberfun=&genReadNumber<long long>;
}
template<>
ExpressCore<long>::ExpressCore()
{
    this->readNumberfun=&genReadNumber<long>;
}
template<>
ExpressCore<short>::ExpressCore()
{
    this->readNumberfun=&genReadNumber<short>;
}
template<>
ExpressCore<double>::ExpressCore()
{
    this->readNumberfun=&genReadNumber<double>;
}
//unsigned 特化构造器
template<>
ExpressCore<unsigned int>::ExpressCore()
{
//    this->readNumberfun=[](int &ptr,string &text){return genReadNumber<int>(ptr,text);};
    this->readNumberfun=&genReadNumber<int>;
}
template<>
ExpressCore<unsigned long long>::ExpressCore()
{
    this->readNumberfun=&genReadNumber<long long>;
}
template<>
ExpressCore<unsigned long>::ExpressCore()
{
    this->readNumberfun=&genReadNumber<long>;
}
template<>
ExpressCore<unsigned short>::ExpressCore()
{
    this->readNumberfun=&genReadNumber<short>;
}
//以上为特化构造器

template<class T>
ExpressCore<T>::ExpressCore(ReadNumFunc rnfunc)
{
    //这是通用情况下 使用外部的数字读取器
    this->readNumberfun=rnfunc;
}

//这个函数用于整数
template<class T>
auto genReadNumber(int &ptr,string &text)->T
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

////////////////////////////

template<class T>
auto ExpressCore<T>::readNumber(int &ptr, string &text)->Word
{
    Word ret;
    ret.type=WordType::Number;
    ret.data.number=this->readNumberfun(ptr,text); //从自定义数字解析器解析数字 得到valueType型
    return ret;
}

template<class T>
auto ExpressCore<T>::readSymbol(int &ptr, string &text)->Word
{
    //运算符 遇到任何字母或数字时停止
    string symbuf;
    for(char c=text[ptr++];!isalnum(c);c=text[ptr++])
    {
        symbuf.push_back(c);
    }
    ptr--;//调节
    auto sptr=symmap.find(symbuf);
    if(sptr!=symmap.end())
    {
        Word ret;
        ret.type=WordType::Sym;
        ret.data.sym=*(sptr->second);
        return ret;
    }
    throw "运算符错误";
}
template<class T>
auto ExpressCore<T>::readFunction(int &ptr, string &text)->Word
{
    //函数遇到字母时进入函数读取模式
    string namebuf;//函数名存储
    for(char c=text[ptr++];isalnum(c);c=text[ptr++])
    {
        namebuf.push_back(c);
    }
    //此时ptr指向'第一个不是数字和字母的字符‘ 后面的那个 正常情况下那个字符应该是'('
    if(text[ptr-1]!='(')
    {
        string err;
        err="期望得到'(',实际得到'";
        err.push_back(text[ptr]);
        err.push_back('\'');
        throw err;
    }
    auto sptr=exmap.find(namebuf);
    if(sptr==exmap.end()) throw "没有这个函数";
    ExpFunc fun=sptr->second;
    //读取参数表 参数表以逗号分割 每一个参数都是一个表达式
    vector<T> pars;//参数表
    while(ptr<text.length())//这里保证其不会越界
    {
        T p=this->readExpression(ptr,text,",)");//以逗号和反括号结尾 结尾时ptr指向逗号
        pars.push_back(p);
        if(text[ptr]==',') {ptr++;continue;}
        if(text[ptr]==')') {ptr++;break;}//遇到反括号就跳出
        else throw "内部错误！";//这里理应是上面两个字符 如果不是就是内部错误了
    }
    //调用函数得到结果
    T num=fun(pars);
    Word ret;
    ret.type=WordType::Number;
    ret.data.number=num;
    return ret;
}
template<class T>
auto ExpressCore<T>::readExpression(int &ptr, string &text, string signs)->T
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
        if(c==' ') continue;//跳过空格
        if(c=='(')
        {
            int p=readExpression(ptr,text,")");
            Word temp;
            temp.type=WordType::Number;
            temp.data.number=p;
            stack2.push(temp);
            //此时ptr指向反括号
            ptr++;//跳过反括号
        }
        else if(isdigit(c))
        {
            //数字
            Word temp=readNumber(ptr,text);
            stack2.push(temp);
        }
        else if(isalpha(c))
        {
            //字母
            //当函数处理
            Word temp=readFunction(ptr,text);
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
        if(w.type==WordType::Number) nums.push(w.data.number);
        else if(w.type==WordType::Sym)
        {
            //运算符 取出两个计算后再压入
            //例如 1+2+这样的表达式就会出现 12++这样的逆波兰式 这样就会出现取数错误
            try
            {
                //a为后入栈 所以是后面的数
                T a=nums.top();
                nums.pop();
                T b=nums.top();
                nums.pop();
                T sum=w.data.sym.func(b,a); //1-2则是 func(1,2) 栈中从栈顶开始为2 1
                nums.push(sum);
            }
            catch(...)
            {
                throw "取数错误！数字个数不正确";
            }
        }
        else throw "内部错误";//无故出现None型对象 内部错误
    }
    //得到结果
    T ret=nums.top();
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
void ExpressCore<T>::registFunction(const string &name, ExpFunc func)
{
//    bool isexist=exmap.find(name)!=exmap.end()? true:false;//如果存在就返回true
//    if(isexist) exmap.erase(name);//擦除
    //由于无需释放，所以直接替换 应该没事
    this->exmap[name]=func;//允许替换
}
template<class T>
ExpressCore<T>::Symbol::Symbol(int level, const SymbolFunc &func)
{
    this->level=level;
    this->func=func;
}


#endif // EXPRESSCORE_HH
